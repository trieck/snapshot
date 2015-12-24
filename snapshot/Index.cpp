#include "stdafx.h"
#include "Index.h"
#include "Primes.h"
#include "DoubleHash.h"

// helper macros
#define BUCKET_KEY(p, b)        (&(p->buckets[b].key))
#define BUCKET_VAL_LEN(p, b)    (p->buckets[b].len)
#define BUCKET_OFFSET(p, b)     (p->buckets[b].offset)
#define DATA_PTR(p, o)          (&(p->data[o]))

// number of buckets on a page
constexpr auto BUCKETS_PER_PAGE = BlockIO::BLOCK_SIZE / sizeof(Bucket);

Index::Index() : tablesize_(0), pageno_(0), offset_(0)
{
    bpage_ = static_cast<LPBUCKETPAGE>(mkblock());
    dpager_ = static_cast<LPDATAPAGE>(mkblock());
    dpagew_ = static_cast<LPDATAPAGE>(mkblock());
}

Index::~Index()
{
    freeblock(bpage_);
    freeblock(dpager_);
    freeblock(dpagew_);
    close();
}

void Index::open(const char* filename, OpenMode mode, uint32_t entries)
{
    close();

    tablesize_ = Primes::prime(entries);

    mode |= std::ios::binary;
    auto create = (mode & std::ios::out) != 0;
    if (create) mode |= std::ios::trunc;  // truncate on create
    io_.open(filename, mode);
    if (create) mktable();
}

void Index::close()
{
    io_.close();
}

bool Index::insert(const Event& event)
{
    auto h = hash(event);
    auto pageno = h / BUCKETS_PER_PAGE;
    auto bucket = h % BUCKETS_PER_PAGE;

    if (!io_.readblock(pageno, bpage_))
        return false;

    for (;;) {
        if (keyLength(bucket) == 0)
            break;

        if ((bucket = (bucket + 1) % BUCKETS_PER_PAGE) == 0) {  // next page
            pageno = (pageno + 1) % (tablesize_ / BUCKETS_PER_PAGE);
            if (!io_.readblock(pageno, bpage_))
                return false;
        }
    }

    setKey(bucket, event);

    uint32_t written;
    uint64_t offset;
    if (!writeValue(event, written, offset))
        return false;

    BUCKET_VAL_LEN(bpage_, bucket) = written;
    BUCKET_OFFSET(bpage_, bucket) = offset;

    return io_.writeblock(pageno, bpage_);
}

bool Index::lookup(const std::string& key, std::string& value)
{
    auto h = hash(key);
    auto pageno = h / BUCKETS_PER_PAGE;
    auto bucket = h % BUCKETS_PER_PAGE;

    value.clear();

    if (!io_.readblock(pageno, bpage_))
        return false;

    std::string K;
    for (;;) {
        K = getKey(bucket);
        if (K.length() == 0)
            return false;   // no hit

        if (K == key)
            break;  // hit

        if ((bucket = (bucket + 1) % BUCKETS_PER_PAGE) == 0) {  // next page
            pageno = (pageno + 1) % (tablesize_ / BUCKETS_PER_PAGE);
            if (!io_.readblock(pageno, bpage_))
                return false;
        }
    }

    auto offset = BUCKET_OFFSET(bpage_, bucket);
    auto length = BUCKET_VAL_LEN(bpage_, bucket);

    return readVal(offset, length, value);
}

std::string Index::getKey(uint64_t bucket)
{
    std::ostringstream ss;

    auto key = *BUCKET_KEY(bpage_, bucket);
    for (auto i = 0; i < MAX_KEY_LEN; ++i) {
        if (key[i] == '\0')
            break;
        ss << key[i];
    }

    return ss.str();
}

uint32_t Index::keyLength(uint64_t bucket)
{
    uint32_t length = 0;

    auto key = *BUCKET_KEY(bpage_, bucket);
    for (auto i = 0; i < MAX_KEY_LEN; ++i) {
        if (key[i] == '\0')
            break;
        length++;
    }

    return length;
}

void Index::setKey(uint64_t bucket, const Event& event)
{
    auto objectId = event.getObjectId();
    auto length = std::min(objectId.length(), MAX_KEY_LEN);
    memcpy(BUCKET_KEY(bpage_, bucket), objectId.c_str(), length);
}

void Index::mktable()
{
    auto nblocks = tablesize_ / BUCKETS_PER_PAGE;
    io_.writeblock(nblocks - 1, bpage_);
    io_.flush();
}

void* Index::mkblock()
{
    return new char[BlockIO::BLOCK_SIZE]();
}

void Index::freeblock(void* block)
{
    delete[] block;
}

uint64_t Index::hash(const Event& event)
{
    return hash(event.getObjectId());
}

uint64_t Index::hash(const std::string& s)
{
    return doublehash<std::string>(s) % tablesize_;
}

bool Index::writeValue(const Event& event, uint32_t& written, uint64_t& offset)
{
    auto value = writer_.write(event);
    auto length = static_cast<int>(value.length());

    if (!writeValue(value.c_str(), length, offset))
        return false;

    written = length;

    return true;
}

void Index::newpage()
{
    memset(dpagew_, 0, BlockIO::BLOCK_SIZE);
    pageno_++;
    offset_ = 0;
}

int Index::available() const
{
    return static_cast<int>(BlockIO::BLOCK_SIZE - offset_);
}

bool Index::writeValue(const char* pval, int length, uint64_t& offset)
{
    if (pageno_ == 0) { // first data page
        pageno_ = tablesize_ / BUCKETS_PER_PAGE;
    }

    if (available() == 0) {
        newpage();
    }

    offset = pageno_ * BlockIO::BLOCK_SIZE + offset_;

    while (length > 0) {
        auto avail = available();
        if (avail == 0) {   // full page
            newpage();
            avail = BlockIO::BLOCK_SIZE;
        }

        auto nlength = std::min(length, avail);
        auto save = nlength;

        auto ptr = DATA_PTR(dpagew_, offset_);
        while (nlength > 0) {
            *ptr++ = *pval++;
            nlength--;
            offset_++;
        }

        if (!io_.writeblock(pageno_, dpagew_))
            return false;

        length -= save;
    }

    return true;
}

bool Index::readVal(uint64_t offset, int length, std::string& value)
{
    auto pageno = offset / BlockIO::BLOCK_SIZE;
    auto poffset = static_cast<int>(offset % BlockIO::BLOCK_SIZE);

    std::ostringstream ss;

    for (auto read = 0; read < length; ) {
        if (!io_.readblock(pageno, dpager_))
            return false;

        auto ptr = DATA_PTR(dpager_, poffset);

        auto avail = static_cast<int>(BlockIO::BLOCK_SIZE - poffset);
        auto needed = std::min(avail, length);

        for (; needed > 0; needed--, read++) {
            ss << *ptr++;
        }

        if (read < length) {
            pageno++;
            poffset = 0;
        }
    }

    value = ss.str();

    return true;
}
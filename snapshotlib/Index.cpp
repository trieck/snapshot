#include "snapshotlib.h"
#include "Index.h"
#include "Primes.h"
#include "DoubleHash.h"

// bucket flags                 
#define BF_DELETED              (1 << 0)

// helper macros
#define BUCKET_KEY(p, b)        (&(p->buckets[b].key))
#define BUCKET_VAL_LEN(p, b)    (p->buckets[b].len)
#define BUCKET_OFFSET(p, b)     (p->buckets[b].offset)
#define DATA_PTR(p, o)          (&(p->data[o]))
#define ISDELETED(p, b)         (p->buckets[b].flags & BF_DELETED)
#define SETDELETED(p, b)        (p->buckets[b].flags |= BF_DELETED)

// number of buckets on a page
constexpr auto BUCKETS_PER_PAGE = BlockIO::BLOCK_SIZE / sizeof(Bucket);

Index::Index() : tablesize_(0), nbpages_(0), pageno_(0), offset_(0)
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

void Index::open(const char* filename, uint32_t entries)
{
    filename_ = filename;

    close();

    tablesize_ = Primes::prime(entries);
    perm_.generate(tablesize_ - 1);
    nbpages_ = (tablesize_ / BUCKETS_PER_PAGE) + 1;

    io_.open(filename, std::ios::in | std::ios::out | std::ios::trunc);
    mktable();
}

void Index::close()
{
    io_.close();
}

std::string Index::filename() const
{
    return filename_;
}

bool Index::find(const Event& event)
{
    return find(event.getObjectId());
}

bool Index::find(const std::string& key)
{
    uint64_t pageno, bucket;
    if (!getBucket(key, pageno, bucket))
        return false;

    if (ISDELETED(bpage_, bucket))
        return false;

    return true;
}

bool Index::insert(const Event& event)
{
    auto key = event.getObjectId();

    uint64_t pageno, bucket;
    if (!findSlot(key, pageno, bucket))
        return false;

    setKey(bucket, key);

    uint32_t written;
    uint64_t offset;
    if (!writeValue(event, written, offset))
        return false;

    BUCKET_VAL_LEN(bpage_, bucket) = written;
    BUCKET_OFFSET(bpage_, bucket) = offset;

    io_.writeblock(pageno, bpage_);

    return true;
}

bool Index::lookup(const std::string& key, std::string& value)
{
    value.clear();

    uint64_t pageno, bucket;
    if (!getBucket(key, pageno, bucket))
        return false;

    if (ISDELETED(bpage_, bucket))
        return false;

    auto offset = BUCKET_OFFSET(bpage_, bucket);
    auto length = BUCKET_VAL_LEN(bpage_, bucket);

    readVal(offset, length, value);

    return true;
}

bool Index::destroy(const Event& event)
{
    auto key = event.getObjectId();

    uint64_t pageno, bucket;
    if (!getBucket(key, pageno, bucket))
        return false;

    SETDELETED(bpage_, bucket) = 1;

    io_.writeblock(pageno, bpage_);

    return true;
}

bool Index::update(const Event& event)
{
    auto key = event.getObjectId();

    uint64_t pageno, bucket;
    if (!getBucket(key, pageno, bucket))
        return false;

    uint32_t written;
    uint64_t offset;
    if (!writeValue(event, written, offset))
        return false;

    BUCKET_VAL_LEN(bpage_, bucket) = written;
    BUCKET_OFFSET(bpage_, bucket) = offset;

    io_.writeblock(pageno, bpage_);

    return true;
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

void Index::setKey(uint64_t bucket, const std::string& key)
{
    auto length = std::min(key.length(), MAX_KEY_LEN);
    memcpy(BUCKET_KEY(bpage_, bucket), key.c_str(), length);
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

bool Index::findSlot(const std::string& key, uint64_t& pageno, uint64_t& bucket)
{
    auto h = hash(key);
    pageno = h / BUCKETS_PER_PAGE;
    bucket = h % BUCKETS_PER_PAGE;

    io_.readblock(pageno, bpage_);

    std::string K;
    for (uint64_t i = 0; i < tablesize_ - 1; ++i) {
        K = getKey(bucket);
        if (K.length() == 0)
            return true;  // empty slot

        if (K == key)
            return false;  // already exists

        nextbucket(i, bucket, pageno);
    }

    return false;
}

bool Index::getBucket(const std::string& key, uint64_t& pageno, uint64_t& bucket)
{
    auto h = hash(key);
    pageno = h / BUCKETS_PER_PAGE;
    bucket = h % BUCKETS_PER_PAGE;

    io_.readblock(pageno, bpage_);

    std::string K;
    for (uint64_t i = 0; i < tablesize_ - 1; ++i) {
        K = getKey(bucket);
        if (K.length() == 0)
            return false;   // no hit

        if (K == key)
            return true;  // hit

        nextbucket(i, bucket, pageno);
    }

    return false;
}

bool Index::writeValue(const char* pval, int length, uint64_t& offset)
{
    if (pageno_ == 0) { // first data page
        pageno_ = nbpages_;
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

        io_.writeblock(pageno_, dpagew_);
        length -= save;
    }

    return true;
}

void Index::readVal(uint64_t offset, int length, std::string& value)
{
    auto pageno = offset / BlockIO::BLOCK_SIZE;
    auto poffset = static_cast<int>(offset % BlockIO::BLOCK_SIZE);

    std::ostringstream ss;

    for (auto read = 0; read < length; ) {
        io_.readblock(pageno, dpager_);

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
}

uint64_t Index::filesize()
{
    return io_.getFileSize();
}

uint64_t Index::tablesize() const
{
    return tablesize_;
}

float Index::loadfactor()
{
    auto filled = 0;

    uint64_t bucket = 0, pageno = 0;
    io_.readblock(pageno, bpage_);

    for (;;) {
        if (keyLength(bucket) != 0)
            filled++;

        if ((bucket = (bucket + 1) % BUCKETS_PER_PAGE) == 0) {  // next page
            if ((pageno = (pageno + 1) % nbpages_) == 0)
                break;  // wrapped

            io_.readblock(pageno, bpage_);
        }
    }

    return 100 * (filled / static_cast<float>(tablesize_));
}

uint64_t Index::maxrun()
{
    uint64_t maxrun = 0, bucket = 0, pageno = 0;

    io_.readblock(pageno, bpage_);

    std::string k;
    for (;;) {
        k = getKey(bucket);
        if (k.length() != 0) {
            maxrun = std::max(maxrun, runLength(k));
            io_.readblock(pageno, bpage_);  // must re-read block
        }

        if ((bucket = (bucket + 1) % BUCKETS_PER_PAGE) == 0) {  // next page
            if ((pageno = (pageno + 1) % nbpages_) == 0)
                break;  // wrapped

            io_.readblock(pageno, bpage_);
        }
    }

    return maxrun;
}

uint64_t Index::perm(uint64_t i) const
{
    auto perm = 1 + perm_[i];
    BOOST_ASSERT(1 <= perm && perm < tablesize_);
    return perm; // pseudo-random probing
}

void Index::nextbucket(uint64_t i, uint64_t& bucket, uint64_t& pageno)
{
    auto realbucket = BUCKETS_PER_PAGE * pageno + bucket;
    auto nextbucket = (realbucket + perm(i)) % tablesize_;
    auto nextpage = nextbucket / BUCKETS_PER_PAGE;
    bucket = nextbucket % BUCKETS_PER_PAGE;
    if (pageno != nextpage) {
        io_.readblock(nextpage, bpage_);
        pageno = nextpage;
    }
}

uint64_t Index::runLength(const std::string& key)
{
    auto h = hash(key);
    auto pageno = h / BUCKETS_PER_PAGE;
    auto bucket = h % BUCKETS_PER_PAGE;
    auto run = 1ULL;

    io_.readblock(pageno, bpage_);

    std::string K;
    for (uint64_t i = 0; i < tablesize_ - 1; ++i, ++run) {
        K = getKey(bucket);
        if (K.length() == 0)
            return 0;   // no hit

        if (K == key)
            break;  // hit

        nextbucket(i, bucket, pageno);
    }

    return run;
}

#include "stdafx.h"
#include "Index.h"
#include "Primes.h"
#include "DoubleHash.h"

// page type flags
#define PTF_BUCKET              (0)
#define PTF_DATA                (1 << 0)

// helper macros
#define IS_DATA_PAGE(p)         (p->header.flags & PTF_DATA)
#define SET_DATA_PAGE(p)        (p->header.flags |= PTF_DATA)
#define SET_BUCKET_PAGE(p)      (p->header.flags &= ~PTF_DATA)
#define BUCKET_KEY(p, b)        (&(p->buckets[b].key))
#define BUCKET_VAL_LEN(p, b)    (p->buckets[b].len)
#define BUCKET_OFFSET(p, b)     (p->buckets[b].offset)
#define DATA_PTR(p, o)          (&(p->data[o]))

// number of buckets on a page
constexpr auto BUCKETS_PER_PAGE = ((BlockIO::BLOCK_SIZE - sizeof(PageHeader)) / sizeof(Bucket));

Index::Index() : tablesize_(0), pageno_(0), offset_(0)
{
    bpage_ = static_cast<LPBUCKETPAGE>(mkblock());
    dpage_ = static_cast<LPDATAPAGE>(mkblock());
}

Index::~Index()
{
    freeblock(bpage_);
    freeblock(dpage_);
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

    auto objectId = event.getObjectId();

    auto length = std::min(objectId.length(), MAX_KEY_LEN);
    memcpy(BUCKET_KEY(bpage_, bucket), objectId.c_str(), length);

    uint32_t written;
    uint64_t offset;
    if (!writeValue(event, written, offset))
        return false;

    BUCKET_VAL_LEN(bpage_, bucket) = written;
    BUCKET_OFFSET(bpage_, bucket) = offset;

    return io_.writeblock(pageno, bpage_);
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
    auto objectId = event.getObjectId();
    auto h = doublehash<std::string>(objectId) % tablesize_;
    return h;
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
    memset(dpage_, 0, BlockIO::BLOCK_SIZE);
    SET_DATA_PAGE(dpage_);
    pageno_++;
    offset_ = 0;
}

int Index::available() const
{
    return static_cast<int>(BlockIO::BLOCK_SIZE - sizeof(PageHeader) - offset_);
}

bool Index::writeValue(const char* pval, int length, uint64_t& offset)
{
    if (pageno_ == 0) { // first data page
        pageno_ = tablesize_ / BUCKETS_PER_PAGE;
    }

    if (available() == 0) {
        newpage();
    }

    auto start_page = pageno_;
    auto start_offset = offset_;

    while (length > 0) {
        auto avail = available();
        if (avail == 0) {   // full page
            newpage();
            avail = BlockIO::BLOCK_SIZE - sizeof(PageHeader);
        }

        auto nlength = std::min(length, avail);
        auto save = nlength;

        auto ptr = DATA_PTR(dpage_, offset_);
        while (nlength > 0) {
            *ptr++ = *pval++;
            nlength--;
            offset_++;
        }

        if (!io_.writeblock(pageno_, dpage_))
            return false;

        length -= save;
    }

    // offset to beginning of value
    offset = start_page * BlockIO::BLOCK_SIZE + sizeof(PageHeader) + start_offset;

    return true;
}

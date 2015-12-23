#include "stdafx.h"
#include "Index.h"
#include "Primes.h"
#include "DoubleHash.h"

// page type flags
#define PTF_BUCKET          (0)
#define PTF_DATA		    (1 << 0)

// helper macros
#define IS_DATA_PAGE(p)		(p->header.flags & PTF_DATA)
#define SET_DATA_PAGE(p)	(p->header.flags |= PTF_DATA)
#define SET_BUCKET_PAGE(p)	(p->header.flags &= ~PTF_DATA)
#define PAGENO(p)		    (p->header.pageno)

// number of buckets on a page
constexpr auto BUCKETS_PER_PAGE = ((BlockIO::BLOCK_SIZE - sizeof(PageHeader)) / sizeof(Bucket));

Index::Index() : buckets_(0)
{
    bpage_ = static_cast<LPBUCKETPAGE>(mkblock());
}

Index::~Index()
{
    freeblock(bpage_);
    close();
}

void Index::open(const char* filename, OpenMode mode, uint32_t entries)
{
    close();

    buckets_ = Primes::prime(entries);

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

void Index::mktable()
{
    auto nblocks = buckets_ / BUCKETS_PER_PAGE;
    PAGENO(bpage_) = nblocks - 1;
    io_.writeblock(nblocks - 1, bpage_);
    io_.flush();
    io_.close();
}

void* Index::mkblock()
{
    return new char[BlockIO::BLOCK_SIZE]();
}

void Index::freeblock(void* block)
{
    delete[] block;
}

#pragma once

#include "BlockIO.h"

constexpr auto MAX_KEY_LEN = 40;

// ensure one byte alignment for structures below
#pragma pack (push, 1)

typedef struct PageHeader {
    uint8_t flags;	        // page flags
    uint64_t pageno;		// page number
    uint64_t nextpage;      // next page if linked
} *LPPAGEHEADER;

typedef struct Bucket {
    char key[MAX_KEY_LEN];  // bucket key
    uint64_t pageno;        // data page where value is stored
    uint64_t offset;        // offset into data page where value begins
} *LPBUCKET;

typedef struct BucketPage {
    PageHeader header;		// page header
    Bucket buckets[1];      // page data
} *LPBUCKETPAGE;

typedef struct DataPage {
    PageHeader header;      // page header
    char data[1];           // data page
} *LPDATAPAGE;

// restore default structure alignment
#pragma pack (pop)

class Index
{
public:
    Index();
    ~Index();

    void open(const char* filename,
        OpenMode mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc,
        uint32_t entries = DEFAULT_ENTRIES);
    void close();
private:
    void mktable();
    void* mkblock();
    void freeblock(void* block);

    static constexpr auto DEFAULT_ENTRIES = 10000UL;

    BlockIO io_;            // block i/o
    uint64_t buckets_;      // bucket count
    LPBUCKETPAGE bpage_;    // bucket page
};

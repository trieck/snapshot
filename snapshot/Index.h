#pragma once

#include "BlockIO.h"
#include "Event.h"
#include "EventWriter.h"

constexpr size_t MAX_KEY_LEN = 40UL;

// ensure one byte alignment for structures below
#pragma pack (push, 1)

typedef struct PageHeader {
    uint8_t flags;	        // page flags
} *LPPAGEHEADER;

typedef struct Bucket {
    char key[MAX_KEY_LEN];  // bucket key
    uint32_t len;           // value length
    uint64_t offset;        // offset to data page where value begins
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
    bool insert(const Event& event);
private:
    void mktable();
    void* mkblock();
    void freeblock(void* block);
    uint64_t hash(const Event& event);
    bool writeValue(const Event& event, uint32_t& written, uint64_t& offset);
    bool spill(const char* pval, int length);

    static constexpr auto DEFAULT_ENTRIES = 10000UL;

    BlockIO io_;            // block i/o
    uint64_t tablesize_;    // size of hash table
    uint64_t pageno_;       // current data page while writing
    uint64_t offset_;       // current offset in data page for writing
    LPBUCKETPAGE bpage_;    // bucket page
    LPDATAPAGE dpage_;      // data page
    EventWriter writer_;    // event writer
};

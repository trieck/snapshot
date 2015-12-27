#pragma once

#include "BlockIO.h"
#include "Event.h"
#include "EventWriter.h"
#include "RandomPerm.h"

constexpr size_t MAX_KEY_LEN = 40UL;

// ensure one byte alignment for structures below
#pragma pack (push, 1)

typedef struct Bucket {
    char key[MAX_KEY_LEN];  // bucket key
    uint8_t flags;          // bucket flags
    uint32_t len;           // value length
    uint64_t offset;        // offset to data page where value begins
} *LPBUCKET;

typedef struct BucketPage {
    Bucket buckets[1];      // page data
} *LPBUCKETPAGE;

typedef struct DataPage {
    char data[1];           // data page
} *LPDATAPAGE;

// restore default structure alignment
#pragma pack (pop)

class Index
{
public:
    Index();
    ~Index();

    void open(const char* filename, uint32_t entries = DEFAULT_ENTRIES);
    void close();
    std::string filename() const;
    bool find(const Event& event);
    bool find(const std::string& key);
    bool insert(const Event& event);
    bool lookup(const std::string& key, std::string& value);
    bool destroy(const Event& event);
    bool update(const Event& event);
    uint64_t filesize();
    uint64_t tablesize() const;
    float loadfactor();
    uint64_t maxrun();
private:
    void mktable();
    void* mkblock();
    void freeblock(void* block);
    uint64_t hash(const Event& event);
    uint64_t hash(const std::string& s);
    std::string getKey(uint64_t bucket);
    uint32_t keyLength(uint64_t bucket);
    void setKey(uint64_t bucket, const std::string& key);
    bool writeValue(const Event& event, uint32_t& written, uint64_t& offset);
    bool writeValue(const char* pval, int length, uint64_t& offset);
    void readVal(uint64_t offset, int length, std::string& value);
    void newpage();
    int available() const;
    bool findSlot(const std::string& key, uint64_t& pageno, uint64_t& bucket);
    bool getBucket(const std::string& key, uint64_t& pageno, uint64_t& bucket);
    uint64_t perm(uint64_t i) const;
    void nextbucket(uint64_t i, uint64_t& bucket, uint64_t& page);
    uint64_t runLength(const std::string& s);

    static constexpr auto DEFAULT_ENTRIES = 10000UL;

    std::string filename_;      // file name
    BlockIO io_;                // block i/o
    uint64_t tablesize_;        // size of hash table
    uint64_t nbpages_;          // number of bucket pages
    uint64_t pageno_;           // current data page while writing
    uint64_t offset_;           // current offset in data page while writing
    LPBUCKETPAGE bpage_;        // bucket page
    LPDATAPAGE dpagew_;         // write data page
    LPDATAPAGE dpager_;         // read data page
    EventWriter writer_;        // event writer
    RandomPerm perm_;           // random permutation for pseudo-random linear probing
};
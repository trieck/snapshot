#pragma once

#include <boost/uuid/sha1.hpp>
#include "BlockIO.h"
#include "Event.h"
#include "EventWriter.h"
#include "RandomPerm.h"

using digest_type = boost::uuids::detail::sha1::digest_type;
constexpr auto SHA1_DIGEST_INTS = sizeof(digest_type) / sizeof(uint32_t);

// ensure one byte alignment for structures below
#pragma pack (push, 1)

typedef struct Bucket {                 // hash table bucket
    uint32_t flags;                     // bucket flags
    uint64_t datum;                     // offset to datum
    uint32_t digest[SHA1_DIGEST_INTS];  // sha-1 digest
} *LPBUCKET;

typedef struct BucketPage {             // hash table page
    Bucket buckets[1];                  // hash table
} *LPBUCKETPAGE;

typedef struct Datum {                  // 512-byte datum
    uint64_t next;                      // next datum if linked
    uint64_t length;                    // total datum length
    char data[496];                     // first 496 bytes of data
} *LPDATUM;

typedef struct DataPage {               // data page
    Datum data[1];
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
    bool find(const std::string&key, Event& event);
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
    uint64_t hash(digest_type digest);
    void getDigest(uint64_t bucket, digest_type digest);
    void setKey(uint64_t bucket, const std::string& key);
    bool writeValue(const Event& event, uint64_t& offset);
    bool writeValue(const char* pval, int length, uint64_t& offset);
    bool updateValue(const Event& event, uint64_t offset);
    bool updateValue(const char* pval, int length, uint64_t offset);
    void readVal(uint64_t offset, std::string& value);
    void newpage();
    bool fullpage() const;
    uint64_t datumoffset();
    uint64_t datumoffset(uint64_t pageno, uint8_t datum) const;
    uint64_t nextdatumoffset() const;
    void newdatum();
    bool findSlot(const std::string& key, uint64_t& pageno, uint64_t& bucket);
    bool getBucket(const std::string& key, uint64_t& pageno, uint64_t& bucket);
    uint64_t perm(uint64_t i) const;
    void nextbucket(uint64_t i, uint64_t& bucket, uint64_t& page);
    uint64_t runLength(digest_type digest);
    bool isEqualDigest(digest_type d1, digest_type d2) const;

    static constexpr auto DEFAULT_ENTRIES = 10000UL;

    std::string filename_;      // file name
    BlockIO io_;                // block i/o
    uint64_t tablesize_;        // size of hash table
    uint64_t nbpages_;          // number of bucket pages
    uint64_t dpageno_;          // current data page while writing
    uint8_t ddatum_;            // current datum on data page while writing
    LPBUCKETPAGE bpage_;        // bucket page
    LPDATAPAGE dpagew_;         // write data page
    LPDATAPAGE dpager_;         // read data page
    EventWriter writer_;        // event writer
    RandomPerm perm_;           // random permutation for pseudo-random probing
};

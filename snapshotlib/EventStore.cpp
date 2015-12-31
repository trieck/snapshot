#include "snapshotlib.h"
#include "EventStore.h"
#include "Primes.h"
#include "fnvhash.h"
#include "sha1.h"

// bucket flags                 
#define BF_FILLED               (1 << 0)
#define BF_DELETED              (1 << 1)

// bucket macros
#define IS_EMPTY(p, b)          (!(p->buckets[b].flags & BF_FILLED))
#define IS_FILLED(p, b)         (p->buckets[b].flags & BF_FILLED)
#define SET_FILLED(p, b)        (p->buckets[b].flags |= BF_FILLED)
#define IS_DELETED(p, b)        (p->buckets[b].flags & BF_DELETED)
#define SET_DELETED(p, b)       (p->buckets[b].flags |= BF_DELETED)
#define BUCKET(p, b)            (p->buckets[b])
#define BUCKET_DIGEST(p, b)     (p->buckets[b].digest)
#define BUCKET_DATUM(p, b)      (p->buckets[b].datum)

// number of buckets on a page
constexpr auto BUCKETS_PER_PAGE = BlockIO::BLOCK_SIZE / sizeof(Bucket);

EventStore::EventStore() : tablesize_(0), nbpages_(0), fillcount_(0)
{
    page_ = static_cast<LPBUCKETPAGE>(BlockIO::mkblock());
}

EventStore::~EventStore()
{
    BlockIO::freeblock(page_);
    close();
}

void EventStore::open(const char* filename, uint32_t entries)
{
    close();
    mktable(filename, entries);
    repo_.open(std::tmpnam(nullptr));
}

void EventStore::mktable(const char* filename, uint32_t entries)
{
    tablesize_ = Primes::prime(entries);
    perm_.generate(tablesize_);
    nbpages_ = (tablesize_ / BUCKETS_PER_PAGE) + 1;
    io_.open(filename, std::ios::in | std::ios::out | std::ios::trunc);
    io_.writeblock(nbpages_ - 1, page_);
    io_.flush();
}

void EventStore::close()
{
    repo_.close();
    io_.close();
}

void EventStore::unlink()
{
    close();
    io_.unlink();
}

std::string EventStore::filename() const
{
    return io_.filename();
}

bool EventStore::find(const Event& event)
{
    return find(event.getObjectId());
}

bool EventStore::find(const std::string& key)
{
    uint64_t pageno, bucket;
    if (!getBucket(key, pageno, bucket))
        return false;

    if (IS_DELETED(page_, bucket))
        return false;

    return true;
}

bool EventStore::find(const std::string& key, Event& event)
{
    std::string value;
    if (!lookup(key, value))
        return false;

    Json::Reader reader;
    Json::Value oValue;
    if (!reader.parse(value, oValue, false)) {
        boost::format message = boost::format("can't parse event for object id: \"%s\".") % key;
        throw std::exception(message.str().c_str());
    }

    event = Event(oValue);

    return true;
}

bool EventStore::insert(const Event& event)
{
    auto key = event.getObjectId();

    uint64_t pageno, bucket;
    if (!findSlot(key, pageno, bucket))
        return false;

    setKey(bucket, key);

    uint64_t offset;
    repo_.writeEvent(event, offset);

    SET_FILLED(page_, bucket);
    BUCKET_DATUM(page_, bucket) = offset;

    fillcount_++;

    io_.writeblock(pageno, page_);

    if (isfull()) {
        resize();
    }

    return true;
}

uint64_t EventStore::hash(const Event& event)
{
    return hash(event.getObjectId());
}

uint64_t EventStore::hash(const std::string& s)
{
    uint32_t digest[SHA1_DIGEST_INTS];
    sha1(s, digest);
    return hash(digest);
}

uint64_t EventStore::hash(digest_type digest)
{
    return hash(digest, tablesize_);
}

uint64_t EventStore::hash(digest_type digest, uint64_t m)
{
    return fnvhash64(digest) % m;
}

bool EventStore::lookup(const std::string& key, std::string& value)
{
    value.clear();

    uint64_t pageno, bucket;
    if (!getBucket(key, pageno, bucket))
        return false;

    if (IS_DELETED(page_, bucket))
        return false;

    auto offset = BUCKET_DATUM(page_, bucket);

    repo_.readVal(offset, value);

    return true;
}

bool EventStore::destroy(const Event& event)
{
    auto key = event.getObjectId();

    uint64_t pageno, bucket;
    if (!getBucket(key, pageno, bucket))
        return false;

    SET_DELETED(page_, bucket) = 1;

    io_.writeblock(pageno, page_);

    return true;
}

bool EventStore::update(const Event& event)
{
    auto key = event.getObjectId();

    uint64_t pageno, bucket;
    if (!getBucket(key, pageno, bucket))
        return false;

    uint64_t offset = BUCKET_DATUM(page_, bucket);
    repo_.updateEvent(event, offset);

    return true;
}

void EventStore::getDigest(uint64_t bucket, digest_type digest)
{
    auto bdigest = BUCKET_DIGEST(page_, bucket);
    memcpy(digest, bdigest, sizeof(digest_type));
}

void EventStore::setKey(uint64_t bucket, const std::string& key)
{
    uint32_t digest[SHA1_DIGEST_INTS];
    sha1(key, digest);
    memcpy(BUCKET_DIGEST(page_, bucket), digest, sizeof(digest_type));
}

bool EventStore::findSlot(const std::string& key, uint64_t& pageno, uint64_t& bucket)
{
    uint32_t digest[SHA1_DIGEST_INTS];
    sha1(key, digest);

    return findSlot(digest, pageno, bucket);
}

bool EventStore::findSlot(digest_type digest, uint64_t& pageno, uint64_t& bucket)
{
    auto h = hash(digest);
    pageno = h / BUCKETS_PER_PAGE;
    bucket = h % BUCKETS_PER_PAGE;

    io_.readblock(pageno, page_);

    if (IS_EMPTY(page_, bucket))
        return true;

    uint32_t bdigest[SHA1_DIGEST_INTS];
    for (auto i = 0; i < tablesize_; ++i) {
        if (IS_EMPTY(page_, bucket))
            return true;    // empty slot

        getDigest(bucket, bdigest);
        if (isEqualDigest(bdigest, digest))
            return false;   // already exists

        nextbucket(i, bucket, pageno);
    }

    return false;
}

bool EventStore::isEqualDigest(digest_type d1, digest_type d2) const
{
    return memcmp(d1, d2, sizeof(digest_type)) == 0;
}

bool EventStore::isfull() const
{
    return fillcount_ >= (tablesize_ / 2);
}

void EventStore::resize()
{
    cout << "resizing..";

    EventStore store;
    auto newsize = static_cast<uint32_t>(tablesize_ * 2);
    store.mktable(std::tmpnam(nullptr), newsize);

    uint64_t bucket = 0, pageno = 0;
    io_.readblock(pageno, page_);

    for (;;) {
        if (IS_FILLED(page_, bucket)) {
            LPBUCKET pbucket = &BUCKET(page_, bucket);
            store.transfer(pbucket);
        }

        if ((++bucket %= BUCKETS_PER_PAGE) == 0) {  // next page
            if ((++pageno %= nbpages_) == 0)
                break;  // wrapped

            io_.readblock(pageno, page_);
        }
    }

    store.close();
    io_.unlink();
    io_.open(store.filename().c_str(), std::ios::in | std::ios::out);

    tablesize_ = store.tablesize_;
    nbpages_ = store.nbpages_;
    perm_ = store.perm_;

    cout << "complete." << endl;
}

bool EventStore::transfer(LPBUCKET pbucket)
{
    uint64_t pageno, bucket;
    if (!findSlot(pbucket->digest, pageno, bucket))
        return false;

    BUCKET(page_, bucket) = *pbucket;

    fillcount_++;

    io_.writeblock(pageno, page_);

    return true;
}

bool EventStore::getBucket(const std::string& key, uint64_t& pageno, uint64_t& bucket)
{
    auto h = hash(key);
    pageno = h / BUCKETS_PER_PAGE;
    bucket = h % BUCKETS_PER_PAGE;

    io_.readblock(pageno, page_);
    if (IS_EMPTY(page_, bucket))
        return false;	// no hit

    uint32_t bdigest[SHA1_DIGEST_INTS], kdigest[SHA1_DIGEST_INTS];
    sha1(key, kdigest);

    for (auto i = 0; i < tablesize_; ++i) {
        if (IS_EMPTY(page_, bucket))
            return false;   // no hit

        getDigest(bucket, bdigest);
        if (isEqualDigest(bdigest, kdigest))
            return true;   // hit

        nextbucket(i, bucket, pageno);
    }

    return false;
}

uint64_t EventStore::filesize()
{
    return io_.getFileSize();
}

uint64_t EventStore::tablesize() const
{
    return tablesize_;
}

uint64_t EventStore::fillcount() const
{
    return fillcount_;
}

float EventStore::loadfactor() const
{
    return 100 * (fillcount_ / static_cast<float>(tablesize_));
}

uint64_t EventStore::maxrun()
{
    uint64_t maxrun = 0, bucket = 0, pageno = 0;
    uint32_t digest[SHA1_DIGEST_INTS];

    io_.readblock(pageno, page_);

    for (;;) {
        if (IS_FILLED(page_, bucket)) {
            getDigest(bucket, digest);
            maxrun = std::max(maxrun, runLength(digest));
            io_.readblock(pageno, page_);  // must re-read block
        }

        if ((++bucket %= BUCKETS_PER_PAGE) == 0) {  // next page
            if ((++pageno %= nbpages_) == 0)
                break;  // wrapped

            io_.readblock(pageno, page_);
        }
    }

    return maxrun;
}

uint64_t EventStore::perm(uint64_t i) const
{
    return 1 + perm_[i];    // pseudo-random probing
}

void EventStore::nextbucket(uint64_t i, uint64_t& bucket, uint64_t& pageno)
{
    auto realbucket = BUCKETS_PER_PAGE * pageno + bucket;
    auto nextbucket = (realbucket + perm(i)) % tablesize_;
    auto nextpage = nextbucket / BUCKETS_PER_PAGE;
    bucket = nextbucket % BUCKETS_PER_PAGE;
    if (pageno != nextpage) {
        io_.readblock(nextpage, page_);
        pageno = nextpage;
    }
}

uint64_t EventStore::runLength(digest_type digest)
{
    auto h = hash(digest);
    auto pageno = h / BUCKETS_PER_PAGE;
    auto bucket = h % BUCKETS_PER_PAGE;

    io_.readblock(pageno, page_);
    if (IS_EMPTY(page_, bucket))
        return 0;

    auto run = 1ULL;
    for (auto i = 0; i < tablesize_; ++i, ++run) {
        if (IS_EMPTY(page_, bucket))
            break;

        nextbucket(i, bucket, pageno);
    }

    return run;
}
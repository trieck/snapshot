#include "snapshotlib.h"
#include "Index.h"
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
#define BUCKET_DIGEST(p, b)     (p->buckets[b].digest)
#define BUCKET_DATUM(p, b)      (p->buckets[b].datum)

// datum macros
#define DATUM_NEXT(p, d)        (p->data[d].next)
#define DATUM_LENGTH(p, d)      (p->data[d].length)
#define DATUM_PTR(p, d)         (p->data[d].data)

// number of buckets on a page
constexpr auto BUCKETS_PER_PAGE = BlockIO::BLOCK_SIZE / sizeof(Bucket);

Index::Index() : tablesize_(0), nbpages_(0), dpageno_(0), ddatum_(0)
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

    if (IS_DELETED(bpage_, bucket))
        return false;

    return true;
}

bool Index::find(const std::string& key, Event& event)
{
    std::string value;
    if (!lookup(key, value))
        return false;

    Json::Reader reader;
    Json::Value oValue;
    if (!reader.parse(value, oValue, false))
        return false;

    event = Event(oValue);

    return true;
}

bool Index::insert(const Event& event)
{
    auto key = event.getObjectId();

    uint64_t pageno, bucket;
    if (!findSlot(key, pageno, bucket))
        return false;

    setKey(bucket, key);

    uint64_t offset;
    if (!writeValue(event, offset))
        return false;

    SET_FILLED(bpage_, bucket);
    BUCKET_DATUM(bpage_, bucket) = offset;

    io_.writeblock(pageno, bpage_);

    return true;
}

uint64_t Index::hash(const Event& event)
{
    return hash(event.getObjectId());
}

uint64_t Index::hash(const std::string& s)
{
    uint32_t digest[SHA1_DIGEST_INTS];
    sha1(s, digest);
    return hash(digest);
}

uint64_t Index::hash(digest_type digest)
{
    return fnvhash64<digest_type>()(digest) % tablesize_;
}

bool Index::lookup(const std::string& key, std::string& value)
{
    value.clear();

    uint64_t pageno, bucket;
    if (!getBucket(key, pageno, bucket))
        return false;

    if (IS_DELETED(bpage_, bucket))
        return false;

    auto offset = BUCKET_DATUM(bpage_, bucket);

    readVal(offset, value);

    return true;
}

bool Index::destroy(const Event& event)
{
    auto key = event.getObjectId();

    uint64_t pageno, bucket;
    if (!getBucket(key, pageno, bucket))
        return false;

    SET_DELETED(bpage_, bucket) = 1;

    io_.writeblock(pageno, bpage_);

    return true;
}


bool Index::update(const Event& event)
{
    auto key = event.getObjectId();

    uint64_t pageno, bucket;
    if (!getBucket(key, pageno, bucket))
        return false;

    uint64_t offset = BUCKET_DATUM(bpage_, bucket);
    if (!updateValue(event, offset))
        return false;

    return true;
}

void Index::getDigest(uint64_t bucket, digest_type digest)
{
    auto bdigest = BUCKET_DIGEST(bpage_, bucket);
    memcpy(digest, bdigest, sizeof(digest_type));
}

void Index::setKey(uint64_t bucket, const std::string& key)
{
    uint32_t digest[SHA1_DIGEST_INTS];
    sha1(key, digest);
    memcpy(BUCKET_DIGEST(bpage_, bucket), digest, sizeof(digest_type));
}

void Index::mktable()
{
    io_.writeblock(nbpages_ - 1, bpage_);
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

bool Index::writeValue(const Event& event, uint64_t& offset)
{
    auto value = writer_.write(event);
    auto length = static_cast<int>(value.length());
    return writeValue(value.c_str(), length, offset);
}

bool Index::findSlot(const std::string& key, uint64_t& pageno, uint64_t& bucket)
{
    auto h = hash(key);
    pageno = h / BUCKETS_PER_PAGE;
    bucket = h % BUCKETS_PER_PAGE;

    io_.readblock(pageno, bpage_);

    if (IS_EMPTY(bpage_, bucket))
        return true;

    uint32_t bdigest[SHA1_DIGEST_INTS], kdigest[SHA1_DIGEST_INTS];
    sha1(key, kdigest);
    for (uint64_t i = 0; i < tablesize_ - 1; ++i) {
        if (IS_EMPTY(bpage_, bucket))
            return true;    // empty slot

        getDigest(bucket, bdigest);
        if (isEqualDigest(bdigest, kdigest))
            return false;   // already exists

        nextbucket(i, bucket, pageno);
    }

    return false;
}

bool Index::isEqualDigest(digest_type d1, digest_type d2) const
{
    return memcmp(d1, d2, sizeof(digest_type)) == 0;
}

bool Index::getBucket(const std::string& key, uint64_t& pageno, uint64_t& bucket)
{
    auto h = hash(key);
    pageno = h / BUCKETS_PER_PAGE;
    bucket = h % BUCKETS_PER_PAGE;

    io_.readblock(pageno, bpage_);
    if (IS_EMPTY(bpage_, bucket))
        return false;	// no hit

    uint32_t bdigest[SHA1_DIGEST_INTS], kdigest[SHA1_DIGEST_INTS];
    sha1(key, kdigest);

    for (uint64_t i = 0; i < tablesize_ - 1; ++i) {
        if (IS_EMPTY(bpage_, bucket))
            return false;   // no hit

        getDigest(bucket, bdigest);
        if (isEqualDigest(bdigest, kdigest))
            return true;   // hit

        nextbucket(i, bucket, pageno);
    }

    return false;
}

void Index::newpage()
{
    memset(dpagew_, 0, BlockIO::BLOCK_SIZE);
    dpageno_++;
    ddatum_ = 0;
}

bool Index::fullpage() const
{
    // is the current data page full?
    return ddatum_ == BlockIO::BLOCK_SIZE / sizeof(Datum);
}

uint64_t Index::datumoffset()
{
    if (dpageno_ == 0) {    // first data page
        dpageno_ = nbpages_;
    }

    if (fullpage()) {
        newpage();
    }

    return datumoffset(dpageno_, ddatum_);
}

uint64_t Index::datumoffset(uint64_t pageno, uint8_t datum) const
{
    return (pageno * BlockIO::BLOCK_SIZE) + (datum * sizeof(Datum));
}

uint64_t Index::nextdatumoffset() const
{
    auto pageno = dpageno_;
    auto ddatum = ddatum_ + 1;

    if (fullpage()) {
        pageno++;
        ddatum = 0;
    }

    return datumoffset(pageno, ddatum);
}

void Index::newdatum()
{
    ddatum_++;
    if (fullpage()) {
        io_.writeblock(dpageno_, dpagew_);
        newpage();
    }
}

bool Index::writeValue(const char* pval, int length, uint64_t& offset)
{
    offset = datumoffset();

    constexpr auto avail = static_cast<int>(sizeof(Datum::data));

    int written = 0;
    for (auto i = 0, written = 0; length > 0; ++i) {
        if (i > 0) {
            DATUM_NEXT(dpagew_, ddatum_) = nextdatumoffset();
            newdatum();
            written = 0;
        }

        auto nlength = std::min(length, avail);
        auto ptr = DATUM_PTR(dpagew_, ddatum_);
        while (nlength > 0) {
            *ptr++ = *pval++;
            nlength--;
            written++;
        }

        DATUM_LENGTH(dpagew_, ddatum_) = written;
        length -= written;
    }

    io_.writeblock(dpageno_, dpagew_);
    newdatum();

    return true;
}

bool Index::updateValue(const Event& event, uint64_t offset)
{
    auto value = writer_.write(event);
    auto length = static_cast<int>(value.length());
    return updateValue(value.c_str(), length, offset);
}

bool Index::updateValue(const char* pval, int length, uint64_t offset)
{
    uint64_t pageno = offset / BlockIO::BLOCK_SIZE;
    uint64_t datum = (offset - pageno * BlockIO::BLOCK_SIZE) / sizeof(Datum);
    io_.readblock(pageno, dpager_);

    constexpr auto avail = static_cast<int>(sizeof(Datum::data));

    for (auto i = 0, written = 0; length > 0; ++i) {
        if (i > 0) {
            if ((offset = DATUM_NEXT(dpager_, datum)) == 0) {
                DATUM_NEXT(dpager_, datum) = datumoffset();
                io_.writeblock(pageno, dpager_);
                return writeValue(pval, length, offset);
            } else {
                io_.writeblock(pageno, dpager_);
                pageno = offset / BlockIO::BLOCK_SIZE;
                datum = (offset - pageno * BlockIO::BLOCK_SIZE) / sizeof(Datum);
                io_.readblock(pageno, dpager_);
            }
            written = 0;
        }

        auto nlength = std::min(length, avail);
        auto ptr = DATUM_PTR(dpager_, datum);
        while (nlength > 0) {
            *ptr++ = *pval++;
            nlength--;
            written++;
        }

        nlength = std::max(0, static_cast<int>(DATUM_LENGTH(dpager_, datum) - written));
        while (nlength > 0) {   // clear leftover
            *ptr++ = '\0';
            nlength--;
        }

        DATUM_LENGTH(dpager_, datum) = written;
        length -= written;
    }

    io_.writeblock(pageno, dpager_);

    return true;
}

void Index::readVal(uint64_t offset, std::string& value)
{
    std::ostringstream ss;

    uint64_t pageno, datum;

    do {
        pageno = offset / BlockIO::BLOCK_SIZE;
        datum = (offset - pageno * BlockIO::BLOCK_SIZE) / sizeof(Datum);
        io_.readblock(pageno, dpager_);

        auto ptr = DATUM_PTR(dpager_, datum);
        auto length = static_cast<int>(DATUM_LENGTH(dpager_, datum));
        while (length > 0) {
            ss << *ptr++;
            length--;
        }
    } while ((offset = DATUM_NEXT(dpager_, datum)));

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
        if (IS_FILLED(bpage_, bucket))
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
    uint32_t digest[SHA1_DIGEST_INTS];

    io_.readblock(pageno, bpage_);

    for (;;) {
        if (IS_FILLED(bpage_, bucket)) {
            getDigest(bucket, digest);
            maxrun = std::max(maxrun, runLength(digest));
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

uint64_t Index::runLength(digest_type digest)
{
    auto h = hash(digest);
    auto pageno = h / BUCKETS_PER_PAGE;
    auto bucket = h % BUCKETS_PER_PAGE;

    io_.readblock(pageno, bpage_);
    if (IS_EMPTY(bpage_, bucket))
        return 0;

    auto run = 1ULL;
    for (uint64_t i = 0; i < tablesize_ - 1; ++i, ++run) {
        if (IS_EMPTY(bpage_, bucket))
            break;

        nextbucket(i, bucket, pageno);
    }

    return run;
}

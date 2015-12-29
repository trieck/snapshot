#include "snapshotlib.h"
#include "Repository.h"

// datum macros
#define DATUM_NEXT(p, d)        (p->data[d].next)
#define DATUM_LENGTH(p, d)      (p->data[d].length)
#define DATUM_PTR(p, d)         (p->data[d].data)

Repository::Repository() : dpageno_(0), ddatum_(0)
{
    dpager_ = static_cast<LPDATAPAGE>(BlockIO::mkblock());
    dpagew_ = static_cast<LPDATAPAGE>(BlockIO::mkblock());
}

Repository::~Repository()
{
    close();
    BlockIO::freeblock(dpager_);
    BlockIO::freeblock(dpagew_);
}

void Repository::open(const char* filename)
{
    filename_ = filename;
    io_.open(filename, std::ios::in | std::ios::out | std::ios::trunc);
}

void Repository::close()
{
    io_.close();
}

bool Repository::writeEvent(const Event& event, uint64_t& offset)
{
    auto value = writer_.write(event);
    auto length = static_cast<int>(value.length());
    return writeValue(value.c_str(), length, offset);
}

void Repository::newpage()
{
    memset(dpagew_, 0, BlockIO::BLOCK_SIZE);
    dpageno_++;
    ddatum_ = 0;
}

bool Repository::fullpage() const
{
    // is the current data page full?
    return ddatum_ == BlockIO::BLOCK_SIZE / sizeof(Datum);
}

uint64_t Repository::datumoffset()
{
    if (fullpage()) {
        newpage();
    }

    return datumoffset(dpageno_, ddatum_);
}

uint64_t Repository::datumoffset(uint64_t pageno, uint8_t datum) const
{
    return (pageno * BlockIO::BLOCK_SIZE) + (datum * sizeof(Datum));
}

uint64_t Repository::nextdatumoffset() const
{
    auto pageno = dpageno_;
    auto ddatum = ddatum_ + 1;

    if (fullpage()) {
        pageno++;
        ddatum = 0;
    }

    return datumoffset(pageno, ddatum);
}

void Repository::newdatum()
{
    ddatum_++;
    if (fullpage()) {
        io_.writeblock(dpageno_, dpagew_);
        newpage();
    }
}

bool Repository::writeValue(const char* pval, int length, uint64_t& offset)
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

bool Repository::updateEvent(const Event& event, uint64_t offset)
{
    auto value = writer_.write(event);
    auto length = static_cast<int>(value.length());
    return updateValue(value.c_str(), length, offset);
}

bool Repository::updateValue(const char* pval, int length, uint64_t offset)
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

void Repository::readVal(uint64_t offset, std::string& value)
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

#include "snapshotlib.h"
#include "Repository.h"

// datum macros
#define DATUM_NEXT(p, d)        (p->data[d].next)
#define DATUM_LENGTH(p, d)      (p->data[d].length)
#define DATUM_PTR(p, d)         (p->data[d].data)

auto constexpr DATUM_PER_PAGE = BlockIO::BLOCK_SIZE / sizeof(Datum);

Repository::Repository() : dpageno_(0), ddatum_(0)
{
    dpage_ = static_cast<LPDATAPAGE>(BlockIO::mkblock());
}

Repository::~Repository()
{
    close();
    BlockIO::freeblock(dpage_);
}

void Repository::open(const char* filename)
{
    filename_ = filename;
    io_.open(filename, std::ios::in | std::ios::out | std::ios::trunc);
    io_.writeblock(dpageno_, dpage_);
}

void Repository::close()
{
    io_.close();
}

void Repository::writeEvent(const Event& event, uint64_t& offset)
{
    auto value = writer_.write(event);
    auto length = static_cast<int>(value.length());
    writeValue(value.c_str(), length, offset);
}

void Repository::newpage()
{
    memset(dpage_, 0, BlockIO::BLOCK_SIZE);
    io_.writeblock(++dpageno_, dpage_);
}

uint64_t Repository::datumoffset()
{
    return datumoffset(dpageno_, ddatum_);
}

uint64_t Repository::datumoffset(uint64_t pageno, uint8_t datum) const
{
    return (pageno * BlockIO::BLOCK_SIZE) + (datum * sizeof(Datum));
}

uint64_t Repository::nextdatumoffset() const
{
    auto pageno = dpageno_;
    auto ddatum = ddatum_;

    if ((++ddatum %= DATUM_PER_PAGE) == 0) {
        pageno++;
    }

    return datumoffset(pageno, ddatum);
}

void Repository::newdatum()
{
    if ((++ddatum_ %= DATUM_PER_PAGE) == 0) {
        io_.writeblock(dpageno_, dpage_);
        newpage();
    }
}

void Repository::writeValue(const char* pval, int length, uint64_t& offset)
{
    offset = datumoffset();

    io_.readblock(dpageno_, dpage_);

    constexpr auto avail = static_cast<int>(sizeof(Datum::data));

    int written = 0;
    for (auto i = 0, written = 0; length > 0; ++i) {
        if (i > 0) {
            DATUM_NEXT(dpage_, ddatum_) = nextdatumoffset();
            newdatum();
            written = 0;
        }

        auto nlength = std::min(length, avail);
        auto ptr = DATUM_PTR(dpage_, ddatum_);
        while (nlength > 0) {
            *ptr++ = *pval++;
            nlength--;
            written++;
        }

        DATUM_LENGTH(dpage_, ddatum_) = written;
        length -= written;
    }

    io_.writeblock(dpageno_, dpage_);

    newdatum();
}

void Repository::updateEvent(const Event& event, uint64_t offset)
{
    auto value = writer_.write(event);
    auto length = static_cast<int>(value.length());
    updateValue(value.c_str(), length, offset);
}

void Repository::updateValue(const char* pval, int length, uint64_t offset)
{
    uint64_t pageno = offset / BlockIO::BLOCK_SIZE;
    uint64_t datum = (offset - pageno * BlockIO::BLOCK_SIZE) / sizeof(Datum);

    io_.readblock(pageno, dpage_);

    constexpr auto avail = static_cast<int>(sizeof(Datum::data));

    for (auto i = 0, written = 0; length > 0; ++i) {
        if (i > 0) {
            if ((offset = DATUM_NEXT(dpage_, datum)) == 0) {
                DATUM_NEXT(dpage_, datum) = datumoffset();
                io_.writeblock(pageno, dpage_);
                writeValue(pval, length, offset);
                return;
            } else {
                io_.writeblock(pageno, dpage_);
                pageno = offset / BlockIO::BLOCK_SIZE;
                datum = (offset - pageno * BlockIO::BLOCK_SIZE) / sizeof(Datum);
                io_.readblock(pageno, dpage_);
            }
            written = 0;
        }

        auto nlength = std::min(length, avail);
        auto ptr = DATUM_PTR(dpage_, datum);
        while (nlength > 0) {
            *ptr++ = *pval++;
            nlength--;
            written++;
        }

        nlength = std::max(0, static_cast<int>(DATUM_LENGTH(dpage_, datum) - written));
        while (nlength > 0) {   // clear leftover
            *ptr++ = '\0';
            nlength--;
        }

        DATUM_LENGTH(dpage_, datum) = written;
        length -= written;
    }

    io_.writeblock(pageno, dpage_);
}

void Repository::readVal(uint64_t offset, std::string& value)
{
    std::ostringstream ss;

    uint64_t pageno, datum;

    do {
        pageno = offset / BlockIO::BLOCK_SIZE;
        datum = (offset - pageno * BlockIO::BLOCK_SIZE) / sizeof(Datum);
        io_.readblock(pageno, dpage_);

        auto ptr = DATUM_PTR(dpage_, datum);
        auto length = static_cast<int>(DATUM_LENGTH(dpage_, datum));
        while (length > 0) {
            ss << *ptr++;
            length--;
        }
    } while ((offset = DATUM_NEXT(dpage_, datum)));

    value = ss.str();
}

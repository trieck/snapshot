#include "snapshotlib.h"
#include "Repository.h"
#include "ByteBuffer.h"
#include "EventBuffer.h"

// datum macros
#define DATUM_NEXT(p, d)            (p->data[d].next)
#define DATUM_TOTAL_LENGTH(p, d)    (p->data[d].totalLength)
#define DATUM_LENGTH(p, d)          (p->data[d].length)
#define DATUM_PTR(p, d)             (p->data[d].data)

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
    io_.open(filename, std::ios::in | std::ios::out | std::ios::trunc);
    io_.writeblock(dpageno_, dpage_);
}

void Repository::close()
{
    io_.close();
}

void Repository::_unlink()
{
    close();
    io_._unlink();
}

void Repository::writeEvent(const Event& event, uint64_t& offset)
{
    EventBufferPtr buffer = EventBuffer::makeBuffer(event);
    writeValue(*buffer, buffer->size(), buffer->size(), offset);
}

void Repository::writeValue(const uint8_t* bytes, int totalLength, int length, uint64_t& offset)
{
    offset = datumoffset();

    io_.readblock(dpageno_, dpage_);

    constexpr auto avail = static_cast<int>(sizeof(Datum::data));

    for (auto i = 0; length > 0; ++i) {
        if (i > 0) {
            DATUM_NEXT(dpage_, ddatum_) = nextdatumoffset();
            newdatum();
        }

        auto written = 0, nlength = std::min(length, avail);
        auto* ptr = DATUM_PTR(dpage_, ddatum_);
        while (nlength > 0) {
            *ptr++ = *bytes++;
            nlength--;
            written++;
        }

        DATUM_TOTAL_LENGTH(dpage_, ddatum_) = static_cast<uint32_t>(totalLength);
        DATUM_LENGTH(dpage_, ddatum_) = static_cast<uint32_t>(written);
        length -= written;
    }

    io_.writeblock(dpageno_, dpage_);

    newdatum();
}

void Repository::newpage()
{
    memset(dpage_, 0, BlockIO::BLOCK_SIZE);
    io_.writeblock(++dpageno_, dpage_);
}

uint64_t Repository::datumoffset() const
{
    return datumoffset(dpageno_, ddatum_);
}

uint64_t Repository::datumoffset(uint64_t pageno, uint8_t datum)
{
    return (pageno * BlockIO::BLOCK_SIZE) + (datum * sizeof(Datum));
}

uint64_t Repository::nextdatumoffset() const
{
    auto pageno = dpageno_;
    auto ddatum = ddatum_;

    if ((ddatum = static_cast<uint8_t>((ddatum + 1) % DATUM_PER_PAGE)) == 0) {
        pageno++;
    }

    return datumoffset(pageno, ddatum);
}

void Repository::newdatum()
{
    if ((ddatum_ = static_cast<uint8_t>((ddatum_ + 1) % DATUM_PER_PAGE)) == 0) {
        io_.writeblock(dpageno_, dpage_);
        newpage();
    }
}

void Repository::updateEvent(const Event& event, uint64_t offset)
{
    EventBufferPtr buffer = EventBuffer::makeBuffer(event);
    updateValue(*buffer, buffer->size(), offset);
}

void Repository::updateValue(const uint8_t* bytes, int totalLength, uint64_t offset)
{
    uint64_t pageno = offset / BlockIO::BLOCK_SIZE;
    uint64_t datum = (offset - pageno * BlockIO::BLOCK_SIZE) / sizeof(Datum);

    io_.readblock(pageno, dpage_);

    constexpr auto avail = static_cast<int>(sizeof(Datum::data));

    for (auto i = 0, length = totalLength; length > 0; ++i) {
        if (i > 0) {
            if ((offset = DATUM_NEXT(dpage_, datum)) == 0) {
                DATUM_NEXT(dpage_, datum) = datumoffset();
                io_.writeblock(pageno, dpage_);
                writeValue(bytes, totalLength, length, offset);
                return;
            } else {
                io_.writeblock(pageno, dpage_);
                pageno = offset / BlockIO::BLOCK_SIZE;
                datum = (offset - pageno * BlockIO::BLOCK_SIZE) / sizeof(Datum);
                io_.readblock(pageno, dpage_);
            }
        }

        auto written = 0, nlength = std::min(length, avail);
        auto* ptr = DATUM_PTR(dpage_, datum);
        while (nlength > 0) {
            *ptr++ = *bytes++;
            nlength--;
            written++;
        }

        nlength = std::max(0, static_cast<int>(DATUM_LENGTH(dpage_, datum) - written));
        while (nlength > 0) {   // clear leftover
            *ptr++ = '\0';
            nlength--;
        }

        DATUM_TOTAL_LENGTH(dpage_, datum) = static_cast<uint32_t>(totalLength);
        DATUM_LENGTH(dpage_, datum) = static_cast<uint32_t>(written);
        if ((length -= written) == 0) {
            DATUM_NEXT(dpage_, datum) = 0;
        }
    }

    io_.writeblock(pageno, dpage_);
}

void Repository::readVal(uint64_t offset, EventBufferPtr& event)
{
    uint64_t pageno, datum;

    pageno = offset / BlockIO::BLOCK_SIZE;
    datum = (offset - pageno * BlockIO::BLOCK_SIZE) / sizeof(Datum);
    io_.readblock(pageno, dpage_);

    ByteBuffer buffer(DATUM_TOTAL_LENGTH(dpage_, datum));

    for (; ;) {
        auto* ptr = DATUM_PTR(dpage_, datum);
        auto length = static_cast<int>(DATUM_LENGTH(dpage_, datum));

        while (length > 0) {
            buffer << *ptr++;
            length--;
        }

        if ((offset = DATUM_NEXT(dpage_, datum)) == 0)
            break;

        pageno = offset / BlockIO::BLOCK_SIZE;
        datum = (offset - pageno * BlockIO::BLOCK_SIZE) / sizeof(Datum);
        io_.readblock(pageno, dpage_);
    }

    event = EventBuffer::makeBuffer(buffer);
}


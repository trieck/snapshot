#pragma once

#include "BlockIO.h"
#include "Event.h"

// ensure one byte alignment for structures below
#pragma pack (push, 1)

typedef struct Datum {                  // 512-byte datum
    uint64_t next;                      // next datum if linked
    uint32_t totalLength;               // total length of datum
    uint32_t length;                    // length of this datum segment
    uint8_t data[496];                  // first 496 bytes of data
} *LPDATUM;

typedef struct DataPage {               // data page
    Datum data[1];
} *LPDATAPAGE;

// restore default structure alignment
#pragma pack (pop)

class Repository
{
public:
    Repository();
    ~Repository();

    void open(const char* filename);
    void close();
    void unlink();

    void writeEvent(const Event& event, uint64_t& offset);
    void updateEvent(const Event& event, uint64_t offset);
    void readVal(uint64_t offset, EventBufferPtr& event);

private:
    void writeValue(const uint8_t* bytes, int totalLength, int length, uint64_t& offset);
    void updateValue(const uint8_t* bytes, int totalLength, uint64_t offset);

    void newpage();
    uint64_t datumoffset() const;
    static uint64_t datumoffset(uint64_t pageno, uint8_t datum);
    uint64_t nextdatumoffset() const;
    void newdatum();

    BlockIO io_;                // block i/o
    uint64_t dpageno_;          // current data page while writing
    uint8_t ddatum_;            // current datum on data page while writing
    LPDATAPAGE dpage_;          // data page
};

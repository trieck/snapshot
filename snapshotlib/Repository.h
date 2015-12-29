#pragma once

#include "BlockIO.h"
#include "Event.h"
#include "EventWriter.h"

// ensure one byte alignment for structures below
#pragma pack (push, 1)

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

class Repository
{
public:
    Repository();
    ~Repository();

    void open(const char* filename);
    void close();

    bool writeEvent(const Event& event, uint64_t& offset);
    bool updateEvent(const Event& event, uint64_t offset);
    void readVal(uint64_t offset, std::string& value); 

private:
    bool writeValue(const char* pval, int length, uint64_t& offset);
    bool updateValue(const char* pval, int length, uint64_t offset);
    
    void newpage();
    bool fullpage() const;
    uint64_t datumoffset();
    uint64_t datumoffset(uint64_t pageno, uint8_t datum) const;
    uint64_t nextdatumoffset() const;
    void newdatum();

    std::string filename_;      // file name
    BlockIO io_;                // block i/o
    EventWriter writer_;        // event writer
    uint64_t dpageno_;          // current data page while writing
    uint8_t ddatum_;            // current datum on data page while writing
    LPDATAPAGE dpager_;         // read data page
    LPDATAPAGE dpagew_;         // write data page
};

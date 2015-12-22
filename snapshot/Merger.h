#pragma once

#include "Partition.h"

struct mergerec {
    mergerec() : key(0), stream(nullptr) {}
    ~mergerec() { stream->close(); }
    uint64_t key;
    Partition* stream;
};

class Merger
{
public:
    Merger(const std::string& key);
    ~Merger();

    PartitionPtr merge(const PartitionVec& vec);
private:
    size_t countpasses(size_t argc);
    PartitionPtr mergeonce(size_t argc, PartitionVec::const_iterator it);
    PartitionPtr mergemany(size_t argc, PartitionVec::const_iterator it);
    bool read(mergerec** recs) const;
    mergerec** least(mergerec** recs);
    bool write(std::ostream& out, mergerec** recs);

    std::string key_;   // partition key
    size_t pass_;       // count down of pass number
    mergerec **array;	// array for least function
};


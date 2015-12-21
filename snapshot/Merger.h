#pragma once

#include "Partition.h"

struct mergerec {
    uint64_t key;
    std::ifstream stream;
};

class Merger
{
public:
    Merger(const std::string& key);
    ~Merger();

    std::string merge(const PartitionVec& vec);

private:
    void close();
    static stringvec transform(const PartitionVec& vec);

    size_t countpasses(size_t argc);
    std::string mergeonce(size_t argc, stringvec::const_iterator it);
    std::string mergemany(size_t argc, stringvec::const_iterator it);
    bool read(mergerec** recs) const;
    mergerec** least(mergerec** recs);
    bool write(mergerec** recs);

    std::string key_;
    size_t pass_;       // count down of pass number
    std::ofstream out_; // output stream
    mergerec **array;	// array for least function
};


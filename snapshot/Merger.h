#pragma once

#include "Partition.h"

class Merger
{
public:
    Merger(const std::string& key);
    ~Merger();

    std::string merge(const PartitionVec& vec);

private:
    size_t countpasses(size_t argc);
    std::string mergemany(size_t argc, PartitionVec::const_iterator it);

    std::string key_;
    size_t pass_;     // count down of pass number
};


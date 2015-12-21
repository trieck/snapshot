#include "stdafx.h"
#include "Merger.h"

namespace {
    const size_t NWAY = 100;
}

Merger::Merger(const std::string & key) : key_(key)
{
    pass_ = 0;
}

Merger::~Merger()
{
}

std::string Merger::merge(const PartitionVec& vec)
{
    pass_ = countpasses(vec.size());
    return mergemany(vec.size(), vec.begin());
}

size_t Merger::countpasses(size_t argc)
{
    uint32_t i = 0;

    if (argc <= NWAY)
        return 1;

    while (argc > 0) {
        i++;
        argc -= std::min(argc, NWAY);
    }

    return i + countpasses(i);
}

std::string Merger::mergemany(size_t argc, PartitionVec::const_iterator it)
{
    return std::string();
}

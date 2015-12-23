#pragma once

#include "Index.h"
#include "Partition.h"

class SnapshotTree
{
public:
    SnapshotTree();
    ~SnapshotTree();

    void load(Partition* partition);

    void process(const std::string& objectId, const std::string& event);

private:
    Index index_;
};


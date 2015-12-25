#pragma once

#include "Index.h"
#include "Partition.h"

class SnapshotTree
{
public:
    SnapshotTree();
    ~SnapshotTree();

    void load(Partition* partition);
    void process(const Event& event);

private:
    void insert(const Event& event);
    void destroy(const Event& event);
    void update(const Event& event);
    Index index_;
};


#pragma once

#include "Event.h"
#include "Partitioner.h"

class Snapshotter
{
public:
    Snapshotter();
    ~Snapshotter();

    void snapshot(const char* file);
private:
    void partition(const char* file);
    void merge();
    void insert(const Event& event);
    void snapshot();
    void snapshot(const PartitionPtr& partition);

    Partitioner partitioner_;
    PartitionVec partitions_;   // merged set
};

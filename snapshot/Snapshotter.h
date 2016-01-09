#pragma once

#include "Event.h"
#include "Partitioner.h"

class Snapshotter
{
public:
    Snapshotter();
    ~Snapshotter();

    void snapshot(const char* infile, const char* outfile);
private:
    void partition(const char* infile);
    void merge();
    void insert(const Event& event);
    void snapshot(const char* outfile);
    static void snapshot(const PartitionPtr& partition, std::ostream& os);

    Partitioner partitioner_;
    PartitionVec partitions_;   // merged set
};

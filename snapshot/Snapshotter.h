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
    void insert(const Event& event);
    void merge();

    Partitioner partitioner_;
};

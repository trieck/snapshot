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
    void partition(const Event& event);

    Partitioner _partitioner;
};

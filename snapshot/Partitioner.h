#pragma once

#include "Event.h"

class Partitioner
{
public:
    Partitioner();
    ~Partitioner();

    void partition(const Event& event);
private:
    void alloc();

    uint32_t lookup(const Event& event);
    static std::string getKey(const Event& event);

    char **record_; // array of pointers to partitions
    char *pool_;    // allocation pool
    char *ppool_;   // current allocation pointer

    uint32_t size_; // total records in table
    uint32_t count_;
};


#pragma once

#include "Event.h"

class Partition
{
private:
    Partition(const std::string& key);
public:
    ~Partition();
    void write(const std::vector<Event> &vec);
    static std::unique_ptr<Partition> makePartition(const std::string& key);
private:
    std::string key_;   // key
    std::string name_;  // file name
};


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

    std::string getFilename() const;

private:
    std::string key_;   // key
    std::string name_;  // file name
};

typedef std::vector<std::unique_ptr<Partition>> PartitionVec;


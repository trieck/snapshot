#pragma once

#include "Event.h"

class Partition;
using PartitionPtr = std::unique_ptr<Partition>;
using OpenMode = std::ios::openmode;

class Partition
{
private:
    Partition(const std::string& key);
    Partition(const std::string& key, const std::string& name);
public:
    ~Partition();
    void open(OpenMode mode);
    void close();
    void write(const std::vector<Event> &vec);
    static PartitionPtr makePartition(const std::string& key);
    static PartitionPtr makePartition(const std::string& key, const std::string& name);

    std::string getKey() const;
    std::string getFilename() const;
    std::iostream& stream();
private:
    std::fstream stream_;   // stream
    std::string key_;       // key
    std::string name_;      // file name
};

using PartitionVec = std::vector<PartitionPtr>;


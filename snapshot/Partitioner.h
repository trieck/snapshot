#pragma once

#include "Event.h"
#include "EventWriter.h"
#include "Partition.h"

class Partitioner
{
public:
    Partitioner();
    ~Partitioner();

    void partition(const Event& event);
    bool isfull() const;
    void flush();
    PartitionVec merge();

private:
    EventVec& lookup(const Event& event);
    static std::string getKey(const Event& event);

    void flush(const std::string& key, std::vector<Event>& value);
    void sort(EventVec& vec) const;

    PartitionVec& getPartitions(const std::string& key);

    EventWriter writer_;
    uint32_t count_;
    std::unordered_map<std::string, EventVec> map_;
    std::unordered_map<std::string, PartitionVec> partitions_;
};


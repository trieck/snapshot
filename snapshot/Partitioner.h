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
    void merge();

private:
    typedef std::vector<Event> EventVec;
    std::unordered_map<std::string, EventVec> map_;
    
    EventVec& lookup(const Event& event);
    static std::string getKey(const Event& event);

    void flush(const std::string& key, std::vector<Event>& value);
    void sort(std::vector<Event>& vec);

    EventWriter writer_;
    uint32_t count_;
    std::vector<std::unique_ptr<Partition>> partitions_;
};


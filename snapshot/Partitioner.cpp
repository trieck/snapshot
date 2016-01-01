#include "stdafx.h"
#include "Partitioner.h"
#include "Merger.h"
#include "radixsort.h"

namespace { const auto MAX_EVENTS = 1000; }

struct EventPred : public std::unary_function<const Event&, bool> {
    typedef uint64_t KEY_TYPE;
    KEY_TYPE bit_;

    EventPred(KEY_TYPE bit) : bit_(bit) {}
    inline bool operator() (const Event& event) const {
        auto num = event.getSequenceNumber();
        return !(num & (1ULL << bit_));
    }
};

Partitioner::Partitioner() : count_(0)
{
}

Partitioner::~Partitioner()
{
}

void Partitioner::partition(const Event& event)
{
    EventVec& vec = lookup(event);
    vec.push_back(event);
    count_++;
}

bool Partitioner::isfull() const
{
    return count_ >= MAX_EVENTS;
}

void Partitioner::flush()
{
    if (map_.size() == 0)
        return;

    for (auto& p : map_) {
        flush(p.first, p.second);
    }

    map_.clear();
    count_ = 0;
}

PartitionVec Partitioner::merge()
{
    PartitionVec output;

    flush();

    PartitionPtr partition;

    for (auto& pair : partitions_) {
        Merger merger(pair.first);
        output.push_back(merger.merge(pair.second));
    }

    partitions_.clear();

    return output;
}

void Partitioner::flush(const std::string& key, std::vector<Event>& vec)
{
    sort(vec);

    auto partition = Partition::makePartition(key);
    partition->write(vec);

    auto& partitions = getPartitions(key);
    partitions.push_back(std::move(partition));
}

void Partitioner::sort(EventVec& vec)
{
    radixsort<EventPred>(vec.begin(), vec.end());
}

PartitionVec& Partitioner::getPartitions(const std::string& key)
{
    auto it = partitions_.find(key);
    if (it == partitions_.end()) {
        return partitions_[key] = PartitionVec();
    } else {
        return (*it).second;
    }
}

EventVec& Partitioner::lookup(const Event& event)
{
    std::string key = getKey(event);

    auto it = map_.find(key);
    if (it == map_.end()) {
        return map_[key] = EventVec();
    } else {
        return (*it).second;
    }
}

std::string Partitioner::getKey(const Event& event)
{
    std::ostringstream ss;

    ss << event["SESSION_ID"].asString();
    auto root = event.getMeta("RootWindowObjectId");
    if (!root.isNull()) {
        ss << '|' << root.asString();
    }

    return ss.str();
}


#include "stdafx.h"
#include "Partitioner.h"
#include "radixsort.h"

namespace {
    const auto MAX_EVENTS = 1000;

    struct EventPred : public std::unary_function<const Event&, bool> {
        uint64_t bit_;

        EventPred(uint64_t bit) : bit_(bit) {}
        inline bool operator() (const Event& event) const {
            auto sequenceNumber = event.getSequenceNumber();
            return !(sequenceNumber & (1ULL << bit_));
        }
    };
}

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
    cout << "flushing to disk...";

    for (auto& p : map_) {
        flush(p.first, p.second);
    }

    map_.clear();
    count_ = 0;

    cout << "flushed." << endl;
}

void Partitioner::merge()
{
    //   for (auto& p : partitions_) {
   //
     //  }
}

void Partitioner::flush(const std::string& key, std::vector<Event>& vec)
{
    sort(vec);

    std::unique_ptr<Partition> partition = Partition::makePartition(key);
    partition->write(vec);
    partitions_.push_back(std::move(partition));
}

void Partitioner::sort(std::vector<Event>& vec)
{
    radixsort<uint64_t, EventPred>(vec.begin(), vec.end());
}

Partitioner::EventVec& Partitioner::lookup(const Event& event)
{
    std::string key = getKey(event);

    auto it = map_.find(key);
    if (it == map_.end()) {
        return map_[key] = EventVec();
    }
    else {
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


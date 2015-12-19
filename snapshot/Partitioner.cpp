#include "stdafx.h"
#include "Partitioner.h"
#include "util.h"
#include "Primes.h"

namespace {

    // maximum memory size for records
    constexpr auto MEM_SIZE = 67108864L;

    // maximum pool size
    constexpr auto MAX_POOL = 2 * MEM_SIZE;

    // size of pool
    constexpr auto POOL_SIZE = 3 * MEM_SIZE;

    // average size of a record
    constexpr auto AVERAGE_SIZE = 1024L;

    // maximum number of records
    constexpr auto MAX_COUNT = MEM_SIZE / AVERAGE_SIZE;

    constexpr auto FILL_RATIO = 2;
};

Partitioner::Partitioner()
{
    record_ = nullptr;
    pool_ = ppool_ = nullptr;
    size_ = count_ = 0;
}

Partitioner::~Partitioner()
{
    delete[] record_;
    delete[] pool_;
}

void Partitioner::partition(const Event& event)
{
    if (record_ == nullptr)
        alloc();

    auto index = lookup(event);
}

uint32_t Partitioner::lookup(const Event& event)
{
    std::string key = getKey(event);

    uint32_t i = std::hash<std::string>()(key) % size_;

    while (record_[i] && strcmp(record_[i], key.c_str()))
        i = (i + 1) % size_;

    return i;
}

std::string Partitioner::getKey(const Event& event)
{
    std::ostringstream ss;

    ss << event["SESSION_ID"].asString();
    auto root = event.getMeta("RootWindowObjectId");
    if (!root.isNull()) {
        ss << '|' << root.asString();
    }

    return Util::sha1(ss.str());
}

void Partitioner::alloc()
{
    size_ = (uint32_t)Primes::prime(FILL_RATIO * MAX_COUNT);
    count_ = 0;

    record_ = new char*[size_];
    memset(record_, 0, size_ * sizeof(char*));

    ppool_ = pool_ = new char[POOL_SIZE];
}
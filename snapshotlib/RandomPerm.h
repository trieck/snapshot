#pragma once

#include <random>

class RandomPerm
{
public:
    RandomPerm();
    ~RandomPerm();

    void generate(uint64_t n);
    uint64_t size() const {
        return n_;
    }
    uint64_t operator[](uint64_t index) const;

private:
    void free();
    uint64_t uniform(uint64_t i);
    std::mt19937 generator_;
    uint64_t* table_;
    uint64_t n_;
};


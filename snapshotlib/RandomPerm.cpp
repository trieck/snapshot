#include "snapshotlib.h"
#include "RandomPerm.h"

RandomPerm::RandomPerm() : table_(nullptr), n_(0)
{
}

RandomPerm::RandomPerm(const RandomPerm& rhs)
{
    *this = rhs;
}

RandomPerm::~RandomPerm()
{
    free();
}

RandomPerm& RandomPerm::operator=(const RandomPerm& rhs)
{
    free();

    n_ = rhs.n_;
    table_ = new uint64_t[n_];
    memcpy(table_, rhs.table_, sizeof(uint64_t)* n_);

    return *this;
}

void RandomPerm::generate(uint64_t n)
{
    free();
    table_ = new uint64_t[n_ = n];

    std::random_device rd;
    generator_.seed(rd());

    for (uint64_t i = 0, j; i < n_; ++i) {
        j = uniform(i);
        table_[i] = table_[j];
        table_[j] = i;
    }
}

uint64_t RandomPerm::operator[](uint64_t index) const
{
    if (index >= n_)
        throw std::out_of_range("permutation index out of range.");

    return table_[index];
}

void RandomPerm::free()
{
    delete[] table_;
    table_ = nullptr;
}

uint64_t RandomPerm::uniform(uint64_t i)
{
    std::uniform_int_distribution<uint64_t> dis(0, i);
    return dis(generator_);
}

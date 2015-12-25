#include "snapshotlib.h"
#include "Primes.h"

namespace {
    /////////////////////////////////////////////////////////////////////////////
    // The delta table generates primes just less than a power of two.  A prime
    // number is calculated by subtracting its delta value from its 
    // corresponding power of two.
    const uint8_t delta[] = {
        0, 0, 1, 3, 3, 1, 3, 1, 5, 3,
        3, 9, 3, 1, 3, 19, 15, 1, 5, 1,
        3, 9, 3, 15, 3, 39, 5, 39, 57, 3,
        35, 1, 5, 9, 41, 31, 5,
        25, 45, 7,
        87, 21, 11, 57, 17, 55, 21, 115, 59, 81,
        27,
        129, 47, 111, 33, 55, 5, 13, 27, 55,
        93, 1, 57, 25, 59
    };
};

Primes::Primes()
{
}

Primes::~Primes()
{
}

uint64_t Primes::prime(uint64_t i)
{
    auto j = 1;

    while (i >>= 1)
        j++;

    return (1i64 << j) - delta[j];
}

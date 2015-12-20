#pragma once

template <typename KeyType, typename Pred, typename It>
void radixsort(It first, It last)
{
    int msb = sizeof(KeyType) * CHAR_BIT - 1;
    radixsort_msb<KeyType, Pred>(first, last, msb);
}

template <typename KeyType, typename Pred, typename It>
void radixsort_msb(It first, It last, int msb)
{
    if (first != last && msb >= 0) {
        auto mid = std::partition(first, last, Pred(msb));
        msb--;
        radixsort_msb<KeyType, Pred>(first, mid, msb);
        radixsort_msb<KeyType, Pred>(mid, last, msb);
    }
}

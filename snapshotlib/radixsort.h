#pragma once

template<typename Pred, typename It>
void radixsort_msb(It first, It last, int msb)
{
    if (first != last && msb >= 0) {
        auto mid = std::partition(first, last, Pred(msb));
        msb--;
        radixsort_msb<Pred>(first, mid, msb);
        radixsort_msb<Pred>(mid, last, msb);
    }
}

template<typename Pred, typename It>
void radixsort(It first, It last)
{
    auto msb = static_cast <int>(sizeof(typename Pred::KEY_TYPE) * CHAR_BIT - 1);
    radixsort_msb<Pred>(first, last, msb);
}

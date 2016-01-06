#pragma once

inline size_t _fnvhash64(const void *k, size_t len) {
    size_t i, hash;

    auto key = static_cast<const uint8_t*>(k);
    for (hash = 0, i = 0; i < len; ++i) {
        hash ^= key[i]; 
        hash *= 1099511628211;
    }

    return hash;
}

template<typename T>
size_t fnvhash64(const T& key) {
    return _fnvhash64(&key, sizeof(T));
}

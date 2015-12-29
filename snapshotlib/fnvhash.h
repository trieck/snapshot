#pragma once

size_t _fnvhash64(const void *k, size_t len) {
    size_t i, hash;

    const uint8_t* key = static_cast<const uint8_t*>(k);
    for (hash = 0, i = 0; i < len; ++i) {
        hash ^= key[i]; 
        hash *= 1099511628211;
    }

    return hash;
}

template<typename T>
struct fnvhash64 {
    size_t operator() (const T& key) const {
        return _fnvhash64(&key, sizeof(T));
    }
};

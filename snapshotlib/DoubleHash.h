#pragma once

template<typename T>
std::size_t doublehash(const T& value)
{
    std::size_t h = std::hash<T>()(value);

    h = std::hash<std::size_t>()(h);

    return h;
}

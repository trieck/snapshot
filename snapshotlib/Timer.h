#pragma once

#include <chrono>

class Timer
{
public:
    Timer();
    ~Timer();

    std::string str() const;
    void restart();
private:
    using Clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<std::chrono::steady_clock>;
    time_point start_;
};

inline std::ostream& operator << (std::ostream& s, const Timer& timer)
{
    return s << timer.str();
}
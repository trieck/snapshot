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
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::time_point<std::chrono::steady_clock> time_point;
    time_point start_;
};

inline std::ostream& operator << (std::ostream& s, const Timer& timer)
{
    return s << timer.str();
}
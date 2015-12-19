#pragma once

#include <boost/timer.hpp>

class Timer
{
public:
    Timer();
    ~Timer();
    
    std::string str() const;
    void restart();
private:
    boost::timer timer_;
};

inline std::ostream& operator << (std::ostream& s, const Timer& timer)
{
    return s << timer.str();
}
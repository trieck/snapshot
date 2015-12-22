#include "stdafx.h"
#include "Timer.h"

Timer::Timer()
{
    start_ = Clock::now();
}

Timer::~Timer()
{

}

std::string Timer::str() const
{
    using std::chrono::duration_cast;
    auto now = Clock::now();

    auto elapsed = duration_cast<std::chrono::milliseconds>(now - start_).count();

    auto hours = (elapsed / 1000) / 3600;
    auto minutes = ((elapsed / 1000) % 3600) / 60;
    auto seconds = (elapsed / 1000) % 60;
    auto millis = elapsed % 1000;

    boost::format fmt;

    if (hours)
        fmt = boost::format("%2d:%02d:%02d hours") % hours % minutes % seconds;
    else if (minutes)
        fmt = boost::format("%2d:%02d minutes") % minutes % seconds;
    else
        fmt = boost::format("%2d.%03d seconds") % seconds % millis;

    return fmt.str();
}

void Timer::restart()
{
    start_ = Clock::now();
}

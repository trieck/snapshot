#include "stdafx.h"
#include "Timer.h"

Timer::Timer()
{
}

Timer::~Timer()
{
}

std::string Timer::str() const
{
    auto elapsed = static_cast<int>(timer_.elapsed());

    auto hours = elapsed / 3600;
    auto minutes = (elapsed % 3600) / 60;
    auto seconds = elapsed % 60;

    boost::format fmt;

    if (hours)
        fmt = boost::format("%2d:%02d:%02d hours") % hours % minutes % seconds;
    else if (minutes)
        fmt = boost::format("%2d:%02d minutes") % minutes % seconds;
    else
        fmt = boost::format("%2d seconds") % seconds;

    return fmt.str();
}

void Timer::restart()
{
    timer_.restart();
}

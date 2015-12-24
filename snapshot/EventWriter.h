#pragma once

#include "json/json.h"
#include "Event.h"

class EventWriter
{
public:
    EventWriter();
    ~EventWriter();
    void write(std::ostream& os, const Event& event);
    std::string write(const Event& event);

private:
    Json::StreamWriterBuilder builder_;
};


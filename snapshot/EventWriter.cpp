#include "stdafx.h"
#include "EventWriter.h"

EventWriter::EventWriter()
{
    builder_.settings_["commentStyle"] = "None";
    builder_.settings_["indentation"] = "";
}

EventWriter::~EventWriter()
{
}

void EventWriter::write(std::ostream& os, const Event& event)
{
    os << Json::writeString(builder_, event.getEvent());
}

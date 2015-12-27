#include "snapshotlib.h"
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
    os << write(event);
}

std::string EventWriter::write(const Event& event)
{
    return Json::writeString(builder_, event.getEvent());;
}
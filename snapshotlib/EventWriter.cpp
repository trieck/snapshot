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

std::ostream& EventWriter::write(std::ostream& os, const Event& event) const
{
    return os << write(event);
}

std::string EventWriter::write(const Event& event) const
{
    return Json::writeString(builder_, event.getEvent());;
}

#include "stdafx.h"
#include "Event.h"

Event::Event(const Json::Value& event) : event_(event)
{
    sequence_ = (*this)["EVENT_SEQUENCE_NUMBER"].asUInt64();
    parseMeta();
}

Event::~Event()
{
}

const Json::Value& Event::operator[](const char* key) const
{
    return event_[key];
}

const Json::Value& Event::operator[](const std::string & key) const
{
    return event_[key];
}

void Event::parseMeta()
{
    auto meta = event_["METADATA"];

    std::string name;
    auto size = meta.size();

    for (unsigned i = 0; i < size; ++i) {
        name = meta[i]["METADATA_NAME"].asString();
        Json::Value value = meta[i]["METADATA_VALUE"];
        _meta.insert(std::pair<std::string, Json::Value>{name, value});
    }
}

const Json::Value& Event::getMeta(const char* key) const
{
    auto it = _meta.find(key);
    if (it == _meta.end()) {
        return Json::Value::nullRef;
    }

    return it->second;
}

const Json::Value & Event::getEvent() const
{
    return event_;
}

uint64_t Event::getSequenceNumber() const
{
    return sequence_;
}

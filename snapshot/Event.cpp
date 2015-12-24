#include "stdafx.h"
#include "Event.h"

Event::Event(const Json::Value& event) : event_(event)
{
    parseMeta();
    sequence_ = (*this)["EVENT_SEQUENCE_NUMBER"].asUInt64();
    objectId_ = getMeta("ObjectId").asString();
}

Event::Event(const Event & rhs)
{
    *this = rhs;
}

Event::~Event()
{
}

Event & Event::operator=(const Event & rhs)
{
    if (this != &rhs) {
        event_ = rhs.event_;
        meta_ = rhs.meta_;
        objectId_ = rhs.objectId_;
        sequence_ = rhs.sequence_;
    }
    return *this;
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
        meta_.insert(std::pair<std::string, Json::Value>{name, value});
    }
}

const Json::Value& Event::getMeta(const char* key) const
{
    auto it = meta_.find(key);
    if (it == meta_.end()) {
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

std::string Event::getObjectId() const
{
    return objectId_;
}

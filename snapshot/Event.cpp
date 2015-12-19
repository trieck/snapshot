#include "stdafx.h"
#include "Event.h"

Event::Event(const Json::Value& event) : _event(event)
{
    parseMeta();
}

Event::~Event()
{
}

const Json::Value& Event::operator[](const char* key) const
{
    return _event[key];
}

const Json::Value& Event::operator[](const std::string & key) const
{
    return _event[key];
}

void Event::parseMeta()
{
    auto meta = _event["METADATA"];

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

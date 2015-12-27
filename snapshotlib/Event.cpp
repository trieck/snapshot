#include "snapshotlib.h"
#include "Event.h"

Event::Event() : event_(Json::objectValue)
{
    event_["METADATA"] = Json::arrayValue;
}

Event::Event(const Json::Value& event) : event_(event)
{
    parseMeta();
}

Event::Event(const Event& rhs)
{
    *this = rhs;
}

Event::~Event()
{
}

Event & Event::operator=(const Event& rhs)
{
    if (this != &rhs) {
        event_ = rhs.event_;
        meta_ = rhs.meta_;
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
    return (*this)["EVENT_SEQUENCE_NUMBER"].asUInt64();
}

std::string Event::getObjectId() const
{
    return getMeta("ObjectId").asString();
}

std::string Event::getParentId() const
{
    return getMeta("ParentObjectId").asString();
}

std::string Event::getRootId() const
{
    return getMeta("RootWindowObjectId").asString();
}

void Event::setObjectId(const std::string& objectId)
{
    putMeta("ObjectId", objectId);
}

void Event::putMeta(const std::string&name, const std::string& value)
{
    removeMeta(name);

    auto object = Json::Value(Json::objectValue);
    object["METADATA_NAME"] = name;
    object["METADATA_VALUE"] = value;
    event_["METADATA"].append(object);

    meta_.insert(std::pair<std::string, Json::Value>{name, value});
}

void Event::removeMeta(const std::string& name)
{
    auto& meta = event_["METADATA"];

    Json::Value def;
    for (Json::ArrayIndex i = 0; i < meta.size(); ++i) {
        auto& v = meta.get(i, def);
        if (v["METADATA_NAME"] == name) {
            meta.removeIndex(i, &def);
            break;
        }
    }
    
    meta_.erase(name);
}
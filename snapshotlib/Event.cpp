#include "snapshotlib.h"
#include "Event.h"

namespace {
    auto constexpr METADATA = "METADATA";
    auto constexpr METADATA_NAME = "METADATA_NAME";
    auto constexpr METADATA_VALUE = "METADATA_VALUE";
    auto constexpr EVENT_SEQUENCE_NUMBER = "EVENT_SEQUENCE_NUMBER";
    auto constexpr OBJECT_ID = "ObjectId";
    auto constexpr PARENT_OBJECT_ID = "ParentObjectId";
    auto constexpr ROOT_WINDOW_OBJECT_ID = "RootWindowObjectId";
    auto constexpr TREE_CHILDREN = "TreeChildren";
}

Event::Event() : event_(Json::objectValue)
{
    event_[METADATA] = Json::arrayValue;
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
    auto& meta = event_[METADATA];

    auto size = meta.size();
    for (Json::ArrayIndex i = 0; i < size; ++i) {
        auto name = meta[i][METADATA_NAME].asString();
        auto& value = meta[i][METADATA_VALUE];
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
    return (*this)[EVENT_SEQUENCE_NUMBER].asUInt64();
}

std::string Event::getObjectId() const
{
    return getMeta(OBJECT_ID).asString();
}

std::string Event::getParentId() const
{
    return getMeta(PARENT_OBJECT_ID).asString();
}

std::string Event::getRootId() const
{
    return getMeta(ROOT_WINDOW_OBJECT_ID).asString();
}

void Event::setObjectId(const std::string& objectId)
{
    putMeta(OBJECT_ID, objectId);
}

void Event::putMeta(const std::string&name, const std::string& value)
{
    removeMeta(name);

    auto object = Json::Value(Json::objectValue);
    object[METADATA_NAME] = name;
    object[METADATA_VALUE] = value;
    event_[METADATA].append(object);

    meta_.insert(std::pair<std::string, Json::Value>{name, value});
}

void Event::removeMeta(const std::string& name)
{
    auto& meta = event_[METADATA];
    auto size = meta.size();

    Json::Value def;
    for (Json::ArrayIndex i = 0; i < size; ++i) {
        auto& v = meta.get(i, def);
        if (v[METADATA_NAME] == name) {
            meta.removeIndex(i, &def);
            break;
        }
    }

    meta_.erase(name);
}

void Event::addChild(const std::string& objectId)
{
    auto& children = event_[TREE_CHILDREN];
    if (children.isNull()) {
        event_[TREE_CHILDREN] = Json::arrayValue;
    }

    if (!hasChild(objectId)) {
        children.append(objectId);
    }
}

bool Event::hasChild(const std::string& objectId)
{
    auto& children = event_[TREE_CHILDREN];
    if (!children.isArray())
        return false;

    for (Json::ValueIterator it = children.begin(); it != children.end(); it++) {
        if (*it == objectId)
            return true;
    }

    return false;
}

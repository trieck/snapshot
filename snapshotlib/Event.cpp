#include "snapshotlib.h"
#include "Event.h"
#include "EventBuffer.h"

namespace {
    auto constexpr METADATA = "METADATA";
    auto constexpr METADATA_NAME = "METADATA_NAME";
    auto constexpr METADATA_VALUE = "METADATA_VALUE";
    auto constexpr EVENT_SEQUENCE_NUMBER = "EVENT_SEQUENCE_NUMBER";
    auto constexpr OBJECT_ID = "ObjectId";
    auto constexpr PARENT_OBJECT_ID = "ParentObjectId";
    auto constexpr ROOT_WINDOW_OBJECT_ID = "RootWindowObjectId";
    auto constexpr TREE_CHILDREN = "TreeChildren";
    auto constexpr INITIAL_SEQUENCE_NUMBER = "InitialSequenceNumber";
}

Event::Event() : event_(Json::objectValue)
{
    event_[METADATA] = Json::arrayValue;
}

Event::Event(EventBufferPtr& buffer)
{
    *this = buffer;
}

Event::Event(const Json::Value& event) : event_(event)
{
    setInitialSequenceNumber();
    parseMeta();
}

Event::Event(const Event& rhs)
{
    *this = rhs;
}

Event::~Event()
{
}

Event& Event::operator=(const Event& rhs)
{
    if (this != &rhs) {
        event_ = rhs.event_;
        meta_ = rhs.meta_;
    }
    return *this;
}

Event& Event::operator=(EventBufferPtr& buffer)
{
    clear();

    auto event = buffer->getEvent();

    copyStringField("EVENT_NAME", event->name());
    event_["EVENT_SEQUENCE_NUMBER"] = event->sequence();
    copyStringField("EVENT_SOURCE", event->source());
    copyStringField("OPERATION_ID", event->operation_id());
    event_["PROCESS_ID"] = event->process_id();
    copyStringField("SESSION_ID", event->session_id());
    copyStringField("TIME_STAMP", event->time_stamp());
    copyStringField("TIME_ZONE_NAME", event->time_zone_name());
    copyStringField("USER_ID", event->user_id());
    copyChildren(event->tree_children());

    if (event->initial_sequence()) {
        event_["INITIAL_SEQUENCE_NUMBER"] = event->initial_sequence();
    }

    auto metadata = event->metadata();
    if (metadata && metadata->size()) {
        for (auto it = metadata->begin(); it != metadata->end(); ++it) {
            auto name = (*it)->name();
            auto value = (*it)->value();
            putMeta(name->c_str(), value->c_str());
        }
    }

    return *this;
}

const Json::Value& Event::operator[](const char* key) const
{
    return event_[key];
}

const Json::Value& Event::operator[](const std::string& key) const
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
        meta_.insert(std::make_pair(name, value));
    }
}

void Event::copyStringField(const char* fieldName, const flatbuffers::String* source)
{
    if (source != nullptr) {
        event_[fieldName] = source->c_str();
    }
}

void Event::copyChildren(const FBStringVec* children)
{
    if (children == nullptr)
        return;

    for (auto it = children->begin(); it != children->end(); ++it) {
        addChild(it->c_str());
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

const Json::Value& Event::getEvent() const
{
    return event_;
}

uint64_t Event::getSequenceNumber() const
{
    return (*this)[EVENT_SEQUENCE_NUMBER].asUInt64();
}

uint64_t Event::initialSequenceNumber() const
{
    return (*this)[INITIAL_SEQUENCE_NUMBER].asUInt64();
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

void Event::setParentId(const std::string& parentId)
{
    putMeta(PARENT_OBJECT_ID, parentId);
}

void Event::setRootId(const std::string& rootId)
{
    putMeta(ROOT_WINDOW_OBJECT_ID, rootId);
}

void Event::putMeta(const std::string& name, const std::string& value)
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
        const auto& v = meta.get(i, def);
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

bool Event::hasChild(const std::string& objectId) const
{
    auto& children = (*this)[TREE_CHILDREN];
    if (!children.isArray())
        return false;

    for (auto it = children.begin(); it != children.end(); it++) {
        if (*it == objectId)
            return true;
    }

    return false;
}

bool Event::hasChildren() const
{
    auto& children = (*this)[TREE_CHILDREN];
    return children.isArray();
}

bool Event::removeChild(const std::string& objectId)
{
    auto& children = event_[TREE_CHILDREN];
    if (!children.isArray())
        return false;

    auto size = children.size();

    Json::Value removed;
    for (Json::ArrayIndex i = 0; i < size; ++i) {
        if (children[i] == objectId) {
            return children.removeIndex(i, &removed);
        }
    }

    return false;
}

Event Event::merge(const Event& event) const
{
    Event m(*this);

    auto& children = event[TREE_CHILDREN];
    for (Json::ValueConstIterator it = children.begin(); it != children.end(); it++) {
        m.addChild((*it).asString());
    }

    m.copyInitialSequenceNumber(event);

    return m;
}

const Json::Value& Event::children() const
{
    return (*this)[TREE_CHILDREN];
}

void Event::clear()
{
    event_.clear();
    meta_.clear();
}

void Event::setInitialSequenceNumber()
{
    if ((*this)[INITIAL_SEQUENCE_NUMBER].isNull() && !(*this)[EVENT_SEQUENCE_NUMBER].isNull()) {
        event_[INITIAL_SEQUENCE_NUMBER] = event_[EVENT_SEQUENCE_NUMBER];
    }
}

void Event::copyInitialSequenceNumber(const Event& event)
{
    auto& initialSequenceNumber = event[INITIAL_SEQUENCE_NUMBER];
    if (!initialSequenceNumber.isNull()) {
        event_[INITIAL_SEQUENCE_NUMBER] = initialSequenceNumber;
    }
}

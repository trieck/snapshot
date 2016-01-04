#pragma once

#include "flatbuffers/flatbuffers.h"

class EventBuffer;
using EventBufferPtr = std::unique_ptr<EventBuffer>;

class Event
{
public:
    Event();
    Event(EventBufferPtr& buffer);
    Event(const Json::Value & event);
    Event(const Event& rhs);
    ~Event();

    Event& operator =(const Event& rhs);
    Event& operator =(EventBufferPtr& buffer);

    const Json::Value & operator[](const char* key) const;
    const Json::Value& operator[](const std::string& key) const;
    const Json::Value& getMeta(const char* key) const;
    const Json::Value& getEvent() const;

    uint64_t getSequenceNumber() const;
    uint64_t initialSequenceNumber() const;
    std::string getObjectId() const;
    std::string getParentId() const;
    std::string getRootId() const;

    void setObjectId(const std::string& objectId);
    void setParentId(const std::string& parentId);
    void setRootId(const std::string& rootId);
    void putMeta(const std::string& name, const std::string& value);
    void removeMeta(const std::string& name);
    void addChild(const std::string& objectId);
    bool hasChild(const std::string& objectId) const;    
    bool hasChildren() const;
    bool removeChild(const std::string& objectId);
    Event merge(const Event& event) const;    
    const Json::Value& children() const;
    void clear();

private:
    using FBStringVec = flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>;

    void setInitialSequenceNumber();
    void copyInitialSequenceNumber(const Event& event);
    void parseMeta();
    void copyStringField(const char* fieldName, const flatbuffers::String* source);
    void copyChildren(const FBStringVec* children);
    Json::Value event_;
    std::unordered_map<std::string, Json::Value> meta_;
};

using EventVec = std::vector<Event>;

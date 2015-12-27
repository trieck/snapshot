#pragma once

class Event
{
public:
    Event();
    Event(const Json::Value & event);
    Event(const Event& rhs);
    ~Event();

    Event& operator =(const Event& rhs);

    const Json::Value & operator[](const char* key) const;
    const Json::Value& operator[](const std::string& key) const;
    const Json::Value& getMeta(const char* key) const;
    const Json::Value& getEvent() const;

    uint64_t getSequenceNumber() const;
    std::string getObjectId() const;
    std::string getParentId() const;
    std::string getRootId() const;

    void setObjectId(const std::string& objectId);
    void putMeta(const std::string& name, const std::string& value);
    void removeMeta(const std::string& name);
private:
    void parseMeta();
    Json::Value event_;
    std::unordered_map<std::string, Json::Value> meta_;
};
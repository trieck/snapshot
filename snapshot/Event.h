#pragma once

class Event
{
public:
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
private:
    void parseMeta();
    Json::Value event_;
    uint64_t sequence_;
    std::string objectId_;
    std::unordered_map<std::string, Json::Value> meta_;
};

#pragma once

class Event
{
public:
    Event(const Json::Value & event);
    ~Event();

    const Json::Value & operator[](const char* key) const;
    const Json::Value& operator[](const std::string& key) const;
    const Json::Value& getMeta(const char* key) const;
    const Json::Value& getEvent() const;

    uint64_t getSequenceNumber() const;
private:
    void parseMeta();
    Json::Value event_;
    uint64_t sequence_;
    std::unordered_map<std::string, Json::Value> _meta;
};


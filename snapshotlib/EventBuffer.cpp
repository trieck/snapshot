#include "snapshotlib.h"
#include "EventBuffer.h"

EventBuffer::EventBuffer(const uint8_t* ptr, uint32_t size)
{
    builder_.PushFlatBuffer(ptr, size);
}

EventBuffer::EventBuffer(const Event& event)
{
    *this = event;
}

EventBuffer::~EventBuffer()
{
}

EventBuffer& EventBuffer::operator=(const Event& event)
{
    construct(event);
    return *this;
}

void EventBuffer::construct(const Event& event)
{
    builder_.Clear();

    auto ename = builder_.CreateString(event["EVENT_NAME"].asString());
    auto sequence = event["EVENT_SEQUENCE_NUMBER"].asUInt64();
    auto initial_sequence = event["InitialSequenceNumber"].asUInt64();
    auto source = builder_.CreateString(event["EVENT_SOURCE"].asString());
    auto opid = builder_.CreateString(event["OPERATION_ID"].asString());
    auto procid = event["PROCESS_ID"].asUInt64();
    auto sessionid = builder_.CreateString(event["SESSION_ID"].asString());
    auto time_stamp = builder_.CreateString(event["TIME_STAMP"].asString());
    auto time_zone_name = builder_.CreateString(event["TIME_ZONE_NAME"].asString());
    auto userid = builder_.CreateString(event["USER_ID"].asString());

    std::vector<flatbuffers::Offset<FBMeta>> mdvector;
    const auto& metadata = event["METADATA"];
    auto size = metadata.size();
    for (Json::ArrayIndex i = 0; i < size; ++i) {
        auto name = builder_.CreateString(metadata[i]["METADATA_NAME"].asString());
        auto value = builder_.CreateString(metadata[i]["METADATA_VALUE"].asString());
        auto meta = CreateFBMeta(builder_, name, value);
        mdvector.push_back(meta);
    }

    auto fbmeta = builder_.CreateVector(mdvector);

    std::vector<flatbuffers::Offset<flatbuffers::String>> vchildren;
    const auto& children = event["TreeChildren"];
    for (auto it = children.begin(); it != children.end(); ++it) {
        auto child = (*it).asString();
        vchildren.push_back(builder_.CreateString(child));
    }

    auto fbchildren = vchildren.size() > 0 ? builder_.CreateVector(vchildren) : 0;

    auto root = CreateFBEvent(builder_, ename, sequence, initial_sequence,
        source, opid, procid, sessionid, time_stamp, time_zone_name, userid, fbchildren, fbmeta);

    FinishFBEventBuffer(builder_, root);
}

const FBEvent* EventBuffer::getEvent() const
{
    return GetFBEvent(builder_.GetBufferPointer());
}

std::string EventBuffer::getMeta(const char* key) const
{
    auto metadata = getEvent()->metadata();
    
    if (metadata && metadata->size()) {
        for (auto it = metadata->begin(); it != metadata->end(); ++it) {
            if (strcmp(it->name()->c_str(), key) == 0) {
                return it->value()->c_str();
            }
        }
    }

    return "";
}

EventBufferPtr EventBuffer::makeBuffer(const Event& event)
{
    return EventBufferPtr(new EventBuffer(event));
}

EventBufferPtr EventBuffer::makeBuffer(const uint8_t* ptr, uint32_t size)
{
    return EventBufferPtr(new EventBuffer(ptr, size));
}

EventBufferPtr EventBuffer::makeBuffer(const ByteBuffer& buffer)
{
    return EventBufferPtr(new EventBuffer(buffer.ptr(), buffer.size()));
}

EventBuffer::operator uint8_t*() const
{
    return builder_.GetBufferPointer();
}

uint32_t EventBuffer::size() const
{
    return builder_.GetSize();
}

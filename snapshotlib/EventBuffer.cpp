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

    auto name = builder_.CreateString(event["EVENT_NAME"].asString());
    auto sequence = event["EVENT_SEQUENCE_NUMBER"].asUInt64();
    auto source = builder_.CreateString(event["EVENT_SOURCE"].asString());
    auto opid = builder_.CreateString(event["OPERATION_ID"].asString());
    auto procid = event["PROCESS_ID"].asUInt64();
    auto sessionid = builder_.CreateString(event["SESSION_ID"].asString());
    auto time_stamp = builder_.CreateString(event["TIME_STAMP"].asString());
    auto time_zone_name = builder_.CreateString(event["TIME_ZONE_NAME"].asString());
    auto userid = builder_.CreateString(event["USER_ID"].asString());

    // TODO: add metadata

    auto root = CreateFBEvent(builder_, name, sequence, source, opid, procid,
        sessionid, time_stamp, time_zone_name, userid, 0);

    FinishFBEventBuffer(builder_, root);
}

const FBEvent* EventBuffer::getEvent() const
{
    return GetFBEvent(builder_.GetBufferPointer());
}

EventBufferPtr EventBuffer::makeBuffer(const Event& event)
{
    return EventBufferPtr(new EventBuffer(event));
}

EventBuffer::operator uint8_t*() const
{
    return builder_.GetBufferPointer();
}

uint32_t EventBuffer::size() const
{
    return builder_.GetSize();
}

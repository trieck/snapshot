#pragma once

#include "Event.h"
#include "ByteBuffer.h"
#include "event_generated.h"

class EventBuffer;
using EventBufferPtr = std::unique_ptr<EventBuffer>;

class EventBuffer
{
public:
    EventBuffer() = delete;
    EventBuffer(const EventBuffer& rhs) = delete;
    ~EventBuffer();    
    EventBuffer& operator = (const EventBuffer& rhs) = delete;
    
    static EventBufferPtr makeBuffer(const Event& event);
    static EventBufferPtr makeBuffer(const uint8_t* ptr, uint32_t size);
    static EventBufferPtr makeBuffer(const ByteBuffer& buffer);

    const FBEvent* getEvent() const;

    operator uint8_t*() const;
    uint32_t size() const;
private:
    EventBuffer(const uint8_t* ptr, uint32_t size);
    EventBuffer(const Event& event);
    EventBuffer& operator = (const Event& event);
    void construct(const Event& event);
    
    flatbuffers::FlatBufferBuilder builder_;
};


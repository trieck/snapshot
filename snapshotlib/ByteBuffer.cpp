#include "snapshotlib.h"
#include "ByteBuffer.h"

ByteBuffer::ByteBuffer(uint32_t size) : size_(size)
{
    pbuf_ = buf_ = new uint8_t[size];
}

ByteBuffer::~ByteBuffer()
{
    delete[] buf_;
}

ByteBuffer & ByteBuffer::operator<<(uint8_t b)
{
    *pbuf_++ = b;
    return *this;
}

void ByteBuffer::reset() noexcept
{
    pbuf_ = buf_;
}

uint32_t ByteBuffer::size() const
{
    return size_;
}

uint8_t* ByteBuffer::ptr() const
{
    return buf_;
}

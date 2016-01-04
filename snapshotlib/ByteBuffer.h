#pragma once

class ByteBuffer
{
public:
    ByteBuffer(uint32_t size);
    ByteBuffer(const ByteBuffer& rhs) = delete;
    ~ByteBuffer();

    ByteBuffer& operator = (const ByteBuffer& rhs) = delete;
    ByteBuffer& operator << (uint8_t b);
    void reset() noexcept;

    uint32_t size() const;
    uint8_t* ptr() const;

private:
    uint8_t *buf_, *pbuf_;
    uint32_t size_;
};


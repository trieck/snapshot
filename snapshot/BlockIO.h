#pragma once
class BlockIO
{
public:
    BlockIO();
    ~BlockIO();

    void open(const char* filename, OpenMode mode);
    void close();
    bool readblock(uint64_t blockno, void* pv);
    bool writeblock(uint64_t blockno, const void* pv);
    bool seekblock(uint64_t blockno);

    uint64_t tell();
    uint64_t getFileSize();
    void flush();

    static constexpr auto BLOCK_SIZE = 4096UL;
private:
    std::fstream stream_;
};


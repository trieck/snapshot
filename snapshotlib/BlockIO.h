#pragma once

class BlockIO
{
public:
    BlockIO();
    ~BlockIO();

    void open(const char* filename, OpenMode mode);
    void close();
    void readblock(uint64_t blockno, void* pv);
    void writeblock(uint64_t blockno, const void* pv);
    void seekblock(uint64_t blockno);

    uint64_t tell();
    uint64_t getFileSize();
    void flush();

    static constexpr auto BLOCK_SIZE = 4096UL;

    static void* mkblock();
    static void freeblock(void* block);
private:
    std::fstream stream_;
};

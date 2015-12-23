#include "stdafx.h"
#include "BlockIO.h"

BlockIO::BlockIO()
{
}

BlockIO::~BlockIO()
{
    close();
}

void BlockIO::open(const char* filename, OpenMode mode)
{
    close();
    stream_.open(filename, mode | std::ios::binary);
    if (!stream_.is_open()) {
        boost::format message = boost::format("unable to open file \"%s\".") % filename;
        throw std::exception(message.str().c_str());
    }
}

void BlockIO::close()
{
    if (stream_.is_open()) {
        stream_.close();
    }
}

bool BlockIO::readblock(uint64_t blockno, void* pv)
{
    if (!seekblock(blockno))
        return false;
    stream_.read(static_cast<char*>(pv), BLOCK_SIZE);
    return stream_.good();
}

bool BlockIO::writeblock(uint64_t blockno, const void* pv)
{
    if (!seekblock(blockno))
        return false;
    stream_.write(static_cast<const char*>(pv), BLOCK_SIZE);
    return stream_.good();
}

bool BlockIO::seekblock(uint64_t blockno)
{
    auto offset = blockno * BLOCK_SIZE;
    stream_.seekp(offset, std::ios::beg);
    return stream_.good();
}

uint64_t BlockIO::tell()
{
    return stream_.tellp();
}

uint64_t BlockIO::getFileSize()
{
    auto save = stream_.tellp();
    stream_.seekp(0, std::ios::end);
    auto end = stream_.tellp();
    stream_.seekp(save, std::ios::beg);
    return end;
}

void BlockIO::flush()
{
    stream_.flush();
}

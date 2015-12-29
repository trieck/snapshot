#include "snapshotlib.h"
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

void BlockIO::readblock(uint64_t blockno, void* pv)
{
    seekblock(blockno);
    if (!stream_.read(static_cast<char*>(pv), BLOCK_SIZE))
        throw std::exception("cannot read block.");
}

void BlockIO::writeblock(uint64_t blockno, const void* pv)
{
    seekblock(blockno);
    if (!stream_.write(static_cast<const char*>(pv), BLOCK_SIZE))
        throw std::exception("cannot write block.");
}

void BlockIO::seekblock(uint64_t blockno)
{
    auto offset = blockno * BLOCK_SIZE;
    if (!stream_.seekp(offset, std::ios::beg))
        throw std::exception("cannot seek block.");
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

void* BlockIO::mkblock()
{
    return new char[BlockIO::BLOCK_SIZE]();
}

void BlockIO::freeblock(void* block)
{
    delete[] block;
}

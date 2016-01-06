#include "stdafx.h"
#include "Partition.h"
#include "EventWriter.h"

Partition::Partition(const std::string& key) : key_(key)
{
    name_ = std::tmpnam(nullptr);
}

Partition::Partition(const std::string& key, const std::string& name)
    : key_(key), name_(name)
{
}

Partition::~Partition()
{
    close();
    _unlink(name_.c_str());
}

void Partition::open(OpenMode mode)
{
    close();
    stream_.open(name_, mode);
    if (!stream_.is_open()) {
        boost::format message = boost::format("unable to open file \"%s\".") % name_;
        throw std::runtime_error(message.str().c_str());
    }
}

void Partition::close()
{
    if (stream_.is_open()) {
        stream_.close();
    }
}

void Partition::write(const std::vector<Event>& vec)
{
    open(std::ios::out);

    EventWriter writer;
    for (const auto& event : vec) {
        stream_ << event.getSequenceNumber() << '\t';
        writer.write(stream_, event);
        stream_ << endl;
    }

    close();
}

PartitionPtr Partition::makePartition(const std::string& key)
{
    return PartitionPtr(new Partition(key));
}

PartitionPtr Partition::makePartition(const std::string& key, const std::string& name)
{
    return PartitionPtr(new Partition(key, name));
}

std::string Partition::getKey() const
{
    return key_;
}

std::string Partition::getFilename() const
{
    return name_;
}

std::iostream& Partition::stream()
{
    return stream_;
}

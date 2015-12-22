#include "stdafx.h"
#include "Partition.h"
#include "EventWriter.h"

Partition::Partition(const std::string& key) : key_(key)
{
    name_ = std::tmpnam(nullptr);
}

Partition::~Partition()
{
    _unlink(name_.c_str());
}

void Partition::write(const std::vector<Event>& vec)
{
    std::ofstream ofs(name_);
    if (!ofs.is_open()) {
        boost::format message = boost::format("unable to open file \"%s\".") % name_;
        throw std::exception(message.str().c_str());
    }
    
    EventWriter writer;
    for (const auto& event : vec) {
        ofs << event.getSequenceNumber() << '\t';
        writer.write(ofs, event);
        ofs << endl;
    }

    ofs.flush();
    ofs.close();
}

std::unique_ptr<Partition> Partition::makePartition(const std::string& key)
{
    return std::unique_ptr<Partition>(new Partition(key));
}

std::string Partition::getFilename() const
{
    return name_;
}

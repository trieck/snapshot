#include "stdafx.h"
#include "Snapshotter.h"

Snapshotter::Snapshotter()
{
}

Snapshotter::~Snapshotter()
{
}

void Snapshotter::snapshot(const char* file)
{
    std::ifstream stream(file);
    if (!stream.is_open()) {
        boost::format message = boost::format("unable to open file \"%s\".") % file;
        throw std::exception(message.str().c_str());
    }

    Json::Reader reader;
    Json::Value event;
    
    std::string line;
    while (getline(stream, line)) {
        reader.parse(line, event, false);
        insert(event);
    }
    
    merge();
}

void Snapshotter::insert(const Event& event)
{
    partitioner_.partition(event);
    if (partitioner_.isfull()) {
        partitioner_.flush();
    }
}

void Snapshotter::merge()
{
    partitioner_.merge();
}
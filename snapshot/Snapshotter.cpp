#include "stdafx.h"
#include "Snapshotter.h"

namespace {
    constexpr int BUF_SIZE = 8192;
}

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
    char line[BUF_SIZE];
    while (stream.getline(line, BUF_SIZE)) {
        reader.parse(line, event, false);
        partition(event);
    }
}

void Snapshotter::partition(const Event& event)
{
    _partitioner.partition(event);
}

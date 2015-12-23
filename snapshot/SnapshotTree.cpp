#include "stdafx.h"
#include "SnapshotTree.h"

SnapshotTree::SnapshotTree()
{
    index_.open(std::tmpnam(nullptr));
}

SnapshotTree::~SnapshotTree()
{
    index_.close();
}

void SnapshotTree::load(Partition* partition)
{
    partition->open(std::ios::in);
    std::istream& stream = partition->stream();

    Json::Reader reader;
    Json::Value event;

    uint64_t key;
    std::string line;
    while (stream >> key) {
        stream.rdbuf()->sbumpc();   // tab
        getline(stream, line);
        reader.parse(line, event, false);
        process(event);
    }

    partition->close();
}

void SnapshotTree::process(const Event& event)
{
}

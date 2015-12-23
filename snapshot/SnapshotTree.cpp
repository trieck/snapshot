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

void SnapshotTree::load(std::istream& stream)
{
    uint64_t key;
    std::string objectId, event;
    while (stream >> key) {
        stream >> objectId;
        stream.rdbuf()->sbumpc();   // tab
        getline(stream, event);
        process(objectId, event);
    }
}

void SnapshotTree::process(const std::string& objectId, const std::string& event)
{
}
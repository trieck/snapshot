#include "stdafx.h"
#include "Snapshotter.h"
#include "SnapshotTree.h"

Snapshotter::Snapshotter()
{
}

Snapshotter::~Snapshotter()
{
}

void Snapshotter::snapshot(const char* file)
{
    partition(file);
    mktrees();
}

void Snapshotter::partition(const char* file)
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

    partitions_ = partitioner_.merge();
}

void Snapshotter::insert(const Event& event)
{
    partitioner_.partition(event);
    if (partitioner_.isfull()) {
        partitioner_.flush();
    }
}

void Snapshotter::mktrees()
{
    for (const auto& partition : partitions_) {
        mktree(partition);
    }
}

void Snapshotter::mktree(const PartitionPtr& partition)
{
    SnapshotTree tree;
    tree.load(partition->stream());
}

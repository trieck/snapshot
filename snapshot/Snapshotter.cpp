#include "stdafx.h"
#include "Snapshotter.h"
#include "SnapshotTree.h"
#include "Timer.h"

Snapshotter::Snapshotter()
{
}

Snapshotter::~Snapshotter()
{
}

void Snapshotter::snapshot(const char* file)
{
    partition(file);
    merge();
    snapshot();
}

void Snapshotter::partition(const char* file)
{
    Timer timer;
    cout << "partitioning...";

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

    cout << "complete (" << timer << ")" << endl;
}

void Snapshotter::merge()
{
    Timer timer;
    cout << "merging...";
    partitions_ = partitioner_.merge();
    cout << "complete (" << timer << ")" << endl;
}

void Snapshotter::insert(const Event& event)
{
    partitioner_.partition(event);
    if (partitioner_.isfull()) {
        partitioner_.flush();
    }
}

void Snapshotter::snapshot()
{
    Timer timer;
    cout << "snapshotting...";
    for (const auto& partition : partitions_) {
        snapshot(partition);
    }
    cout << "complete (" << timer << ")" << endl;
}

void Snapshotter::snapshot(const PartitionPtr& partition)
{
    SnapshotTree tree;
    tree.snapshot(partition.get());
}

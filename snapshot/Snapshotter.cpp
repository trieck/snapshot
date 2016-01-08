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
    cout << "partitioning..." << flush;

    std::ifstream stream(file);
    if (!stream.is_open()) {
        auto message = boost::format("unable to open file \"%s\".") % file;
        throw std::runtime_error(message.str().c_str());
    }

    Json::Reader reader;
    Json::Value event;

    std::string line;
    while (getline(stream, line)) {
        reader.parse(line, event, false);
        insert(event);
    }

    cout << "complete (" << timer << ")" << flush << endl;
}

void Snapshotter::merge()
{
    Timer timer;
    cout << "merging..." << flush;
    partitions_ = partitioner_.merge();
    cout << "complete (" << timer << ")" << flush << endl;
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
    cout << "snapshotting..." << flush;
    for (const auto& partition : partitions_) {
        snapshot(partition);
    }
    cout << "complete (" << timer << ")" << flush << endl;
}

void Snapshotter::snapshot(const PartitionPtr& partition)
{
    SnapshotTree tree;
    tree.snapshot(partition.get());
}

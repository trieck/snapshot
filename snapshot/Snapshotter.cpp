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

void Snapshotter::snapshot(const char* infile, const char* outfile)
{
    partition(infile);
    merge();
    snapshot(outfile);
}

void Snapshotter::partition(const char* infile)
{
    Timer timer;
    cout << "partitioning \"" << infile << "\"..." << flush;

    std::ifstream stream(infile);
    if (!stream.is_open()) {
        auto message = boost::format("unable to open file \"%s\".") % infile;
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

void Snapshotter::snapshot(const char* outfile)
{
    Timer timer;
    cout << "snapshotting to \"" << outfile << "\"..." << flush;

    std::ofstream os;
    os.open(outfile, std::ios::out | std::ios::trunc);

    for (const auto& partition : partitions_) {
        snapshot(partition, os);
    }

    cout << "complete (" << timer << ")" << flush << endl;
}

void Snapshotter::snapshot(const PartitionPtr& partition, std::ostream& os)
{
    SnapshotTree tree;
    tree.snapshot(partition.get(), os);
}

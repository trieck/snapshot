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

void SnapshotTree::stats()
{
    cout << endl << "    Index filename: " << index_.filename() << endl;
    cout << "    Index file size: " << comma(index_.filesize()) << " bytes" << endl;
    cout << "    Hash table size: " << comma(index_.tablesize()) << " buckets" << endl;
    cout << "    Hash table load factor: " << boost::format("%02.2f%%") % index_.loadfactor() << endl;
    cout << "    Longest run: " << comma(index_.maxrun()) << " buckets" << endl;
}

void SnapshotTree::process(const Event& event)
{
    auto name = event["EVENT_NAME"].asString();
    if (name == "Created") {
        insert(event);
    } else if (name == "Destroyed") {
        destroy(event);
    } else {
        update(event);
    }
}

void SnapshotTree::insert(const Event& event)
{
    index_.insert(event);
}

void SnapshotTree::destroy(const Event& event)
{
    index_.destroy(event);
}

void SnapshotTree::update(const Event& event)
{
    if (!index_.update(event))
        index_.insert(event);
}

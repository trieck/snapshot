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

void SnapshotTree::snapshot(Partition* partition)
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
    } else if (name == "Reparented") {
        reparent(event);
    } else {
        update(event);
    }
}

void SnapshotTree::insert(const Event& event)
{
    if (index_.find(event)) {
        index_.update(event);
    } else {
        insert(event, getParentId(event));
    }
}

void SnapshotTree::insert(const Event& event, const std::string& parentId)
{
    index_.insert(event);
    addChild(parentId, event.getObjectId());
}

void SnapshotTree::addChild(const std::string& parentId, const std::string& objectId)
{
    Event parent;
    if (!index_.find(parentId, parent)) {
        parent.setObjectId(parentId);
        parent.addChild(objectId);
        index_.insert(parent);
    } else {
        parent.addChild(objectId);
        index_.update(parent);
    }
}

void SnapshotTree::destroy(const Event& event)
{
    index_.destroy(event);
}

void SnapshotTree::update(const Event& event)
{
    Event u;
    if (index_.find(event.getObjectId(), u)) {
        index_.update(event.merge(u));
    } else {
        index_.insert(event);
    }
}

void SnapshotTree::reparent(const Event& event)
{
}

std::string SnapshotTree::getParentId(const Event& event) const
{
    auto parentId = event.getParentId();
    if (parentId.length() == 0)
        parentId = event.getRootId();

    return parentId;
}

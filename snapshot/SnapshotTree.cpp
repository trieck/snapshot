#include "stdafx.h"
#include "SnapshotTree.h"

namespace {
    const std::regex CREATED("Created");
    const std::regex DESTROYED("Destroyed");
    const std::regex REPARENTED("Reparented");
    const std::regex SNAP_EVENT("Click|DoubleClick|GotFocus|LostFocus|SelectedIndexChanged|UserModified|CellValueChanged");
}

SnapshotTree::SnapshotTree()
{
    store_.open(std::tmpnam(nullptr));
}

SnapshotTree::~SnapshotTree()
{
    store_.close();
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
    cout << endl << "    Index filename: " << store_.filename() << endl;
    cout << "    Index file size: " << comma(store_.filesize()) << " bytes" << endl;
    cout << "    Hash table size: " << comma(store_.tablesize()) << " buckets" << endl;
    cout << "    Hash table fill count: " << comma(store_.fillcount()) << " buckets" << endl;
    cout << "    Hash table load factor: " << boost::format("%02.2f%%") % store_.loadfactor() << endl;
    cout << "    Longest run: " << comma(store_.maxrun()) << " buckets" << endl;
}

void SnapshotTree::process(const Event& event)
{
    auto name = event["EVENT_NAME"].asString();
    if (std::regex_match(name, CREATED)) {
        insert(event);
    } else if (std::regex_match(name, DESTROYED)) {
        destroy(event);
    } else if (std::regex_match(name, REPARENTED)) {
        reparent(event);
    } else if (std::regex_match(name, SNAP_EVENT)) {
        snapshot(event);
    } else {
        update(event);
    }
}

void SnapshotTree::insert(const Event& event)
{
    if (store_.find(event)) {
        store_.update(event);
    } else {
        insert(event, getParentId(event));
    }
}

void SnapshotTree::insert(const Event& event, const std::string& parentId)
{
    store_.insert(event);
    addChild(parentId, event.getObjectId());
}

void SnapshotTree::addChild(const std::string& parentId, const std::string& objectId)
{
    Event parent;
    if (!store_.find(parentId, parent)) {
        parent.setObjectId(parentId);
        parent.addChild(objectId);
        store_.insert(parent);
    } else {
        parent.addChild(objectId);
        store_.update(parent);
    }
}

void SnapshotTree::parentRemove(const std::string& parentId, const std::string& objectId)
{
    Event p;
    if (store_.find(parentId, p)) {
        if (p.removeChild(objectId))
            store_.update(p);
    }
}

void SnapshotTree::destroy(const Event& event)
{
    store_.destroy(event);
}

void SnapshotTree::update(const Event& event)
{
    Event u;
    if (store_.find(event.getObjectId(), u)) {
        store_.update(event.merge(u));
    } else {
        store_.insert(event);
    }
}

void SnapshotTree::reparent(const Event& event)
{
    Event u;
    auto objectId = event.getObjectId();

    if (store_.find(objectId, u)) {
        auto oldParentId = u.getParentId();
        auto newParentId = event.getParentId();
        if (oldParentId.length()) {
            parentRemove(oldParentId, objectId);
        }
        addChild(newParentId, objectId);
        update(event);
    } else {
        insert(event);
    }
}

void SnapshotTree::snapshot(const Event & event)
{
    SnapshotParser parser; 
    
    update(event);    
    parse(parser, event);
}

void SnapshotTree::parse(SnapshotParser& parser, const Event& event)
{
    Event root;
    if (store_.find(event.getRootId(), root)) {
        parseNode(parser, root);
    }
}

void SnapshotTree::parseNode(SnapshotParser& parser, const Event& node)
{
    auto children = sortedChildren(node);
    for (const auto& child : children) {
        parseNode(parser, child);
    }
}

std::string SnapshotTree::getParentId(const Event& event) const
{
    auto parentId = event.getParentId();
    if (parentId.length() == 0)
        parentId = event.getRootId();

    return parentId;
}

EventVec SnapshotTree::sortedChildren(const Event& event)
{
    EventVec output;

    auto& children = event.children();
    for (Json::ValueConstIterator it = children.begin(); it != children.end(); it++) {
        Event child;
        if (store_.find((*it).asString(), child)) {
            output.push_back(child);
        }
    }

    return output;
}

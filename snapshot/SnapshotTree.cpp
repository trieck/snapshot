#include "stdafx.h"
#include "SnapshotTree.h"
#include "radixsort.h"
#include <EventWriter.h>

namespace {
    const std::regex CREATED("Created");
    const std::regex DESTROYED("Destroyed");
    const std::regex REPARENTED("Reparented");
    const std::regex SNAP_EVENT(
        "Click|DoubleClick|GotFocus|LostFocus|SelectedIndexChanged|UserModified|CellValueChanged");
}

struct InitialSeqPred : public std::unary_function<const EventBufferPtr&, bool>
{
    typedef uint64_t KEY_TYPE;
    KEY_TYPE bit_;

    explicit InitialSeqPred(KEY_TYPE bit) : bit_(bit) { }

    inline bool operator()(const EventBufferPtr& buffer) const
    {
        auto event = buffer->getEvent();
        auto num = event->initial_sequence();
        return !(num & (1ULL << bit_));
    }
};

SnapshotTree::SnapshotTree()
{
    store_.open(std::tmpnam(nullptr));
}

SnapshotTree::~SnapshotTree()
{
    store_.close();
    store_.unlink();
}

void SnapshotTree::snapshot(Partition* partition, std::ostream& os)
{
    partition->open(std::ios::in);
    std::istream& stream = partition->stream();

    EventWriter writer;
    Json::Reader reader;
    Json::Value value;
    Event event;

    uint64_t key;
    std::string line;
    while (stream >> key) {
        stream.rdbuf()->sbumpc();   // tab
        getline(stream, line);
        reader.parse(line, value, false);
        event = value;
        process(event);
        writer.write(os, event) << endl;
    }

    partition->close();
}

void SnapshotTree::stats()
{
    cout << endl << "    Index filename: " << store_.filename() << flush << endl;
    cout << "    Index file size: " << comma(store_.filesize()) << " bytes" << flush << endl;
    cout << "    Hash table size: " << comma(store_.tablesize()) << " buckets" << flush << endl;
    cout << "    Hash table fill count: " << comma(store_.fillcount()) << " buckets" << flush << endl;
    cout << "    Hash table load factor: " << boost::format("%02.2f%%") % store_.loadfactor() << flush << endl;
    cout << "    Longest run: " << comma(store_.maxrun()) << " buckets" << flush << endl;
}

void SnapshotTree::process(Event& event)
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
    addChild(parentId, event);
}

void SnapshotTree::addChild(const std::string& parentId, const Event& event)
{
    auto objectId = event.getObjectId();
    if (objectId == parentId)
        return;

    auto rootId = event.getRootId();

    Event parent;
    if (!store_.find(parentId, parent)) {
        if (rootId != parentId)
            parent.setParentId(rootId);
        parent.setRootId(rootId);
        parent.setObjectId(parentId);
        parent.addChild(objectId);
        store_.insert(parent);
        if (rootId != parentId)
            addChild(rootId, parent);
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
    Event e;
    if (store_.find(event.getObjectId(), e)) {
        parentRemove(e.getParentId(), e.getObjectId());
    }

    store_.destroy(event);    
}

void SnapshotTree::update(const Event& event)
{
    Event u, m;
    if (store_.find(event.getObjectId(), u)) {
        reparent(u, event);
        m = event.merge(u);
        store_.update(m);
    } else {
        store_.insert(event);
    }
}

void SnapshotTree::reparent(const Event& event)
{
    Event u;
    auto objectId = event.getObjectId();

    if (store_.find(objectId, u)) {
        reparent(u, event);
        store_.update(event);
    } else {
        insert(event);
    }
}

void SnapshotTree::reparent(const Event& from, const Event& to)
{
    auto oldParentId = from.getParentId();
    auto newParentId = to.getParentId();

    if (oldParentId.length() && newParentId.length() && oldParentId != newParentId) {
        parentRemove(oldParentId, to.getObjectId());
        addChild(newParentId, to);
    }
}

void SnapshotTree::snapshot(Event& event)
{
    update(event);

    SnapshotParser parser;
    parse(parser, event);
    parser.writePhrases(event);
}

void SnapshotTree::parse(SnapshotParser& parser, const Event& event)
{
    EventBufferPtr root;
    if (store_.find(event.getRootId(), root)) {
        parseNode(parser, root);
    }
}

void SnapshotTree::parseNode(SnapshotParser& parser, const EventBufferPtr& node)
{
    parser.parse(node);
    auto children = sortedChildren(node);
    for (const auto& child : children) {
        parseNode(parser, child);
    }
}

std::string SnapshotTree::getParentId(const Event& event)
{
    auto parentId = event.getParentId();
    if (parentId.length() == 0)
        parentId = event.getRootId();

    return parentId;
}

EventBufferVec SnapshotTree::sortedChildren(const EventBufferPtr& buffer)
{
    EventBufferVec output;

    const auto children = buffer->getEvent()->tree_children();
    if (children == nullptr)
        return output;

    EventBufferPtr child;
    for (auto it = children->begin(); it != children->end(); ++it) {
        if (store_.find(it->c_str(), child)) {
            output.push_back(std::move(child));
        }
    }

    radixsort<InitialSeqPred>(output.begin(), output.end());

    return output;
}

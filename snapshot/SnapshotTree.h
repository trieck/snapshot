#pragma once

#include "EventStore.h"
#include "Partition.h"
#include "SnapshotParser.h"
#include <EventBuffer.h>

class SnapshotTree
{
public:
    SnapshotTree();
    ~SnapshotTree();

    void snapshot(Partition* partition, std::ostream& os);
    void stats();
private:
    void process(Event& event);
    void insert(const Event& event);
    void insert(const Event& event, const std::string& parentId);
    void addChild(const std::string & parentId, const Event& event);
    void parentRemove(const std::string& parentId, const std::string& objectId);
    void destroy(const Event& event);
    void update(const Event& event);
    void reparent(const Event& event);
    void reparent(const Event& from, const Event& to);
    void snapshot(Event& event);
    void parse(SnapshotParser& parser, const Event& event);
    void parseNode(SnapshotParser& parser, const EventBufferPtr& node);
    std::string getParentId(const Event& event) const;
    EventBufferVec sortedChildren(const EventBufferPtr& buffer);
    EventStore store_;
};

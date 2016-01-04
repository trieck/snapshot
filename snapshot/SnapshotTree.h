#pragma once

#include "EventStore.h"
#include "Partition.h"
#include "SnapshotParser.h"

class SnapshotTree
{
public:
    SnapshotTree();
    ~SnapshotTree();

    void snapshot(Partition* partition);
    void stats();
private:
    void process(const Event& event);
    void insert(const Event& event);
    void insert(const Event& event, const std::string& parentId);
    void addChild(const std::string & parentId, const Event& event);
    void parentRemove(const std::string& parentId, const std::string& objectId);
    void destroy(const Event& event);
    void update(const Event& event);
    void reparent(const Event& event);
    void reparent(const Event& from, const Event& to);
    void snapshot(const Event& event);
    void parse(SnapshotParser& parser, const Event& event, int& count);
    void parseNode(SnapshotParser& parser, const Event& node, int& count);
    std::string getParentId(const Event& event) const;
    EventVec sortedChildren(const Event& event);
    EventStore store_;
};

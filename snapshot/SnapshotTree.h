#pragma once

#include "Index.h"
#include "Partition.h"

class SnapshotTree
{
public:
    SnapshotTree();
    ~SnapshotTree();

    void load(Partition* partition);
    void stats();
private:
    void process(const Event& event);
    void insert(const Event& event);
    void insert(const Event& event, const std::string& parentId);
    void addChild(const std::string& parentId, const std::string& objectId);
    void destroy(const Event& event);
    void update(const Event& event);
    void reparent(const Event& event);
    std::string getParentId(const Event& event) const;
    Index index_;
};


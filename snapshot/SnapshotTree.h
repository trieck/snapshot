#pragma once

#include "Index.h"

class SnapshotTree
{
public:
    SnapshotTree();
    ~SnapshotTree();

    void load(std::istream& stream);

    void process(const std::string& objectId, const std::string& event);

private:
    Index index_;
};


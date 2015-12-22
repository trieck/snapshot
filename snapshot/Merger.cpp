#include "stdafx.h"
#include "Merger.h"

namespace { const size_t NWAY = 100; }

Merger::Merger(const std::string& key) : key_(key)
{
    pass_ = 0;
    array = new mergerec*[NWAY + 1];
}

Merger::~Merger()
{
    delete[] array;
}

PartitionPtr Merger::merge(const PartitionVec& vec)
{
    pass_ = countpasses(vec.size());
    return mergemany(vec.size(), vec.begin());
}

size_t Merger::countpasses(size_t argc)
{
    uint32_t i = 0;

    if (argc <= NWAY)
        return 1;

    while (argc > 0) {
        i++;
        argc -= std::min(argc, NWAY);
    }

    return i + countpasses(i);
}

PartitionPtr Merger::mergeonce(size_t argc, PartitionVec::const_iterator it)
{
    pass_--;

    PartitionPtr output = Partition::makePartition(key_);
    output->open(std::ios::out);

    PartitionVec::const_iterator save = it;

    mergerec** recs = new mergerec*[argc + 1];

    uint32_t i;
    for (i = 0; i < argc; i++, it++) {
        recs[i] = new mergerec;
        recs[i]->stream = it->get();
        recs[i]->stream->open(std::ios::in);
    }

    recs[argc] = NULL;

    mergerec** list = recs;

    while (read(list)) {
        list = least(recs);
        write(output->stream(), list);
    }

    for (i = 0, it = save; i < argc; i++, it++) {
        recs[i]->stream->close();
        delete recs[i];
    }

    delete[] recs;

    output->close();

    return output;
}

bool Merger::read(mergerec** recs) const
{
    for (uint32_t i = 0; recs[i]; i++) {
        if (recs[i]->key == UINT64_MAX)
            return false;
        if (!(recs[i]->stream->stream() >> recs[i]->key))
            recs[i]->key = UINT64_MAX;
    }

    return true;
}

mergerec** Merger::least(mergerec** recs)
{
    auto j = 0, k = 0;

    for (auto i = 0; recs[i]; i++) {
        if (recs[i]->key < recs[k]->key) {
            k = i;
            j = 0;
            array[j++] = recs[i];
        } else if (recs[i]->key == recs[k]->key) {
            array[j++] = recs[i];
        }
    }

    array[j] = NULL;

    return array;
}

bool Merger::write(std::ostream& out, mergerec** recs)
{
    if (recs[0]->key == UINT64_MAX)
        return false;

    std::string event;
    for (auto i = 0; recs[i]; i++) {
        getline(recs[i]->stream->stream(), event);
        out << recs[i]->key << event << endl;
    }

    return true;
}

PartitionPtr Merger::mergemany(size_t argc, PartitionVec::const_iterator it)
{
    if (argc <= NWAY)
        return mergeonce(argc, it);

    PartitionVec output;
    output.resize(argc / NWAY + 1);

    size_t i = 0, n;
    for (i = 0; argc > 0; it += n, argc -= n) {
        n = std::min(argc, NWAY);
        output[i++] = mergeonce(n, it);
    }

    return mergemany(i, output.begin());
}

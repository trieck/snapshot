#include "stdafx.h"
#include "Merger.h"

namespace { const size_t NWAY = 100; }

Merger::Merger(const std::string & key) : key_(key)
{
    pass_ = 0;
    array = new mergerec*[NWAY + 1];
}

Merger::~Merger()
{
    close();
    delete[] array;
}

std::string Merger::merge(const PartitionVec& vec)
{
    stringvec input = transform(vec);
    pass_ = countpasses(input.size());
    return mergemany(input.size(), input.begin());
}

void Merger::close()
{
    if (out_.is_open()) {
        out_.close();
    }
}

stringvec Merger::transform(const PartitionVec& vec)
{
    stringvec output;

    for (const auto& p : vec) {
        output.push_back(p->getFilename());
    }

    return output;
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

std::string Merger::mergeonce(size_t argc, stringvec::const_iterator it)
{
    pass_--;

    std::string outfile = std::tmpnam(nullptr);

    close();

    out_.open(outfile);
    if (!out_.is_open()) {
        boost::format message = boost::format("unable to open file \"%s\".") % outfile;
        throw std::exception(message.str().c_str());
    }

    stringvec::const_iterator save = it;

    mergerec** recs = new mergerec*[argc + 1];

    uint32_t i;
    for (i = 0; i < argc; i++, it++) {
        recs[i] = new mergerec;
        recs[i]->key = 0ULL;
        recs[i]->stream.open(*it);
        if (!recs[i]->stream.is_open()) {
            boost::format message = boost::format("unable to open file \"%s\".") % *it;
            throw std::exception(message.str().c_str());
        }
    }

    recs[argc] = NULL;

    mergerec** list = recs;

    while (read(list)) {
        list = least(recs);
        write(list);
    }

    for (i = 0, it = save; i < argc; i++, it++) {
        recs[i]->stream.close();
        delete recs[i];
        _unlink((*it).c_str());
    }

    delete[] recs;

    close();

    return outfile;
}

bool Merger::read(mergerec** recs) const
{
    auto read = 0;

    for (uint32_t i = 0; recs[i]; i++) {
        if (recs[i]->stream >> recs[i]->key)
            read++;
    }

    return read != 0;
}

mergerec** Merger::least(mergerec** recs)
{
    auto j = 0, k = 0;

    for (auto i = 0; recs[i]; i++) {
        switch (memcmp(&recs[i]->key, &recs[k]->key, sizeof(uint64_t))) {
        case -1:
            k = i;
            j = 0;  // fall through
        case 0:
            array[j++] = recs[i];
        }
    }

    array[j] = NULL;

    return array;
}

bool Merger::write(mergerec** recs)
{
    return true;
}

std::string Merger::mergemany(size_t argc, stringvec::const_iterator it)
{
    if (argc <= NWAY)
        return mergeonce(argc, it);

    stringvec outfiles;
    outfiles.resize(argc / NWAY + 1);

    size_t i = 0, n;
    for (i = 0; argc > 0; it += n, argc -= n) {
        n = std::min(argc, NWAY);
        outfiles[i++] = mergeonce(n, it);
    }

    return mergemany(i, outfiles.begin());
}

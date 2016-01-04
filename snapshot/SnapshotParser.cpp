#include "stdafx.h"
#include "SnapshotParser.h"
#include <boost/algorithm/string.hpp>

SnapshotParser::SnapshotParser()
{
}

SnapshotParser::~SnapshotParser()
{
}

void SnapshotParser::addPhrase(const std::string& phrase, Phrase::PhraseStatus status)
{
    auto text = phrase;
    boost::replace_all(text, "\\xA0", " ");
    boost::trim(text);
    phrases_.push_back({ text, status });
}

void SnapshotParser::parse(const EventBufferPtr& event)
{
}

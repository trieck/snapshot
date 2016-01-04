#include "stdafx.h"
#include "SnapshotParser.h"
#include "EventBuffer.h"
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

void SnapshotParser::parse(const EventBufferPtr& buffer)
{
    auto text = buffer->getMeta("Text");
    auto value = buffer->getMeta("Value");
    auto targetType = buffer->getMeta("NormalTargetType");

    if (!boost::empty(text) && (
        boost::iequals(targetType, "Form") ||
        boost::iequals(targetType, "Mdi Child"))) {
        addPhrase(text, Phrase::PhraseStatus::Title);
    } else if (!boost::empty(text)) {
        addPhrase(text, Phrase::PhraseStatus::Regular);
    }

    if (!boost::empty(value) && !boost::equals(text, value)) {
        // TODO: parse xml
        addPhrase(value, Phrase::PhraseStatus::Regular);
    }
}

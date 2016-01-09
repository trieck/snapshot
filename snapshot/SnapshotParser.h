#pragma once

#include "Event.h"
#include "Phrase.h"

class SnapshotParser
{
public:
    SnapshotParser();
    ~SnapshotParser();

    void writePhrases(Event& event) const;
    void parse(const EventBufferPtr& event);
private:
    void addPhrase(const std::string& text, Phrase::PhraseStatus status);
    PhraseVec phrases_;
};


#pragma once

#include "Event.h"
#include "Phrase.h"

class SnapshotParser
{
public:
    SnapshotParser();
    ~SnapshotParser();

    void addPhrase(const std::string& text, Phrase::PhraseStatus status);
    void parse(const Event& event);
private:
    PhraseVec phrases_;
};


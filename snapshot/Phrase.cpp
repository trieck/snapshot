#include "stdafx.h"
#include "Phrase.h"

Phrase::Phrase(const std::string& text, PhraseStatus status) : status_(status), text_(text)
{
}

Phrase::Phrase(const Phrase& rhs)
{
    *this = rhs;
}

Phrase& Phrase::operator=(const Phrase& rhs)
{
    if (this != &rhs) {
        text_ = rhs.text_;
        status_ = rhs.status_;
    }

    return *this;
}

Phrase::~Phrase()
{
}

std::string Phrase::toString(PhraseStatus status)
{
    if (status == PhraseStatus::Regular)
        return "regular";
    if (status == PhraseStatus::Hidden)
        return "hidden";
    if (status == PhraseStatus::Title)
        return "title";

    return "";
}

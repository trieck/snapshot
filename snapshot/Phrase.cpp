#include "stdafx.h"
#include "Phrase.h"

Phrase::Phrase(const std::string& text, PhraseStatus status) : text_(text), status_(status)
{
}

Phrase::Phrase(const Phrase & rhs)
{
    *this = rhs;
}

Phrase & Phrase::operator=(const Phrase & rhs)
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

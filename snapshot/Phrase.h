#pragma once

class Phrase
{
public:
    enum class PhraseStatus {
        Regular,
        Hidden,
        Title
    };

    Phrase(const std::string& text, PhraseStatus status);
    Phrase(const Phrase& rhs);
    Phrase& operator =(const Phrase& rhs);
    ~Phrase();

    static std::string toString(PhraseStatus status);

    PhraseStatus status_;
    std::string text_;
};

using PhraseVec = std::vector<Phrase>;

#include "stdafx.h"

#include <boost/uuid/sha1.hpp>

std::string sha1(const std::string input)
{
    boost::uuids::detail::sha1 sha1;

    sha1.process_bytes(input.c_str(), input.size());

    uint32_t digest[5];
    sha1.get_digest(digest);

    std::ostringstream ss;
    ss << std::hex;

    for (auto i = 0; i < 5; ++i) {
        ss << digest[i];
    }

    return ss.str();
}

bool getline(std::istream& is, std::string& line)
{
    constexpr int BUF_SIZE = 4096;

    char buf[BUF_SIZE], *pbuf = buf;
    line.clear();

    auto streambuf = is.rdbuf();

    int c;
    for (;;) {
        c = streambuf->sbumpc();
        if (c == EOF) {
            *pbuf = '\0';
            line += buf;
            is.setstate(std::ios_base::eofbit);
            break;
        }
        else if (c == '\n') {
            *pbuf = '\0';
            line += buf;
            break;
        }
        else {
            if ((pbuf - buf) + 1 == BUF_SIZE) {
                *pbuf = '\0';
                line += buf;
                pbuf = buf;
            }
            *pbuf++ = (char)c;
        }
    }

    return line.length() > 0;
}
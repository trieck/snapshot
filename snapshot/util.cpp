#include "stdafx.h"
#include "util.h"

#include <boost/uuid/sha1.hpp>

std::string Util::sha1(const std::string input)
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

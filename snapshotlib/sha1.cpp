#include "snapshotlib.h"
#include "sha1.h"

std::string sha1(const std::string& input)
{
    uint32_t digest[5];
    sha1(input, digest);

    std::ostringstream ss;
    ss << std::hex;

    for (auto i = 0; i < 5; ++i) {
        ss << digest[i];
    }

    return ss.str();
}

void sha1(const std::string &input, boost::uuids::detail::sha1::digest_type digest)
{
    boost::uuids::detail::sha1 sha1;
    sha1.process_bytes(input.c_str(), input.size());
    sha1.get_digest(digest);
}

#include "snapshotlib.h"
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

std::string strerror()
{
    return ::strerror(errno);
}

std::string comma(uint64_t i)
{
    std::ostringstream os;

    std::ostringstream ss;
    ss << i;
    std::string input = ss.str();

    auto n = static_cast<int>(input.length());

    for (auto j = n - 1, k = 1; j >= 0; j--, k++) {
        os << input[j];
        if (k % 3 == 0 && j > 0 && j < n - 1)
            os << ',';
    }

    std::string output(os.str());
    std::reverse(output.begin(), output.end());

    return output;
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
        } else if (c == '\n') {
            *pbuf = '\0';
            line += buf;
            break;
        } else {
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
#include "snapshotlib.h"

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
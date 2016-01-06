#include "snapshotlib.h"

constexpr auto LINE_BUF_SIZE = 4096;

std::string strerror()
{
    return ::strerror(errno);
}

std::string comma(uint64_t i)
{
    std::ostringstream os;

    std::ostringstream ss;
    ss << i;
    auto input = ss.str();

    auto n = static_cast<int>(input.length());

    for (auto j = n - 1, k = 1; j >= 0; j--, k++) {
        os << input[j];
        if (k % 3 == 0 && j > 0 && j < n - 1)
            os << ',';
    }

    auto output(os.str());
    reverse(output.begin(), output.end());

    return output;
}

bool getline(std::istream& is, std::string& line)
{
    char buf[LINE_BUF_SIZE], *pbuf = buf;
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
            if ((pbuf - buf) + 1 == LINE_BUF_SIZE) {
                *pbuf = '\0';
                line += buf;
                pbuf = buf;
            }
            *pbuf++ = static_cast<char>(c);
        }
    }

    return line.length() > 0;
}
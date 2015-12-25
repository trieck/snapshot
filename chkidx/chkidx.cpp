#include "stdafx.h"
#include "Timer.h"
#include "Index.h"

void chkidx(const char* filename)
{
    Index index;
    index.open(filename, std::ios::in);

    cout << "    Index filename: " << filename << endl;
    cout << "    Index file size: " << comma(index.filesize()) << " bytes" << endl;
    cout << "    Hash table size: " << comma(index.tablesize()) << " buckets" << endl;
    cout << "    Hash table fill factor: " << boost::format("%02.2f%%") % index.fillfactor() << endl;
    cout << "    Longest run: " << comma(index.maxrun()) << " buckets" << endl;

    index.close();
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "usage: chkidx filename" << endl;
        exit(1);
    }

    Timer timer;

    try {
        chkidx(argv[1]);
    } catch (const std::exception& e) {
        cerr << e.what() << endl;
        exit(1);
    }

    cout << "    elapsed time " << timer << endl;

    return 0;
}

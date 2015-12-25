#include "stdafx.h"
#include "Timer.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "usage: chkidx filename" << endl;
        exit(1);
    }

    Timer timer;

    try {
        ;
    } catch (const std::exception& e) {
        cerr << e.what() << endl;
        exit(1);
    }

    cout << "    elapsed time " << timer << endl;

    return 0;
}


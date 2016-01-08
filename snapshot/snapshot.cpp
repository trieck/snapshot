#include "stdafx.h"
#include "Snapshotter.h"
#include "Timer.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "usage: snapshot filename" << flush << endl;
        exit(1);
    }

    Timer timer;

    Snapshotter snapshotter;
    try {
        snapshotter.snapshot(argv[1]);
    } catch (const std::exception& e) {
        cerr << e.what() << endl;
        exit(1);
    }

    cout << "    elapsed time " << timer << flush << endl;

    return 0;
}

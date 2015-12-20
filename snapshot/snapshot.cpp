// snapshot.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Snapshotter.h"
#include "Timer.h"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "usage: snapshot filename" << endl;
        exit(1);
    }

    Timer timer;

    Snapshotter snapshotter;
    try {
        snapshotter.snapshot(argv[1]);
    }
    catch (const std::exception& e) {
        cerr << e.what() << endl;
        exit(1);
    }

    cout << "    elapsed time " << timer << endl;

    return 0;
}
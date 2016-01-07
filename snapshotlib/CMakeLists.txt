cmake_minimum_required(VERSION 3.3)
project(snapshotlib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
include_directories(../../flatbuffers/include)
set(SOURCE_FILES
        util.cpp
        util.h
        Timer.cpp
        Timer.h
        snapshotlib.cpp
        snapshotlib.h
        fnvhash.h
        sha1.cpp
        sha1.h
        Primes.cpp
        Primes.h
        jsoncpp.cpp
        BlockIO.h
        BlockIO.cpp
        ByteBuffer.cpp
        ByteBuffer.h
        Event.cpp
        Event.h
        EventBuffer.cpp
        EventBuffer.h
        EventStore.cpp
        EventStore.h
        Repository.cpp
        Repository.h
        EventWriter.cpp
        EventWriter.h
        RandomPerm.cpp
        RandomPerm.h
        radixsort.h
        )

add_library(snapshotlib ${SOURCE_FILES})

find_package(Boost 1.59.0)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
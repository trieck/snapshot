cmake_minimum_required(VERSION 3.3)
project(snapshot)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(
        ../../flatbuffers/include
        ../snapshotlib
        ${Boost_INCLUDE_DIR}
)

find_package(Boost 1.59.0)

set(SOURCE_FILES
        snapshot.cpp
        Snapshotter.cpp
        Snapshotter.h
        SnapshotParser.cpp
        SnapshotParser.h
        Merger.cpp
        Merger.h
        Partition.cpp
        Partition.h
        Partitioner.cpp
        Partitioner.h
        Phrase.cpp
        Phrase.h
        SnapshotTree.cpp
        SnapshotTree.h
        stdafx.cpp
        stdafx.h)
add_executable(snapshot ${SOURCE_FILES})

target_link_libraries(snapshot snapshotlib)
add_dependencies(snapshot snapshotlib)
CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
PROJECT(snapshot)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

INCLUDE_DIRECTORIES(
        ../../../flatbuffers/include
        ../snapshotlib
        ${Boost_INCLUDE_DIR}
)

FIND_PACKAGE(Boost 1.68.0)

SET(SOURCE_FILES
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
ADD_EXECUTABLE(snapshot ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(snapshot snapshotlib)
ADD_DEPENDENCIES(snapshot snapshotlib)
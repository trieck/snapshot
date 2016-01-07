CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
PROJECT(snapshotlib)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
INCLUDE_DIRECTORIES(../../flatbuffers/include)
SET(SOURCE_FILES
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

ADD_LIBRARY(snapshotlib ${SOURCE_FILES})

FIND_PACKAGE(Boost 1.59.0)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_SOURCE_DIR}/event_generated.h
        COMMAND flatc -o ${CMAKE_CURRENT_SOURCE_DIR} --cpp ${CMAKE_HOME_DIRECTORY}/schema/event.fbs
        DEPENDS ${CMAKE_HOME_DIRECTORY}/schema/event.fbs
        )

ADD_CUSTOM_TARGET(compile_schema
        DEPENDS ${PROJECT_SOURCE_DIR}/event_generated.h
        )

ADD_DEPENDENCIES(snapshotlib compile_schema)


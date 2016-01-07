cmake_minimum_required(VERSION 3.3)
project(snapshotroot)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(snapshotlib)
add_subdirectory(snapshot)

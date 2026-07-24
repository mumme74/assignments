cmake_minimum_required(VERSION 3.25)

project(testdata)

include_directories(../types ../utils)

file(GLOB testdata_srcs *.c)
file(GLOB testdata_hdrs *.h)

add_library(testdata ${testdata_hdrs} ${testdata_srcs})
target_link_libraries(testdata memory types)

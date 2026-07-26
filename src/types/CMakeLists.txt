cmake_minimum_required(VERSION 3.25)

project(types)

include_directories(../mem, ../utils)

file(GLOB types_srcs *.c)
file(GLOB types_hdrs *.h)

add_library(types ${types_hdrs} ${types_srcs})
target_link_libraries(types utils)

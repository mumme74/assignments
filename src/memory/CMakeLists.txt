cmake_minimum_required(VERSION 3.25)

project(memory)

file(GLOB memory_srcs *.c)
file(GLOB memory_hdrs *.h)

add_library(memory ${memory_hdrs} ${memory_srcs})

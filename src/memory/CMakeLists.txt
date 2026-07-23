cmake_minimum_required(VERSION 3.25)

project(memorylib)

file(GLOB memorylib_srcs *.c)
file(GLOB memorylib_hdrs *.h)

add_library(memorylib ${memorylib_hdrs} ${memorylib_srcs})

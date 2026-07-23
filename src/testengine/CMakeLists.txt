cmake_minimum_required(VERSION 3.25)

project(testengine)

file(GLOB testengine_srcs *.c)
file(GLOB testengine_hdrs *.h)

add_library(testengine ${testengine_hdrs} ${testengine_srcs})

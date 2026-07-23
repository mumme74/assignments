cmake_minimum_required(VERSION 3.25)

project(testdata)

file(GLOB testdata_srcs *.c)
file(GLOB testdata_hdrs *.h)

add_library(testdata ${testdata_hdrs} ${testdata_srcs})

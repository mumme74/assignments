cmake_minimum_required(VERSION 3.25)

project(utils)

file(GLOB utils_srcs *.c)
file(GLOB utils_hdrs *.h)

add_library(utils ${utils_hdrs} ${utils_srcs})

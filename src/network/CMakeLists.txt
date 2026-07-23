cmake_minimum_required(VERSION 3.25)

project(network)

file(GLOB network_srcs *.c)
file(GLOB network_hdrs *.h)

add_library(network ${network_hdrs} ${network_srcs})

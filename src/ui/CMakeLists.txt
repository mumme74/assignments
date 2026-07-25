cmake_minimum_required(VERSION 3.25)

project(ui)

include_directories(../mem ../utils)

file(GLOB ui_srcs *.c)
file(GLOB ui_hdrs *.h)

add_library(ui ${ui_hdrs} ${ui_srcs})

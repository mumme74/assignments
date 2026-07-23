cmake_minimum_required(VERSION 3.25)
project(assignments VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

option(BUILD_TESTS "Build unit tests" FALSE)

if (MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -Werror -Wpedantic)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address -fno-omit-frame-pointer)
    endif()
endif()

add_subdirectory(src)
include_directories(src)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

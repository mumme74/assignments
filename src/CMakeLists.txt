cmake_minimum_required(VERSION 3.25)

set(SUBDIRS
    memory
    network
    testdata
    testengine
    utils
)

foreach(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
endforeach(subdir ${SUBDIRS})


include_directories(memory network testdata testengine)

set(pupil_srcs pupil.c)
set(teacher_srcs teacher.c)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}..)

# pupil bin
add_executable(pupil ${pupil_srcs})
target_link_libraries(pupil memory)


# teacher bin
add_executable(teacher ${teacher_srcs})
target_link_libraries(teacher memory)



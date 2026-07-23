
if (NOT TESTRUNNER_DIR)
    message(FATAL_ERROR "You must set TESTRUNNER_DIR, example: cmake .. -DTESTRUNNER_DIR=...")
endif()

set(SUBDIRS
    types
    memory
    network
    testdata
    testengine
    utils
)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/)

foreach(subdir ${SUBDIRS})
    include_directories(${SRC_DIR}${subdir})
endforeach(subdir ${SUBDIRS})


include_directories(${TESTRUNNER_DIR}/include)
find_library(TESTRUNNER testrunner PATHS ${TESTRUNNER_DIR}/lib)


file(GLOB test_srcs test_*.c)

foreach(testfile ${test_srcs})
    get_filename_component(test_name ${testfile} NAME_WE)
    add_executable(${test_name} ${testfile})
    target_link_libraries(${test_name} PUBLIC ${TESTRUNNER} ${SUBDIRS})
endforeach()

add_executable(testall ${test_srcs})
target_link_libraries(testall PUBLIC ${TESTRUNNER} ${SUBDIRS})
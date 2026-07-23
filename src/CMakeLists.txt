cmake_minimum_required(VERSION 3.25)

add_subdirectory(memory)

include_directories(memory)

set(pupil_srcs pupil.c)
set(teacher_srcs teacher.c)


set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")



# pupil bin
add_executable(pupil ${pupil_srcs})
target_link_libraries(pupil memorylib)


# teacher bin
add_executable(teacher ${teacher_srcs})
target_link_libraries(teacher memorylib)



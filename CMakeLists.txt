cmake_minimum_required (VERSION 2.8 FATAL_ERROR)

PROJECT (logpipe C)

# get rid of the extra default configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Limited configurations" FORCE)

#
# Enable extra compiler warnings
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -Wno-unused-parameter")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")

add_executable(test_str test/test_str.c src/str.c)
add_dependencies(test_str test_str)
add_test(test_str bin/test_str)
target_link_libraries(test_str logpipe_core)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(test_str -lrt)
endif ()

add_custom_target(bin-dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory bin/)

add_subdirectory(src)
add_subdirectory(extsrc)
add_subdirectory(mod)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/extsrc)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/mod)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
ADD_EXECUTABLE(logpipe logpipe.c)
target_link_libraries(logpipe logpipe_mod logpipe_core logpipe_ext)
set(EXECUTABLE_OUTPUT_PATH bin/)

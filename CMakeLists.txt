cmake_minimum_required (VERSION 2.8 FATAL_ERROR)

PROJECT (logpipe C)

enable_testing()

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
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "${warnings} -Weverything")
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

add_executable(test_steps test/test_steps.c)
add_dependencies(test_steps test_steps)
add_test(test_steps bin/test_steps)
target_link_libraries(test_steps logpipe_core)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(test_steps -lrt)
endif ()

add_executable(test_logpipe test/test_logpipe.c)
add_dependencies(test_logpipe test_logpipe)
add_test(test_logpipe bin/test_logpipe)
target_link_libraries(test_logpipe logpipe_core)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(test_logpipe -lrt)
endif ()

add_executable(test_querystring test/test_querystring.c)
add_dependencies(test_querystring test_querystring)
add_test(test_querystring bin/test_querystring)
target_link_libraries(test_querystring logpipe_core)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(test_querystring -lrt)
endif ()

add_custom_target(bin-dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory bin/)

add_subdirectory(src)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
ADD_EXECUTABLE(logpipe logpipe.c)
target_link_libraries(logpipe logpipe_core)
set(EXECUTABLE_OUTPUT_PATH bin/)

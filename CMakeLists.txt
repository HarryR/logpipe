cmake_minimum_required (VERSION 2.8 FATAL_ERROR)

PROJECT (apacheclf2json C)

# get rid of the extra default configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Limited configurations" FORCE)

#
# Enable extra compiler warnings
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")

#
# Ragel is used by the parser
FIND_PROGRAM (RAGEL "ragel")
IF (RAGEL)
    MESSAGE (STATUS "Ragel found at: ${RAGEL}")
ELSE (RAGEL)
    MESSAGE (fatal_error "ragel not found")
ENDIF (RAGEL)
function(ragel_gen in_rl)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${in_rl}.c
      COMMAND ${RAGEL} -G2 -C -o ${CMAKE_CURRENT_BINARY_DIR}/${in_rl}.c ${CMAKE_CURRENT_SOURCE_DIR}/${in_rl}.rl -I ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${in_rl}.rl
      )
endfunction(ragel_gen)


#
# Check if _GNU_SOURCE is available.
# Check if we need GNU extensions. E.g. for S_ISSOCK etc.
INCLUDE(CheckFunctionExists)
include(CheckCSourceCompiles)
check_c_source_compiles("
#include <features.h>
#ifndef __GNU_LIBRARY__
#error no gnu here
#endif
" _GNU_SOURCE)
if (_GNU_SOURCE)
    add_definitions(-D_GNU_SOURCE)
endif()


FILE(GLOB sources "${PROJECT_SOURCE_DIR}/src/*.c")
FILE(GLOB includes "${PROJECT_SOURCE_DIR}/src/*.h")

# We have a portable timegm function available
CHECK_FUNCTION_EXISTS (timegm HAVE_TIMEGM)
IF (NOT HAVE_TIMEGM)
    MESSAGE(STATUS "Using portable timegm")
    SET(sources ${sources} portable/timegm.c)
    SET(includes ${sources} portable/timegm.h)
ENDIF (NOT HAVE_TIMEGM)


# Portable memrchr 
CHECK_FUNCTION_EXISTS (memrchr HAVE_MEMRCHR)
IF (NOT HAVE_MEMRCHR)
    MESSAGE (STATUS "Using portable memrchr")
    SET(sources ${sources} portable/memrchr.c)
    SET(includes ${sources} portable/memrchr.h)
ENDIF (NOT HAVE_MEMRCHR)




CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/hiredis)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/portable)

add_custom_target(bin-dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ../bin/)
RAGEL_GEN(src/parser_apacheclf)
ADD_EXECUTABLE(apacheclf2json ${sources} ${CMAKE_CURRENT_BINARY_DIR}/src/parser_apacheclf.c)
set(EXECUTABLE_OUTPUT_PATH ../bin/)
TARGET_LINK_LIBRARIES(apacheclf2json ${CMAKE_CURRENT_BINARY_DIR}/hiredis/libhiredis.a)
INSTALL(TARGETS apacheclf2json DESTINATION bin)

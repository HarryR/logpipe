cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

PROJECT (apacheclf2json C)

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

FIND_PROGRAM (RAGEL "ragel")
IF (RAGEL)
    MESSAGE (STATUS "Ragel found at: ${RAGEL}")
ELSE (RAGEL)
    MESSAGE (fatal_error "ragel not found")
ENDIF (RAGEL)


INCLUDE(CheckFunctionExists)
CHECK_FUNCTION_EXISTS (memrchr HAVE_MEMRCHR)
CHECK_FUNCTION_EXISTS (timegm HAVE_TIMEGM)


FILE(GLOB sources "${PROJECT_SOURCE_DIR}/src/*.c")
FILE(GLOB includes "${PROJECT_SOURCE_DIR}/src/*.h")


IF (NOT HAVE_TIMEGM)
    SET(sources ${sources} portable/timegm.c)
    SET(includes ${sources} portable/timegm.h)
ENDIF (NOT HAVE_TIMEGM)

IF (NOT HAVE_MEMRCHR)
    MESSAGE (STATUS "Using portable memrchr")
    SET(sources ${sources} portable/memrchr.c)
    SET(includes ${sources} portable/memrchr.h)
ELSE (NOT HAVE_MEMRCHR)
    MESSAGE (STATUS "${HAVE_MEMRCHR}")
ENDIF (NOT HAVE_MEMRCHR)

function(ragel_gen in_rl)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${in_rl}.c
      COMMAND ${RAGEL} -G2 -C -o ${CMAKE_CURRENT_BINARY_DIR}/${in_rl}.c ${CMAKE_CURRENT_SOURCE_DIR}/${in_rl}.rl -I ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${in_rl}.rl
      )
endfunction(ragel_gen)

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/portable)

RAGEL_GEN(src/parser)
ADD_EXECUTABLE(apacheclf2json ${sources} ${CMAKE_CURRENT_BINARY_DIR}/src/parser.c)
INSTALL(TARGETS apacheclf2json DESTINATION bin)

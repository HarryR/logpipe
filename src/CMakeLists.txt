PROJECT (logpipe_core C)

INCLUDE(CheckFunctionExists)

# We have a portable timegm function available
CHECK_FUNCTION_EXISTS (timegm HAVE_TIMEGM)
IF (NOT HAVE_TIMEGM)
    MESSAGE(STATUS "Using portable timegm")
    SET(core_sources ${core_sources} "${PROJECT_SOURCE_DIR}/portable/timegm.c")
    SET(core_includes ${core_sources} "${PROJECT_SOURCE_DIR}/portable/timegm.h")
ENDIF (NOT HAVE_TIMEGM)

# Portable memrchr 
CHECK_FUNCTION_EXISTS (memrchr HAVE_MEMRCHR)
IF (NOT HAVE_MEMRCHR)
    MESSAGE (STATUS "Using portable memrchr")
    SET(core_sources ${core_sources} "${PROJECT_SOURCE_DIR}/portable/memrchr.c")
    SET(core_includes ${core_sources} "${PROJECT_SOURCE_DIR}/portable/memrchr.h")
ENDIF (NOT HAVE_MEMRCHR)


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
#ifndef _GNU_SOURCE
#error no gnu here
#endif
int main() {}
" _GNU_SOURCE)
if (_GNU_SOURCE)
    add_definitions(-D_GNU_SOURCE)
endif()

#
# Check if _XOPEN_SOURCE is available.
# This is required for strptime.h on Linux
check_c_source_compiles("
#include <features.h>
#ifndef _XOPEN_SOURCE
#error no xopen here
#endif
int main() {}
" _XOPEN_SOURCE)
if (_XOPEN_SOURCE)
    add_definitions(-D_XOPEN_SOURCE)
endif()

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third-party)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/..)


FILE(GLOB core_sources "${PROJECT_SOURCE_DIR}/*.c")
FILE(GLOB core_includes "${PROJECT_SOURCE_DIR}/*.h")

RAGEL_GEN(third-party/apacheclf)
RAGEL_GEN(third-party/squid)
FILE(GLOB mod_sources "${PROJECT_SOURCE_DIR}/mod/*.c")
set(mod_sources ${mod_sources} mod/apacheclf.c mod/squid.c)

FILE(GLOB thirdparty_sources "${PROJECT_SOURCE_DIR}/third-party/*.c")
FILE(GLOB thirdparty_includes "${PROJECT_SOURCE_DIR}/third-party/*.h")

add_library(logpipe_core STATIC ${core_sources} ${mod_sources} ${thirdparty_sources})

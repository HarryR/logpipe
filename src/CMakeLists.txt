PROJECT (logpipe_core C)

INCLUDE(CheckFunctionExists)
INCLUDE(CheckStructHasMember)

FILE(GLOB core_sources "${PROJECT_SOURCE_DIR}/*.c")
FILE(GLOB core_includes "${PROJECT_SOURCE_DIR}/*.h")

# We have a portable timegm function available
CHECK_FUNCTION_EXISTS (timegm HAVE_TIMEGM)
IF (NOT HAVE_TIMEGM)
    MESSAGE(STATUS "Using portable timegm")
    SET(core_sources ${core_sources} "${PROJECT_SOURCE_DIR}/portable/timegm.c")
    SET(core_includes ${core_sources} "${PROJECT_SOURCE_DIR}/portable/timegm.h")
ENDIF (NOT HAVE_TIMEGM)

# Portable strptime for Windows
CHECK_FUNCTION_EXISTS (strptime HAVE_STRPTIME)
IF (NOT HAVE_STRPTIME OR PORTABLE_STRPTIME)
    SET(PORTABLE_STRPTIME 1)
    UNSET(HAVE_STRPTIME)
    MESSAGE(STATUS "Using portable strptime")
    SET(core_sources ${core_sources} "${PROJECT_SOURCE_DIR}/portable/strptime.c")
    SET(core_includes ${core_sources} "${PROJECT_SOURCE_DIR}/portable/strptime.h")
ENDIF (NOT HAVE_STRPTIME OR PORTABLE_STRPTIME)

# Portable strncasecmp for Windows
CHECK_FUNCTION_EXISTS (strncasecmp HAVE_STRNCASECMP)
IF (NOT HAVE_STRNCASECMP)
    MESSAGE(STATUS "Using portable strncasecmp")
    SET(core_sources ${core_sources} "${PROJECT_SOURCE_DIR}/portable/strncasecmp.c")
    SET(core_includes ${core_sources} "${PROJECT_SOURCE_DIR}/portable/strncasecmp.h")
ENDIF (NOT HAVE_STRNCASECMP)

# Portable strndup (for Windows)
CHECK_FUNCTION_EXISTS (strndup HAVE_STRNDUP)
IF (NOT HAVE_STRNDUP)
    MESSAGE (STATUS "Using portable strndup")
    SET(core_sources ${core_sources} "${PROJECT_SOURCE_DIR}/portable/strndup.c")
    SET(core_includes ${core_sources} "${PROJECT_SOURCE_DIR}/portable/strndup.h")
ENDIF (NOT HAVE_STRNDUP)

# Portable memrchr (for OSX)
CHECK_FUNCTION_EXISTS (memrchr HAVE_MEMRCHR)
IF (NOT HAVE_MEMRCHR)
    MESSAGE (STATUS "Using portable memrchr")
    SET(core_sources ${core_sources} "${PROJECT_SOURCE_DIR}/portable/memrchr.c")
    SET(core_includes ${core_sources} "${PROJECT_SOURCE_DIR}/portable/memrchr.h")
ENDIF (NOT HAVE_MEMRCHR)

CHECK_FUNCTION_EXISTS (syslog HAVE_SYSLOG)
IF (HAVE_SYSLOG)
    MESSAGE(STATUS "Enabling module: syslog")
    SET(core_sources ${core_sources} "${PROJECT_SOURCE_DIR}/optmod/syslog.c")
ENDIF (HAVE_SYSLOG)

CHECK_FUNCTION_EXISTS (tzset HAVE_TZSET)
CHECK_FUNCTION_EXISTS (localtime_s HAVE_LOCALTIME_S)

check_struct_has_member ("struct tm" tm_gmtoff time.h HAVE_TM_GMTOFF)
check_struct_has_member ("struct tm" tm_zone time.h HAVE_TM_ZONE)

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

RAGEL_GEN(querystring)
RAGEL_GEN(strpair_split)
set(core_sources ${core_sources} querystring.c strpair_split.c)

RAGEL_GEN(mod/apacheclf)
RAGEL_GEN(mod/squid)
FILE(GLOB mod_sources "${PROJECT_SOURCE_DIR}/mod/*.c")
set(mod_sources ${mod_sources} mod/apacheclf.c mod/squid.c)

FILE(GLOB thirdparty_sources "${PROJECT_SOURCE_DIR}/third-party/*.c")
FILE(GLOB thirdparty_includes "${PROJECT_SOURCE_DIR}/third-party/*.h")

add_library(logpipe_core STATIC ${core_sources} ${mod_sources} ${thirdparty_sources})

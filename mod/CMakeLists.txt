cmake_minimum_required (VERSION 2.8 FATAL_ERROR)

PROJECT (logpipe_mod C)

FILE(GLOB mod_sources "${PROJECT_SOURCE_DIR}/*.c")
FILE(GLOB mod_includes "${PROJECT_SOURCE_DIR}/*.h")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../src/)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../extsrc/)

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

RAGEL_GEN(apacheclf)
RAGEL_GEN(squid)

add_custom_target(ragel_graphs
  COMMAND ragel -V apacheclf.rl  > apacheclf.dot && dot -Tpng apacheclf.dot > apacheclf.png
  COMMAND ragel -V squid.rl  > squid.dot && dot -Tpng squid.dot > squid.png
)

set(mod_sources ${mod_sources} apacheclf.c squid.c)

add_library(logpipe_mod STATIC ${mod_sources})
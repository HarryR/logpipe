cmake_minimum_required (VERSION 2.8 FATAL_ERROR)

PROJECT (logpipe_ext C)

INCLUDE(CheckFunctionExists)

FILE(GLOB ext_sources "${PROJECT_SOURCE_DIR}/*.c")
FILE(GLOB ext_includes "${PROJECT_SOURCE_DIR}/*.h")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

add_library(logpipe_ext STATIC ${ext_sources})

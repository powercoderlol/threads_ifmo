project(threads)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

set(SOURCES
  integral_mt_test.cpp
  integral.hpp
  mt_utils.hpp)

add_executable(mt_integral_test ${SOURCES})
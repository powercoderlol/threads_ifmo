project(threads)
cmake_minimum_required(VERSION 3.10)

find_package(OpenMP)
if(OPENMP_FOUND)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})

set(SOURCES
  matrix_omp_test.cpp)

add_executable(matrix_omp_test ${SOURCES})

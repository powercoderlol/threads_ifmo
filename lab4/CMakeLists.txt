project(dijkstra)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenMP REQUIRED)

if(OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})

set(SOURCES
  main.cpp
  dijkstra.hpp
  utils.cpp
  utils.hpp)

add_executable(dijkstra ${SOURCES})

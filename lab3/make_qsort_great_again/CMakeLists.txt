project(make_qsort_great_again)
cmake_minimum_required(VERSION 3.10)

find_package(MPI REQUIRED)

set(SOURCES
  main.cpp
  qsort.hpp
)

add_executable(mqga ${SOURCES})
target_include_directories(mqga 
  PRIVATE
  ${MPI_INCLUDE_PATH})
target_link_libraries(mqga 
  PRIVATE
  ${MPI_CXX_LIBRARIES})
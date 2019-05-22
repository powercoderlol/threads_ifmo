project(jacobi)
cmake_minimum_required(VERSION 3.10)

find_package(MPI REQUIRED)

set(SOURCES
  main.cpp
  jacobi.hpp
  numeric_traits.hpp
  utils.hpp
)

add_executable(jacobi ${SOURCES})
target_include_directories(jacobi 
  PRIVATE
  ${MPI_INCLUDE_PATH})
target_link_libraries(jacobi 
  ${MPI_CXX_LIBRARIES})
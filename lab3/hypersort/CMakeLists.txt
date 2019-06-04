project(hypersort)
cmake_minimum_required(VERSION 3.10)

find_package(MPI REQUIRED)

set(SOURCES
  main.cpp
)

add_executable(hypersort ${SOURCES})
target_include_directories(hypersort 
  PRIVATE
  ${MPI_INCLUDE_PATH})
target_link_libraries(hypersort 
  ${MPI_CXX_LIBRARIES}
  )

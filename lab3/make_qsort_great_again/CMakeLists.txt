project(make_qsort_great_again)
cmake_minimum_required(VERSION 3.10)

find_package(MPI REQUIRED)

# u MUST to set
# BOOST_ROOT 
# or
# BOOST_LIBRARYDIR
# to build target with boost mpi wrappers
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED mpi)

set(SOURCES
  main.cpp
  qsort.hpp
  hyperqsort.hpp
  utils.hpp
)

if(Boost_FOUND)
  message(STATUS "BOOST INCLUDE DIRS: " ${Boost_INCLUDE_DIRS})
  message(STATUS "BOOST LIBRARIES DIR: " ${Boost_LIBRARY_DIRS})
  message(STATUS "BOOST LINK LIBRARIES: " ${Boost_LIBRARIES})
  include_directories(${Boost_INCLUDE_DIRS})  
endif()

add_executable(mqga ${SOURCES})
target_include_directories(mqga 
  PRIVATE
  ${MPI_INCLUDE_PATH})
target_link_libraries(mqga 
  ${MPI_CXX_LIBRARIES}
  ${Boost_LIBRARIES}
  )
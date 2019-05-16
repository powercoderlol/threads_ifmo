project(dijkstra)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenMP REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS iostreams date_time REQUIRED)

if(OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if(Boost_FOUND)
  message(STATUS "BOOST INCLUDE DIRS: " ${Boost_INCLUDE_DIRS})
  message(STATUS "BOOST LIBRARIES DIR: " ${Boost_LIBRARY_DIRS})
  message(STATUS "BOOST LINK LIBRARIES: " ${Boost_LIBRARIES})
  include_directories(${Boost_INCLUDE_DIRS})  
endif()

message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})

set(SOURCES
  main.cpp
  dijkstra.hpp
  utils.cpp
  utils.hpp)

add_executable(dijkstra ${SOURCES})
target_link_libraries(dijkstra
  ${Boost_LIBRARIES})

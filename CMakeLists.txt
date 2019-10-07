cmake_minimum_required(VERSION 3.15)
project(cxxmath)

set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost  REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

include_directories(.)
add_subdirectory(test)

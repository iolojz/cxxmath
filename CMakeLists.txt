cmake_minimum_required(VERSION 3.14)
include(GNUInstallDirs)

project(cxxmath)

set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

include_directories(.)
add_subdirectory(test)

install(DIRECTORY concepts/ DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/cxxmath/concepts)
install(DIRECTORY core/ DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/cxxmath/core)
install(DIRECTORY helpers/ DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/cxxmath/helpers)
install(DIRECTORY models/ DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/cxxmath/models)
install(FILES cxxmath.hpp DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/cxxmath)

configure_file(cxxmath-config.cmake ${CMAKE_BINARY_DIR}/cxxmath-config.cmake)
install(FILES ${CMAKE_BINARY_DIR}/cxxmath-config.cmake DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/cxxmath)

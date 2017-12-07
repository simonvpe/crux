cmake_minimum_required(VERSION 3.8)
project(redux LANGUAGES CXX)

# Default to building release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Default CMAKE flags
if(NOT CMAKE_CXX_FLAGS)
  set(CMAKE_CXX_FLAGS "-Wall -Wextra")
endif()

if(NOT CMAKE_CXX_FLAGS_DEBUG)
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

if(NOT CMAKE_CXX_FLAGS_RELEASE)
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

enable_testing()
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(doctest)

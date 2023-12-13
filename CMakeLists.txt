cmake_minimum_required(VERSION 3.25)
project(market)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if(CODE_COVERAGE)
  include(CodeCoverage)
  include(MarketCov)
  append_coverage_compiler_flags()
endif()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Wpedantic")

add_subdirectory(libs)

enable_testing()
add_subdirectory(tests)

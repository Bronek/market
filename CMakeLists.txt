cmake_minimum_required(VERSION 3.25)
project(market)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Wpedantic")

add_subdirectory(libs)

enable_testing()
add_subdirectory(tests)

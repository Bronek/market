cmake_minimum_required(VERSION 3.9)
project(market)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Wpedantic")

add_subdirectory(apps)
add_subdirectory(libs)
add_subdirectory(demos)

enable_testing()
add_subdirectory(tests)

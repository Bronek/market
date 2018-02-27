cmake_minimum_required(VERSION 3.9)
project(market)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(apps)
add_subdirectory(libs)
add_subdirectory(demos)

enable_testing()
add_subdirectory(tests)

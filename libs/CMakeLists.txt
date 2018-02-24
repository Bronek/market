cmake_minimum_required(VERSION 3.9)
project(libs)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        book.hpp book.cpp)

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

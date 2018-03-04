cmake_minimum_required(VERSION 3.9)
project(libs)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        market/book.hpp market/book.cpp common/utils.hpp common/utils.cpp market/market.hpp market/market.cpp)

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

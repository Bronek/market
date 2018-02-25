cmake_minimum_required(VERSION 3.9)
project(tests)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        main.cpp dummy.cpp)

set(CATCH2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/catch2)
add_library(catch2 INTERFACE)
target_include_directories(catch2 INTERFACE ${CATCH2_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} libs catch2)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} -r junit)

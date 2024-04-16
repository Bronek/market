cmake_minimum_required(VERSION 3.25)
project(tests)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
        main.cpp market.cpp utils.cpp book.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} libs Catch2::Catch2)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} -r junit)

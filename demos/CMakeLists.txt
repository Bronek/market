cmake_minimum_required(VERSION 3.9)
project(demo)

set(SOURCE_FILES
        main.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} libs)

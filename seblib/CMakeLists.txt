CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
project(seblib)

set(LIB_NAME seblib)

add_library(${LIB_NAME})

file(GLOB SOURCES src/*.cpp)
target_sources(${LIB_NAME} PRIVATE ${SOURCES})
target_include_directories(${LIB_NAME} PRIVATE ../../ext-lib/raylib-cpp/include)
target_include_directories(${LIB_NAME} PRIVATE ../../ext-lib/raylib/src)

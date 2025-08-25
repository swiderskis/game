CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

project(seblib)

add_library(
    seblib
    src/sl-hot-reload.cpp
    src/sl-log.cpp
    src/sl-math.cpp
)

target_compile_options(seblib PRIVATE -Wall)
target_compile_options(seblib PRIVATE -Wextra)
target_compile_options(seblib PRIVATE -Werror)
target_compile_options(seblib PRIVATE -Wpedantic)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(seblib PRIVATE -O2)
endif()

target_include_directories(
    seblib
    PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/raylib/src
    ${CMAKE_SOURCE_DIR}/raylib-cpp/include
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(SLOG_LVL=1)
endif()

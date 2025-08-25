CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

project(seb-engine)

add_library(
    seb-engine
    src/se-ecs.cpp
    src/se-ui.cpp
    src/seb-engine.cpp
)

target_compile_options(seb-engine PRIVATE -Wall)
target_compile_options(seb-engine PRIVATE -Wextra)
target_compile_options(seb-engine PRIVATE -Werror)
target_compile_options(seb-engine PRIVATE -Wpedantic)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(seb-engine PRIVATE -O2)
endif()

target_include_directories(
    seb-engine
    PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/raylib/src
    ${CMAKE_SOURCE_DIR}/raylib-cpp/include
    ${CMAKE_SOURCE_DIR}/seblib/include
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(SLOG_LVL=1)
endif()

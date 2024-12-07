# TODO make this cross-platform
# TODO add hot reloading support
# TODO enable warning flags

CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
project(game)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BIN_NAME game)
set(CMAKE_CXX_STANDARD 23)

add_executable(${BIN_NAME})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext-lib/raylib)
target_link_libraries(${BIN_NAME} PRIVATE raylib)

file(GLOB SOURCES src/*.cpp)
target_sources(${BIN_NAME} PRIVATE ${SOURCES})
target_include_directories(${BIN_NAME} PRIVATE ext-lib/raylib-cpp/include)
target_include_directories(${BIN_NAME} PRIVATE ext-lib/raylib/src)

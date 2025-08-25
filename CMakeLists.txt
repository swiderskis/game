cmake_minimum_required(VERSION 3.13)

project(game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "")
endif()

add_executable(
    game
    src/components.cpp
    src/entities.cpp
    src/game.cpp
    src/hot-reload.cpp
    src/main.cpp
    src/sprites.cpp
    src/systems.cpp
)

target_compile_options(game PRIVATE -Wall)
target_compile_options(game PRIVATE -Wextra)
target_compile_options(game PRIVATE -Werror)
target_compile_options(game PRIVATE -Wpedantic)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(game PRIVATE -O2)
endif()

target_include_directories(
    game
    PRIVATE
    include
    raylib/src
    raylib-cpp/include
    seb-engine/include
    seblib/include
)

add_subdirectory(raylib)
add_subdirectory(seblib)
add_subdirectory(seb-engine)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(SLOG_LVL=1)
endif()

target_link_libraries(game PRIVATE raylib)
target_link_libraries(game PRIVATE seblib)
target_link_libraries(game PRIVATE seb-engine)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_library(
        gamelib
        SHARED
        src/components.cpp
        src/entities.cpp
        src/game.cpp
        src/hot-reload.cpp
        src/sprites.cpp
        src/systems.cpp
    )

    target_compile_options(gamelib PRIVATE -Wall)
    target_compile_options(gamelib PRIVATE -Wextra)
    target_compile_options(gamelib PRIVATE -Werror)
    target_compile_options(gamelib PRIVATE -Wpedantic)
    target_compile_options(gamelib PRIVATE -O2)

    target_include_directories(
        gamelib
        PRIVATE
        include
        raylib/src
        raylib-cpp/include
        seb-engine/include
        seblib/include
    )

    add_compile_definitions(TEXTURES="${CMAKE_SOURCE_DIR}/assets/texture-sheet.png")
    if(WIN32)
        add_compile_definitions(SO_NAME="${CMAKE_BINARY_DIR}/gamelib.dll")
    else()
        add_compile_definitions(SO_NAME="${CMAKE_BINARY_DIR}/libgamelib.so")
    endif()

    target_link_libraries(gamelib PRIVATE raylib)
    target_link_libraries(gamelib PRIVATE seblib)
    target_link_libraries(gamelib PRIVATE seb-engine)
endif()

cmake_minimum_required(VERSION 3.13)

project(game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_executable(
    game
    src/main.cpp
)

if(NOT WIN32)
    target_compile_options(game PRIVATE -Wall)
    target_compile_options(game PRIVATE -Wextra)
    target_compile_options(game PRIVATE -Werror)
    target_compile_options(game PRIVATE -Wpedantic)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(game PRIVATE -O1)
    endif()
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

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_SHARED_LIBS ON CACHE BOOL "")
endif()

add_subdirectory(raylib)
add_subdirectory(seblib)
add_subdirectory(seb-engine)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(SLOG_LVL=1)
    if(WIN32)
        add_compile_definitions(NDEBUG)
    endif()
endif()

target_link_libraries(game PRIVATE gamelib)
target_link_libraries(game PRIVATE raylib)
target_link_libraries(game PRIVATE seblib)
target_link_libraries(game PRIVATE seb-engine)

# https://stackoverflow.com/questions/14089284/copy-all-dlls-that-an-executable-links-to-to-the-executable-directory/73550650#73550650
if(WIN32 AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
  add_custom_command(
    TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:game> $<TARGET_FILE_DIR:game>
    COMMAND_EXPAND_LISTS
  )
endif()

add_library(
    gamelib
    src/components.cpp
    src/entities.cpp
    src/game.cpp
    src/hot-reload.cpp
    src/sprites.cpp
    src/systems.cpp
    src/tiles.cpp
)

if(NOT WIN32)
    target_compile_options(gamelib PRIVATE -Wall)
    target_compile_options(gamelib PRIVATE -Wextra)
    target_compile_options(gamelib PRIVATE -Werror)
    target_compile_options(gamelib PRIVATE -Wpedantic)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(gamelib PRIVATE -O2)
    endif()
endif()

target_include_directories(
    gamelib
    PRIVATE
    include
    raylib/src
    raylib-cpp/include
    seb-engine/include
    seblib/include
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(SLOG_LVL=1)
    if(WIN32)
        add_compile_definitions(NDEBUG)
    endif()
else()
    add_compile_definitions(TEXTURES="${CMAKE_SOURCE_DIR}/assets/texture-sheet.png")
    if(WIN32)
        add_compile_definitions(SO_NAME="${CMAKE_BINARY_DIR}/Debug/gamelib.dll")
    else()
        add_compile_definitions(SO_NAME="${CMAKE_BINARY_DIR}/libgamelib.so")
    endif()
endif()

target_link_libraries(gamelib PRIVATE raylib)
target_link_libraries(gamelib PRIVATE seblib)
target_link_libraries(gamelib PRIVATE seb-engine)

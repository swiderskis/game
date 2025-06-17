CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
project(game)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BIN_NAME game)
set(CMAKE_CXX_STANDARD 23)

add_executable(${BIN_NAME})
add_definitions(/DNDEBUG)
add_definitions(/DLOGLVL=2)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext-lib/raylib ${CMAKE_CURRENT_BINARY_DIR}/raylib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/seblib ${CMAKE_CURRENT_BINARY_DIR}/seblib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/seb-engine ${CMAKE_CURRENT_BINARY_DIR}/seb-engine)
target_link_libraries(${BIN_NAME} PRIVATE raylib)
target_link_libraries(${BIN_NAME} PRIVATE seblib)
target_link_libraries(${BIN_NAME} PRIVATE seb-engine)

file(GLOB SOURCES src/*.cpp)
target_sources(${BIN_NAME} PRIVATE ${SOURCES})
target_include_directories(${BIN_NAME} PRIVATE ext-lib/raylib-cpp/include)
target_include_directories(${BIN_NAME} PRIVATE ext-lib/raylib/src)
target_include_directories(${BIN_NAME} PRIVATE lib/seblib/src)
target_include_directories(${BIN_NAME} PRIVATE lib/seb-engine/src)

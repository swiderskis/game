# Game

A C++ game, powered by [raylib](https://www.raylib.com/) using [raylib-cpp](https://github.com/RobLoach/raylib-cpp) bindings.

## Getting started

This project relies only on [GNU Make](https://www.gnu.org/software/make/) to build.
This can be installed for Windows using [MSYS2](https://www.msys2.org/).
Once installed, simply type `make run` to build the raylib library and the project.

This project also contains a rudimentary [CMake](https://cmake.org/) file for quick building and running on Windows using MSVC, if you don't want to go through the hassle of setting up MSYS2.
Simply use the commands
```
cmake -B cmake-build
cmake --build cmake-build
.\cmake-build\Debug\game.exe
```
to build and run the project.
Note that it isn't my primary build tool and so will throw some warnings during the build.

## Credits

[raylib](https://github.com/raysan5/raylib)

[raylib-cpp](https://github.com/RobLoach/raylib-cpp)

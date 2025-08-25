# Game

A C++ game, powered by [raylib](https://www.raylib.com/) using [raylib-cpp](https://github.com/RobLoach/raylib-cpp) bindings.

## Getting started

This project can be built using [CMake](https://cmake.org/) with the commands
```
mkdir build && cd build
cmake ..
```

and run using `./game`.
A release build can be generated using
```
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
```

Note that attempting to run a release build will not work without copying `assets/` to the same directory as the binary.

## Credits

[raylib](https://github.com/raysan5/raylib)

[raylib-cpp](https://github.com/RobLoach/raylib-cpp)

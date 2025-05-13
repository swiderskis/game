#ifndef MISC_HPP_
#define MISC_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <variant> // IWYU pragma: keep

#if defined(_WIN32) || defined(__CYGWIN__)
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C" __attribute__((visibility("default")))
#endif

#endif

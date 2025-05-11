#ifndef MISC_HPP_
#define MISC_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <variant> // IWYU pragma: keep

#if defined(_WIN32) || defined(__CYGWIN__)
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C" __attribute__((visibility("default")))
#endif

// taken from https://en.cppreference.com/w/cpp/utility/variant/visit
template <typename... Ts>
struct overloaded : Ts... // NOLINT(readability-identifier-naming)
{
    using Ts::operator()...;
};

#define MATCH(val, ...) std::visit(overloaded{ __VA_ARGS__ }, val);

#endif

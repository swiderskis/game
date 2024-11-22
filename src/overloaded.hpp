#ifndef OVERLOADED_HPP_
#define OVERLOADED_HPP_

// Taken from https://en.cppreference.com/w/cpp/utility/variant/visit
template <typename... Ts>
struct overloaded : Ts... { // NOLINT(readability-identifier-naming)
    using Ts::operator()...;
};

#endif

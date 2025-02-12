#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include <format>   // IWYU pragma: keep
#include <iostream> // IWYU pragma: keep
#include <string>   // IWYU pragma: keep

#define LOGLVL 3
#define FILENAME_WIDTH 8

#if LOGLVL > 0
#define LOG_ERR(...)                                                                                                   \
    std::clog << "[ERR] " << std::string(__FILE__).substr(std::string(__FILE__).size() - FILENAME_WIDTH)               \
              << std::format(" {:>5}: ", __LINE__) << std::format(__VA_ARGS__) << "\n";
#else
#define LOG_ERR(...)
#endif

#if LOGLVL > 1
#define LOG_WRN(...)                                                                                                   \
    std::clog << "[WRN] " << std::string(__FILE__).substr(std::string(__FILE__).size() - FILENAME_WIDTH)               \
              << std::format(" {:>5}: ", __LINE__) << std::format(__VA_ARGS__) << "\n";
#else
#define LOG_WRN(...)
#endif

#if LOGLVL > 2
#define LOG_INF(...)                                                                                                   \
    std::clog << "[INF] " << std::string(__FILE__).substr(std::string(__FILE__).size() - FILENAME_WIDTH)               \
              << std::format(" {:>5}: ", __LINE__) << std::format(__VA_ARGS__) << "\n";
#else
#define LOG_INF(...)
#endif

#if LOGLVL > 3
#define LOG_TRC(...)                                                                                                   \
    std::clog << "[TRC] " << std::string(__FILE__).substr(std::string(__FILE__).size() - FILENAME_WIDTH)               \
              << std::format(" {:>5}: ", __LINE__) << std::format(__VA_ARGS__) << "\n";
#else
#define LOG_TRC(...)
#endif

#endif

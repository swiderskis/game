#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include <format>   // IWYU pragma: keep
#include <iostream> // IWYU pragma: keep
#include <string>   // IWYU pragma: keep

#define LOGLVL 3
#define FILENAME_WIDTH 8

#define LOG_ERR(...)                                                                                                   \
    if (LOGLVL > 0)                                                                                                    \
    {                                                                                                                  \
        std::clog << "[ERR] " << std::string(__FILE__).substr(std::string(__FILE__).size() - FILENAME_WIDTH)           \
                  << std::format(" {:>5}: ", __LINE__) << std::format(__VA_ARGS__) << "\n";                            \
    }

#define LOG_WRN(...)                                                                                                   \
    if (LOGLVL > 1)                                                                                                    \
    {                                                                                                                  \
        std::clog << "[WRN] " << std::string(__FILE__).substr(std::string(__FILE__).size() - FILENAME_WIDTH)           \
                  << std::format(" {:>5}: ", __LINE__) << std::format(__VA_ARGS__) << "\n";                            \
    }

#define LOG_INF(...)                                                                                                   \
    if (LOGLVL > 2)                                                                                                    \
    {                                                                                                                  \
        std::clog << "[INF] " << std::string(__FILE__).substr(std::string(__FILE__).size() - FILENAME_WIDTH)           \
                  << std::format(" {:>5}: ", __LINE__) << std::format(__VA_ARGS__) << "\n";                            \
    }

#define LOG_TRC(...)                                                                                                   \
    if (LOGLVL > 3)                                                                                                    \
    {                                                                                                                  \
        std::clog << "[TRC] " << std::string(__FILE__).substr(std::string(__FILE__).size() - FILENAME_WIDTH)           \
                  << std::format(" {:>5}: ", __LINE__) << std::format(__VA_ARGS__) << "\n";                            \
    }

#endif

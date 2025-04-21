#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include <cstdlib>  // IWYU pragma: keep
#include <format>   // IWYU pragma: keep
#include <iostream> // IWYU pragma: keep
#include <string>   // IWYU pragma: keep

#define LOGLVL 3
#define FILENAME_WIDTH 8

#define LOG(text_lvl, lvl, ...)                                                                                        \
    if (LOGLVL > lvl)                                                                                                  \
    {                                                                                                                  \
        const auto filename = std::string(__FILE__);                                                                   \
        std::clog << "[" text_lvl "] " << filename.substr(filename.size() - FILENAME_WIDTH)                            \
                  << std::format(" {:>5}: ", __LINE__) << std::format(__VA_ARGS__) << "\n";                            \
    }

#define LOG_FTL(...)                                                                                                   \
    LOG("FTL", -1, __VA_ARGS__)                                                                                        \
    quick_exit(EXIT_FAILURE);
#define LOG_ERR(...) LOG("ERR", 0, __VA_ARGS__)
#define LOG_WRN(...) LOG("WRN", 1, __VA_ARGS__)
#define LOG_INF(...) LOG("INF", 2, __VA_ARGS__)
#define LOG_TRC(...) LOG("TRC", 3, __VA_ARGS__)

#endif

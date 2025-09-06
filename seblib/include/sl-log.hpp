#ifndef SL_LOG_HPP_
#define SL_LOG_HPP_

#include <cstdint>
#include <filesystem>
#include <format>
#include <iostream>
#include <source_location>

namespace seblib::log
{
enum Level : int8_t
{
    FTL = -1,
    ERR = 0,
    WRN = 1,
    INF = 2,
    TRC = 3,
};

// taken from https://stackoverflow.com/questions/14805192/c-variadic-template-function-parameter-with-default-value
template <typename... Args>
struct log // NOLINT(readability-identifier-naming)
{
    explicit log(Level lvl,
                 std::format_string<Args...> fmt,
                 Args&&... args,
                 std::source_location loc = std::source_location::current());
};

template <typename... Args>
log(Level level, std::format_string<Args...> fmt, Args&&... args) -> log<Args...>;
auto level() -> int;
auto set_level(Level level) -> void;
} // namespace seblib::log

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seblib::log
{
namespace fs = std::filesystem;

inline constexpr unsigned FILENAME_WIDTH{ 16 };

template <typename... Args>
log<Args...>::log(const Level lvl,
                  const std::format_string<Args...> fmt,
                  Args&&... args,
                  const std::source_location loc)
{
    if (level() < lvl)
    {
        return;
    }

    std::string level_text;
    switch (lvl)
    {
    case FTL:
        level_text = "FTL";
        break;
    case ERR:
        level_text = "ERR";
        break;
    case WRN:
        level_text = "WRN";
        break;
    case INF:
        level_text = "INF";
        break;
    case TRC:
        level_text = "TRC";
        break;
    }

    const auto filename{ fs::path(loc.file_name()).filename().string() };
    std::clog << "[" << level_text << "] " << std::format("{:>{}}", filename, FILENAME_WIDTH)
              << std::format(" {:>5}: ", loc.line()) << std::format(fmt, std::forward<Args>(args)...) << "\n";

    if (lvl == FTL)
    {
        exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
    }
}
} // namespace seblib::log

#endif

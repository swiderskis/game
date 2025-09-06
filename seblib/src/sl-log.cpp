#include "sl-log.hpp"

#include <cstdlib>

#ifndef SLOG_LVL
#define SLOG_LVL 2
#endif

namespace
{
using namespace seblib::log;

// this will need to change if doing any multithreading
Level log_level{ static_cast<Level>(SLOG_LVL) }; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
} // namespace

namespace seblib::log
{
auto level() -> int
{
    return log_level;
}

auto set_level(const Level level) -> void
{
    log_level = level;
}
} // namespace seblib::log

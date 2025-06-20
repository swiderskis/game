#include "seblib.hpp"

#include <cstdlib>

#ifndef LOGLVL
#define LOGLVL 2
#endif

namespace
{
using namespace seblib::log;

// this will need to change if doing any multithreading
Level log_level = static_cast<Level>(LOGLVL); // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
} // namespace

namespace seblib::log
{
int level()
{
    return log_level;
}

void set_level(const Level level)
{
    log_level = level;
}
} // namespace seblib::log

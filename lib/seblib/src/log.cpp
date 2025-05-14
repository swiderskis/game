#include "seblib.hpp"

#include <cstdlib>

#ifndef LOGLVL
#define LOGLVL 3
#endif

namespace
{
// this will need to change if doing any multithreading
int log_level = LOGLVL; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
} // namespace

namespace seblib::log
{
int level()
{
    return log_level;
}

void set_level(const int level)
{
    log_level = level;
}
} // namespace seblib::log

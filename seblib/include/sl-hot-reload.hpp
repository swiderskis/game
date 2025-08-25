#ifndef SEBLIB_HOT_RELOAD_HPP_
#define SEBLIB_HOT_RELOAD_HPP_

#include "sl-log.hpp"

#if defined(_WIN32) || defined(__CYGWIN__)
#include <libloaderapi.h>
#include <minwindef.h>

#define SLHR_EXPORT extern "C" __declspec(dllexport)
#define SLHR_MODULE HMODULE
#define SLHR_PROC FARPROC
#define SLHR_SO_SUFFIX ".dll"
#else
#include <dlfcn.h>

#define SLHR_EXPORT extern "C" __attribute__((visibility("default")))
#define SLHR_MODULE void*
#define SLHR_PROC void*
#define SLHR_SO_SUFFIX ".so"
#endif

namespace seblib::hot_reload
{
SLHR_MODULE load_lib(const char* so_name, bool exit_on_fail);
template <typename F>
F get_func_address(SLHR_MODULE lib, const char* func_name, bool exit_on_fail);
void free_lib(SLHR_MODULE lib);
} // namespace seblib::hot_reload

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seblib::hot_reload
{
namespace slog = seblib::log;

template <typename F>
F get_func_address(SLHR_MODULE lib, const char* func_name, const bool exit_on_fail)
{
#if defined(_WIN32) || defined(__CYGWIN__)
    SLHR_PROC func_address = GetProcAddress(lib, func_name);
#else
    SLHR_PROC func_address = dlsym(lib, func_name);
#endif
    if (func_address == nullptr)
    {
        slog::log((exit_on_fail ? slog::FTL : slog::ERR), "Failed to get {} function address", func_name);
    }

    return reinterpret_cast<F>(func_address); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
}
} // namespace seblib::hot_reload

#endif

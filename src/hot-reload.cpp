#include "hot-reload.hpp" // IWYU pragma: keep

#include <cstdlib>
#include <string>

#ifndef NDEBUG
#include "seblib-log.hpp"

#include <filesystem>
#include <system_error>

#if defined(_WIN32) || defined(__CYGWIN__)
#include <libloaderapi.h>
#include <minwindef.h>

#define MODULE HMODULE
#define PROC FARPROC
#define SO_SUFFIX ".dll"
#else
#include <dlfcn.h>

#define MODULE void*
#define PROC void*
#define SO_SUFFIX ".so"
#endif

#ifndef SO_NAME
#define SO_NAME "build/debug/game" SO_SUFFIX
#endif
#define SO_TEMP_NAME SO_NAME ".tmp"

namespace fs = std::filesystem;
namespace slog = seblib::log;

namespace
{
MODULE lib = nullptr;       // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
unsigned rand_num = rand(); // NOLINT

MODULE load_lib(const char* so_name);
PROC get_func_address(MODULE lib, const char* func_name);
void free_lib(MODULE lib);
} // namespace

bool hot_reload::reload_lib(GameFuncs& game_funcs)
{
    if (lib != nullptr)
    {
        free_lib(lib);
    }

    std::string so_temp_name = SO_TEMP_NAME + std::to_string(rand_num);
    std::error_code ec;
    fs::remove(so_temp_name, ec);
    if (ec.value() != 0)
    {
        slog::log(slog::ERR, "Failed to delete shared library file");

        return false;
    }

    rand_num = rand(); // NOLINT
    fs::copy(SO_NAME, so_temp_name, ec);
    if (ec.value() != 0)
    {
        slog::log(slog::ERR, "Failed to copy shared library file");

        return false;
    }

    lib = load_lib(so_temp_name.c_str());
    if (lib == nullptr)
    {
        slog::log(slog::ERR, "Failed to load library");

        return false;
    }

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    game_funcs.run = (RunFunc)get_func_address(lib, "run");
    if (game_funcs.run == nullptr)
    {
        slog::log(slog::ERR, "Failed to get run function address");

        return false;
    }

    game_funcs.check_reload_lib = (CheckReloadLibFunc)get_func_address(lib, "check_reload_lib");
    if (game_funcs.check_reload_lib == nullptr)
    {
        slog::log(slog::ERR, "Failed to get check_reload_lib function address");

        return false;
    }
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
#ifdef __GNUC__
#pragma GCC diagnostic error "-Wcast-function-type"
#endif
    slog::log(slog::INF, "Loaded library successfully");

    return true;
}

namespace
{
MODULE load_lib(const char* so_name)
{
#if defined(_WIN32) || defined(__CYGWIN__)
    return LoadLibrary(so_name);
#else
    return dlopen(so_name, RTLD_LAZY);
#endif
}

PROC get_func_address(MODULE lib, const char* func_name)
{
#if defined(_WIN32) || defined(__CYGWIN__)
    return GetProcAddress(lib, func_name);
#else
    return dlsym(lib, func_name);
#endif
}

void free_lib(MODULE lib)
{
#if defined(_WIN32) || defined(__CYGWIN__)
    FreeLibrary(lib);
#else
    dlclose(lib);
#endif
}
} // namespace
#endif

#include "hot-reload.hpp" // IWYU pragma: keep

#ifndef NDEBUG
#include "logging.hpp"

#include <filesystem>
#include <system_error>

#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>

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
#define SO_NAME "make-build/debug/game" SO_SUFFIX
#endif
#define SO_TEMP_NAME SO_NAME ".tmp"

namespace fs = std::filesystem;

namespace
{
MODULE lib = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

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

    std::error_code ec;
    fs::remove(SO_TEMP_NAME);
    fs::copy(SO_NAME, SO_TEMP_NAME, ec);

    if (ec.value() != 0)
    {
        LOG_ERR("Failed to copy shared library file");

        return false;
    }

    lib = load_lib(SO_TEMP_NAME);
    if (lib == nullptr)
    {
        LOG_ERR("Failed to load library");

        return false;
    }

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    game_funcs.run = (RunFunc)get_func_address(lib, "run");
    if (game_funcs.run == nullptr)
    {
        LOG_ERR("Failed to get run function address");

        return false;
    }

    game_funcs.check_reload_lib = (CheckReloadLibFunc)get_func_address(lib, "check_reload_lib");
    if (game_funcs.check_reload_lib == nullptr)
    {
        LOG_ERR("Failed to get check_reload_lib function address");

        return false;
    }
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
#ifdef __GNUC__
#pragma GCC diagnostic error "-Wcast-function-type"
#endif
    LOG_INF("Loaded library successfully");

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

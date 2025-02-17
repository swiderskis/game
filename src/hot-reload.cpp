#include "hot-reload.hpp" // IWYU pragma: keep

#ifndef NDEBUG
#include "logging.hpp"

#include <filesystem>
#include <system_error>
#include <windows.h>

#ifndef SO_NAME
#define SO_NAME "build/debug/game.dll"
#endif
#define SO_TEMP_NAME SO_NAME ".tmp"

namespace fs = std::filesystem;

HMODULE lib = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

bool hot_reload::reload_lib(GameFuncs& game_funcs)
{
    if (lib != nullptr)
    {
        FreeLibrary(lib);
    }

    std::error_code ec;
    fs::remove(SO_TEMP_NAME);
    fs::copy(SO_NAME, SO_TEMP_NAME, ec);

    if (ec.value() != 0)
    {
        LOG_ERR("Failed to copy shared library file");

        return false;
    }

    lib = LoadLibrary(SO_TEMP_NAME);
    if (lib == nullptr)
    {
        LOG_ERR("Failed to load library");

        return false;
    }

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    game_funcs.run = (RunFunc)GetProcAddress(lib, "run");
    if (game_funcs.run == nullptr)
    {
        LOG_ERR("Failed to get run function address");

        return false;
    }

    game_funcs.check_reload_lib = (CheckReloadLibFunc)GetProcAddress(lib, "check_reload_lib");
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
#endif

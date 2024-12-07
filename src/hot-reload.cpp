#include "hot-reload.hpp"

#ifndef NDEBUG
#include <filesystem>
#include <iostream>
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
        std::cerr << "Failed to copy shared library file\n";

        return false;
    }

    lib = LoadLibrary(SO_TEMP_NAME);
    if (lib == nullptr)
    {
        std::cerr << "Failed to load library\n";

        return false;
    }

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    game_funcs.run = (RunFunc)GetProcAddress(lib, "run");
    if (game_funcs.run == nullptr)
    {
        std::cerr << "Failed to get run function address\n";

        return false;
    }

    game_funcs.check_reload_lib = (CheckReloadLibFunc)GetProcAddress(lib, "check_reload_lib");
    if (game_funcs.check_reload_lib == nullptr)
    {
        std::cerr << "Failed to get check_reload_lib function address\n";

        return false;
    }
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
#ifdef __GNUC__
#pragma GCC diagnostic error "-Wcast-function-type"
#endif

    std::cout << "Loaded library successfully\n";

    return true;
}
#endif

#ifndef HOT_RELOAD_HPP_
#define HOT_RELOAD_HPP_

// TODO make this cross-platform
#ifndef NDEBUG

#include "game.hpp"

#include <filesystem>
#include <iostream>
#include <libloaderapi.h>
#include <minwindef.h>
#include <system_error>

#ifndef SO_NAME
#define SO_NAME "build/debug/game.dll"
#endif
#define SO_TEMP_NAME SO_NAME ".tmp"

namespace fs = std::filesystem;

using RunFunc = void (*)(Game*);
using CheckReloadLibFunc = bool (*)();

struct GameFuncs
{
    HMODULE lib = nullptr;
    RunFunc run = nullptr;
    CheckReloadLibFunc check_reload_lib = nullptr;
};

namespace hot_reload
{
bool reload_lib(GameFuncs& game_funcs) // NOLINT(misc-definitions-in-headers)
{
    if (game_funcs.lib != nullptr)
    {
        FreeLibrary(game_funcs.lib);
    }

    std::error_code ec;
    fs::remove(SO_TEMP_NAME);
    fs::copy(SO_NAME, SO_TEMP_NAME, ec);

    if (ec.value() != 0)
    {
        std::cerr << "Failed to copy shared library file\n";

        return false;
    }

    game_funcs.lib = LoadLibrary(SO_TEMP_NAME);
    if (game_funcs.lib == nullptr)
    {
        std::cerr << "Failed to load library\n";

        return false;
    }

#pragma GCC diagnostic ignored "-Wcast-function-type"
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    game_funcs.run = (RunFunc)GetProcAddress(game_funcs.lib, "run");
    if (game_funcs.run == nullptr)
    {
        std::cerr << "Failed to get run function address\n";

        return false;
    }

    game_funcs.check_reload_lib = (CheckReloadLibFunc)GetProcAddress(game_funcs.lib, "check_reload_lib");
    if (game_funcs.check_reload_lib == nullptr)
    {
        std::cerr << "Failed to get check_reload_lib function address\n";

        return false;
    }
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
#pragma GCC diagnostic error "-Wcast-function-type"

    std::cout << "Loaded library successfully\n";

    return true;
}
} // namespace hot_reload

#endif

#endif

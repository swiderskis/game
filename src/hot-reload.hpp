#ifndef HOT_RELOAD_HPP_
#define HOT_RELOAD_HPP_

// TODO make this cross-platform
#ifndef NDEBUG
#pragma GCC diagnostic ignored "-Wcast-function-type"

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

using run_t = void (*)(Game*);
using check_reload_lib_t = bool (*)();

struct GameFuncs {
    HMODULE lib = nullptr;
    run_t run = nullptr;
    check_reload_lib_t check_reload_lib = nullptr;
};

namespace hot_reload
{
bool reload_lib(GameFuncs& game_funcs) // NOLINT(misc-definitions-in-headers)
{
    if (game_funcs.lib != nullptr) {
        FreeLibrary(game_funcs.lib);
    }

    std::error_code ec;
    fs::remove(SO_TEMP_NAME);
    fs::copy(SO_NAME, SO_TEMP_NAME, ec);

    if (ec.value() != 0) {
        std::cerr << "Failed to copy shared library file\n";

        return false;
    }

    game_funcs.lib = LoadLibrary(SO_TEMP_NAME);
    if (game_funcs.lib == nullptr) {
        std::cerr << "Failed to load library\n";

        return false;
    }

    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    game_funcs.run = (run_t)GetProcAddress(game_funcs.lib, "run");
    if (game_funcs.run == nullptr) {
        std::cerr << "Failed to get run function address\n";

        return false;
    }

    game_funcs.check_reload_lib = (check_reload_lib_t)GetProcAddress(game_funcs.lib, "check_reload_lib");
    if (game_funcs.check_reload_lib == nullptr) {
        std::cerr << "Failed to get check_reload_lib function address\n";

        return false;
    }
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)

    std::cout << "Loaded lib successfully\n";

    return true;
}
} // namespace hot_reload

#pragma GCC diagnostic error "-Wcast-function-type"
#endif

#endif

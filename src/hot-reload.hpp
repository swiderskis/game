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

using run_t = void (*)(Game*);
using window_should_close_t = bool (*)(Game*);
using check_reload_lib_t = bool (*)();

struct GameFuncs {
    HMODULE lib = nullptr;
    run_t run = nullptr;
    window_should_close_t window_should_close = nullptr;
    check_reload_lib_t check_reload_lib = nullptr;
};

namespace hot_reload
{
bool reload_lib(GameFuncs& game_funcs) // NOLINT
{
    if (game_funcs.lib != nullptr) {
        FreeLibrary(game_funcs.lib);
    }

    std::error_code ec;
    std::filesystem::remove("build/debug/hot-reload-temp.dll");
    std::filesystem::copy("build/debug/hot-reload.dll", "build/debug/hot-reload-temp.dll", ec);

    if (ec.value() != 0) {
        std::cerr << "Failed to copy shared library file\n";

        return false;
    }

    game_funcs.lib = LoadLibrary("hot-reload-temp");
    if (game_funcs.lib == nullptr) {
        std::cerr << "Failed to load library\n";

        return false;
    }

#pragma GCC diagnostic ignored "-Wcast-function-type"
    game_funcs.run = (run_t)GetProcAddress(game_funcs.lib, "run"); // NOLINT
    if (game_funcs.run == nullptr) {
        std::cerr << "Failed to get run function address\n";

        return false;
    }

#pragma GCC diagnostic ignored "-Wcast-function-type"
    game_funcs.window_should_close
        = (window_should_close_t)GetProcAddress(game_funcs.lib, "window_should_close"); // NOLINT
    if (game_funcs.window_should_close == nullptr) {
        std::cerr << "Failed to get window_should_close function address\n";

        return false;
    }

#pragma GCC diagnostic ignored "-Wcast-function-type"
    game_funcs.check_reload_lib = (check_reload_lib_t)GetProcAddress(game_funcs.lib, "check_reload_lib"); // NOLINT
    if (game_funcs.check_reload_lib == nullptr) {
        std::cerr << "Failed to get check_reload_lib function address\n";

        return false;
    }

    std::cout << "Loaded lib successfully\n";

    return true;
}
} // namespace hot_reload
#endif

#endif

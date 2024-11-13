#include "game.hpp"
#ifndef NDEBUG
#include "hot-reload.hpp"
#endif

int main()
{
    Game game;

#ifndef NDEBUG
    GameFuncs game_funcs;
    if (!hot_reload::reload_lib(game_funcs)) {
        return -1;
    }

    while (!game.window().ShouldClose()) {
        if (game_funcs.check_reload_lib() && !hot_reload::reload_lib(game_funcs)) {
            return -1;
        }

        game_funcs.run(&game);
    }
#else
    while (!game.window().ShouldClose()) {
        game.run();
    }
#endif

    return 0;
}

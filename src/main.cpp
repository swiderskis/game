#include "game.hpp"
#ifndef NDEBUG
#include "hot-reload.hpp"

int main()
{
    GameFuncs game_funcs;
    if (!hot_reload::reload_lib(game_funcs)) {
        return -1;
    }

    Game game;
    while (!game_funcs.window_should_close(&game)) {
        if (game_funcs.check_reload_lib() && !hot_reload::reload_lib(game_funcs)) {
            return -1;
        }

        game_funcs.run(&game);
    }

    return 0;
}
#else

int main()
{
    Game game;
    while (!game.window().ShouldClose()) {
        game.run();
    }

    return 0;
}
#endif

#include "game.hpp"

#ifndef NDEBUG
#include "hot-reload.hpp"

namespace hr = hot_reload;
#endif

int main()
{
    Game game;
#ifndef NDEBUG
    GameFuncs game_funcs = hr::reload_lib();
    while (!game.close && !game.window.ShouldClose())
    {
        if (game_funcs.check_reload_lib())
        {
            game_funcs = hr::reload_lib();
            game_funcs.reload_texture_sheet(&game);
        }

        game_funcs.run(&game);
    }
#else
    while (!game.close && !game.window.ShouldClose())
    {
        game.run();
    }
#endif

    return 0;
}

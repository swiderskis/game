#include "game.hpp"
#ifndef NDEBUG
#include "hot-reload.hpp"
#endif

int main()
{
    Game game;

#ifndef NDEBUG
    GameFuncs game_funcs;
    if (!hot_reload::reload_lib(game_funcs))
    {
        return -1;
    }

    while (!game.close)
    {
        if (game_funcs.check_reload_lib())
        {
            if (!hot_reload::reload_lib(game_funcs))
            {
                return -1;
            }

            game.reload_texture_sheet();
        }

        game_funcs.run(&game);
    }
#else
    while (!game.close)
    {
        game.run();
    }
#endif

    return 0;
}

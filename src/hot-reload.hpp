#ifndef HOT_RELOAD_HPP_
#define HOT_RELOAD_HPP_

// TODO make this cross-platform
#ifndef NDEBUG
class Game;

using RunFunc = void (*)(Game*);
using CheckReloadLibFunc = bool (*)();

struct GameFuncs
{
    RunFunc run = nullptr;
    CheckReloadLibFunc check_reload_lib = nullptr;
};

namespace hot_reload
{
bool reload_lib(GameFuncs& game_funcs);

} // namespace hot_reload
#endif

#endif

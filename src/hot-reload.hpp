#ifndef HOT_RELOAD_HPP_
#define HOT_RELOAD_HPP_

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
GameFuncs reload_lib();
} // namespace hot_reload
#endif

#endif

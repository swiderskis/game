#ifndef HOT_RELOAD_HPP_
#define HOT_RELOAD_HPP_

#ifndef NDEBUG
#if defined(_WIN32) || defined(__CYGWIN__)
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C" __attribute__((visibility("default")))
#endif

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

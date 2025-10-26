#include "sl-hot-reload.hpp"

#include "sl-log.hpp"

namespace seblib::hot_reload
{
auto load_lib(const char* so_name, const bool exit_on_fail) -> SLHR_MODULE
{
    slog::log(slog::INF, "Loading library {}", so_name);
#if defined(_WIN32) || defined(__CYGWIN__)
    SLHR_MODULE lib{ ::LoadLibrary(so_name) };
#else
    SLHR_MODULE lib{ ::dlopen(so_name, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND) };
#endif
    if (lib == nullptr)
    {
        slog::log((exit_on_fail ? slog::FTL : slog::ERR), "Failed to load library {}", so_name);
    }

    return lib;
}

void free_lib(SLHR_MODULE lib)
{
    if (lib != nullptr)
    {
#if defined(_WIN32) || defined(__CYGWIN__)
        ::FreeLibrary(lib);
#else
        ::dlclose(lib);
#endif
    }
}
} // namespace seblib::hot_reload

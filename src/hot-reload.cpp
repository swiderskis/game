#include "hot-reload.hpp" // IWYU pragma: keep

#include <cstdlib>
#include <string>

#ifndef NDEBUG
#include "seblib-hot-reload.hpp"
#include "seblib-log.hpp"

#include <filesystem>
#include <system_error>

#ifndef SO_NAME
#define SO_NAME "build/debug/game" SLHR_SO_SUFFIX
#endif
#define SO_TEMP_NAME SO_NAME ".tmp"

namespace fs = std::filesystem;
namespace slog = seblib::log;
namespace slhr = seblib::hot_reload;

namespace
{
SLHR_MODULE lib = nullptr;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
unsigned rand_num = rand(); // NOLINT(cert-msc30-c, cert-msc50-cpp, concurrency-mt-unsafe,
                            // cppcoreguidelines-avoid-non-const-global-variables)
} // namespace

GameFuncs hot_reload::reload_lib()
{
    slhr::free_lib(lib);
    std::string so_temp_name = SO_TEMP_NAME + std::to_string(rand_num);
    std::error_code ec;
    slog::log(slog::TRC, "Removing {}", so_temp_name);
    fs::remove(so_temp_name, ec);
    if (ec.value() != 0)
    {
        slog::log(slog::FTL, "Failed to delete shared library file");
    }

    rand_num = rand(); // NOLINT(cert-msc30-c, cert-msc50-cpp, concurrency-mt-unsafe)
    so_temp_name = SO_TEMP_NAME + std::to_string(rand_num);
    fs::copy(SO_NAME, so_temp_name, ec);
    if (ec.value() != 0)
    {
        slog::log(slog::FTL, "Failed to copy shared library file");
    }

    lib = slhr::load_lib(so_temp_name.c_str(), true);
    slog::log(slog::INF, "Loaded library successfully");

    return {
        .run = slhr::get_func_address<RunFunc>(lib, "run", true),
        .check_reload_lib = slhr::get_func_address<CheckReloadLibFunc>(lib, "check_reload_lib", true),
    };
}

#endif

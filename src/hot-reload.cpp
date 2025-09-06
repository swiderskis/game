#include "hot-reload.hpp" // IWYU pragma: keep

#include <cstdlib>
#include <string>

#ifndef NDEBUG
#include "sl-hot-reload.hpp"
#include "sl-log.hpp"

#include <filesystem>
#include <system_error>

#ifndef SO_NAME
#define SO_NAME "build/libgamelib" SLHR_SO_SUFFIX
#endif
#define SO_TEMP_NAME SO_NAME ".tmp"

namespace fs = std::filesystem;
namespace slog = seblib::log;
namespace slhr = seblib::hot_reload;

namespace
{
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
SLHR_MODULE lib{ nullptr };
// NOLINTBEGIN(cert-err58-cpp, cert-msc30-c, cert-msc50-cpp, concurrency-mt-unsafe)
std::string so_temp_name{ SO_TEMP_NAME + std::to_string(rand()) };
// NOLINTEND(cert-err58-cpp, cert-msc30-c, cert-msc50-cpp, concurrency-mt-unsafe)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
} // namespace

auto hot_reload::reload_lib() -> GameFuncs
{
    slog::log(slog::INF, "Library file name: {}", SO_NAME);
    slhr::free_lib(lib);
    std::error_code ec;
    slog::log(slog::INF, "Removing {}", so_temp_name);
    fs::remove(so_temp_name, ec);
    if (ec.value() != 0)
    {
        slog::log(slog::WRN, "Failed to delete shared library file");
    }

    so_temp_name = SO_TEMP_NAME + std::to_string(rand()); // NOLINT(cert-msc30-c, cert-msc50-cpp, concurrency-mt-unsafe)
    fs::copy(SO_NAME, so_temp_name, fs::copy_options::overwrite_existing, ec);
    if (ec.value() != 0)
    {
        slog::log(slog::FTL, "Failed to copy shared library file");
    }

    lib = slhr::load_lib(so_temp_name.c_str(), true);
    slog::log(slog::INF, "Loaded library successfully");

    return {
        .run = slhr::get_func_address<RunFunc>(lib, "run", true),
        .check_reload_lib = slhr::get_func_address<CheckReloadLibFunc>(lib, "check_reload_lib", true),
        .reload_texture_sheet = slhr::get_func_address<ReloadTextureSheetFunc>(lib, "reload_texture_sheet", true),
    };
}

#endif

#ifndef GAME_HPP_
#define GAME_HPP_

#include "entities.hpp"
#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "se-ecs.hpp"
#include "se-ui.hpp"
#include "seb-engine.hpp"
#include "seblib.hpp"
#include "settings.hpp"
#include "sprites.hpp"
#include "tiles.hpp"

#ifndef NDEBUG
#define SHOW_CBOXES
#undef SHOW_CBOXES
#define SHOW_HITBOXES
#undef SHOW_HITBOXES
#endif

static constexpr auto WINDOW_TITLE{ "Game Title" };
#ifndef TEXTURES
static constexpr auto TEXTURE_SHEET{ "assets/texture-sheet.png" };
#else
static constexpr auto TEXTURE_SHEET{ TEXTURES };
#endif

inline constexpr float CAMERA_ZOOM{ 2.0 };

inline constexpr size_t WORLD_WIDTH{ 32 };
inline constexpr size_t WORLD_HEIGHT{ 16 };

inline constexpr seblib::SimpleVec2 MELEE_OFFSET{ 32.0, 16.0 };
inline constexpr seblib::SimpleVec2 MELEE_OFFSET_FLIPPED{ -17.0, 16.0 };

struct Inputs
{
    bool left{ false };
    bool right{ false };
    bool up{ false };
    bool down{ false };
    bool click{ false };
    bool spawn_enemy{ false };
    bool pause{ false };
};

struct Game
{
    raylib::Window window{ seb_engine::ui::WINDOW_WIDTH, seb_engine::ui::WINDOW_HEIGHT, WINDOW_TITLE };
    raylib::Camera2D camera{ raylib::Vector2{ seb_engine::ui::WINDOW_WIDTH, seb_engine::ui::WINDOW_HEIGHT } / 2,
                             raylib::Vector2{},
                             0.0,
                             CAMERA_ZOOM };
    raylib::Texture texture_sheet{ TEXTURE_SHEET };
    seb_engine::Entities<MAX_ENTITIES, Entity> entities;
    seb_engine::Components<MAX_ENTITIES> components;
    Sprites sprites;
    seb_engine::World<Tile, SpriteTile, WORLD_WIDTH, WORLD_HEIGHT> world;
    std::vector<size_t> to_destroy;
    Inputs inputs;
    std::optional<seb_engine::ui::Screen> screen;
    size_t player_id{ 0 };
    bool paused{ false };
    bool close{ false };

    Game();

    auto run() -> void;
    auto spawn_player(seb_engine::Coords coords) -> void;
    auto spawn_enemy(Enemy enemy, seb_engine::Coords coords) -> void;
    auto spawn_tile(Tile tile, seb_engine::Coords coords) -> void;
    [[nodiscard]] auto dt() const -> float;
    [[nodiscard]] auto get_mouse_pos() const -> raylib::Vector2;
    auto destroy_entity(size_t id) -> void;
    auto spawn_attack(Attack attack, size_t parent_id) -> void;
    auto toggle_pause() -> void;

    // systems
    auto poll_inputs() -> void;
    auto render_sprites() -> void;
    auto set_player_vel() -> void;
    auto resolve_collisions() -> void;
    auto destroy_entities() -> void;
    auto player_attack() -> void;
    auto update_lifespans() -> void;
    auto damage_entities() -> void;
    auto sync_children() -> void;
    auto update_invuln_times() -> void;
    auto render_ui() -> void;
    auto check_pause_game() -> void;
    auto ui_click_action() -> void;
    auto set_flipped() -> void;
    auto render_damage_lines() -> void;

#ifdef SHOW_CBOXES
    auto render_cboxes() -> void;
#endif

#ifdef SHOW_HITBOXES
    auto render_hitboxes() -> void;
#endif
};

#endif

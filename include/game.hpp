#ifndef GAME_HPP_
#define GAME_HPP_

#include "entities.hpp"
#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "se-ecs.hpp"
#include "se-ui.hpp"
#include "seb-engine.hpp"
#include "seblib.hpp"
#include "sprites.hpp"

static constexpr auto WINDOW_TITLE = "Game Title";
#ifndef TEXTURES
static constexpr auto TEXTURE_SHEET = "assets/texture-sheet.png";
#else
static constexpr auto TEXTURE_SHEET = TEXTURES;
#endif

inline constexpr float CAMERA_ZOOM = 2.0;

inline constexpr unsigned WORLD_WIDTH = 32;
inline constexpr unsigned WORLD_HEIGHT = 16;

inline constexpr seblib::SimpleVec2 MELEE_OFFSET{ 32.0, 16.0 };
inline constexpr seblib::SimpleVec2 MELEE_OFFSET_FLIPPED{ -17.0, 16.0 };

struct Inputs
{
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool click = false;
    bool spawn_enemy = false;
    bool pause = false;
};

struct Game
{
    raylib::Window window{ seb_engine::ui::WINDOW_WIDTH, seb_engine::ui::WINDOW_HEIGHT, WINDOW_TITLE };
    raylib::Camera2D camera{ raylib::Vector2(seb_engine::ui::WINDOW_WIDTH, seb_engine::ui::WINDOW_HEIGHT) / 2,
                             raylib::Vector2(0.0, 0.0),
                             0.0,
                             CAMERA_ZOOM };
    raylib::Texture texture_sheet{ TEXTURE_SHEET };
    seb_engine::Entities<Entity> entities;
    seb_engine::Components components;
    Sprites sprites;
    seb_engine::World<Tile, SpriteTile, WORLD_WIDTH, WORLD_HEIGHT> world;
    Inputs inputs;
    std::optional<seb_engine::ui::Screen> screen;
    unsigned player_id = 0;
    bool paused = false;
    bool close = false;

    Game();

    void run();
    void spawn_player(seb_engine::Coords coords);
    void spawn_enemy(Enemy enemy, seb_engine::Coords coords);
    void spawn_tile(Tile tile, seb_engine::Coords coords);
    [[nodiscard]] float dt() const;
    [[nodiscard]] raylib::Vector2 get_mouse_pos() const;
    void destroy_entity(unsigned id);
    void spawn_attack(Attack attack, unsigned parent_id);
    void toggle_pause();

    // systems
    void poll_inputs();
    void render();
    void set_player_vel();
    void resolve_collisions();
    void destroy_entities();
    void player_attack();
    void update_lifespans();
    void damage_entities();
    void sync_children();
    void update_invuln_times();
    void render_ui();
    void check_pause_game();
    void ui_click_action();
    void set_flipped();
#ifndef NDEBUG
    void reload_texture_sheet();
#endif
};

#endif

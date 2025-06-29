#ifndef GAME_HPP_
#define GAME_HPP_

#include "components.hpp"
#include "entities.hpp"
#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "seb-engine-ecs.hpp"
#include "seb-engine-ui.hpp"
#include "seblib.hpp"
#include "sprites.hpp"

static constexpr auto WINDOW_TITLE = "Game Title";
static constexpr auto TEXTURE_SHEET = "assets/texture-sheet.png";

inline constexpr float CAMERA_ZOOM = 2.0;

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

struct Coordinates
{
    seblib::SimpleVec2 pos;

    Coordinates() = delete;

    constexpr Coordinates(int x, int y) : pos(static_cast<float>(x) * TILE_SIZE, static_cast<float>(-y) * TILE_SIZE)
    {
    }

    operator raylib::Vector2() const // NOLINT(hicpp-explicit-conversions)
    {
        return pos;
    }
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
    seb_engine::Tiles<Tile, SpriteTile> tiles;
    Inputs inputs;
    std::optional<seb_engine::ui::Screen> screen;
    unsigned player_id = 0;
    bool paused = false;
    bool close = false;

    Game();

    void run();
    void spawn_player(raylib::Vector2 pos);
    void spawn_enemy(Enemy enemy, raylib::Vector2 pos);
    void spawn_tile(Tile tile, raylib::Vector2 pos);
    [[nodiscard]] float dt() const;
    [[nodiscard]] raylib::Vector2 get_mouse_pos() const;
    void destroy_entity(unsigned id);
    void spawn_attack(Attack attack, unsigned parent_id);
    void toggle_pause();

    // systems
    void poll_inputs();
    void render();
    void set_player_vel();
    void move_entities();
    void destroy_entities();
    void player_attack();
    void update_lifespans();
    void damage_entities();
    void sync_children();
    void update_invuln_times();
    void render_ui();
    void check_pause_game();
    void ui_click_action();
#ifndef NDEBUG
    void reload_texture_sheet();
#endif
};

#endif

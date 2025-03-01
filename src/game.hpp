#ifndef GAME_HPP_
#define GAME_HPP_

#include "components.hpp"
#include "entities.hpp"
#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "utils.hpp"

static constexpr auto WINDOW_TITLE = "Game Title";
static constexpr auto TEXTURE_SHEET = "assets/texture-sheet.png";

inline constexpr unsigned WINDOW_WIDTH = 800;
inline constexpr unsigned WINDOW_HEIGHT = 450;

inline constexpr float CAMERA_ZOOM = 2.0;

struct Inputs
{
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool attack = false;
    bool spawn_enemy = false;
};

struct Coordinates
{
    SimpleVec2 pos;

    Coordinates() = delete;

    constexpr Coordinates(int x, int y) : pos((float)x * TILE_SIZE, (float)-y * TILE_SIZE)
    {
    }

    operator RVector2() const // NOLINT(hicpp-explicit-conversions)
    {
        return pos;
    }
};

class Game
{
    Entities m_entities;
    Components m_components;
    Inputs m_inputs;
    RWindow m_window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
    RTexture m_texture_sheet{ TEXTURE_SHEET };
    RCamera2D m_camera{ RVector2(WINDOW_WIDTH, WINDOW_HEIGHT) / 2, RVector2(0.0, 0.0), 0.0, CAMERA_ZOOM };

    void spawn_player(RVector2 pos);
    void spawn_enemy(Enemy enemy, RVector2 pos);
    void spawn_tile(Tile tile, RVector2 pos);
    [[nodiscard]] float dt() const;
    [[nodiscard]] RVector2 get_mouse_pos() const;
    void destroy_entity(unsigned id);
    void spawn_attack(Attack attack, unsigned parent_id);

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

public:
    Game();

    void run();
    RWindow& window();
    [[nodiscard]] Entities& entities();
    [[nodiscard]] Components& components();
#ifndef NDEBUG
    void reload_texture_sheet();
#endif
};

#endif

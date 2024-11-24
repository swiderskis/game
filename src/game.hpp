#ifndef GAME_HPP_
#define GAME_HPP_

#include "components.hpp"
#include "entities.hpp"
#include "raylib-cpp.hpp" // IWYU pragma: keep

constexpr auto WINDOW_TITLE = "Game Title";
constexpr auto TEXTURE_SHEET = "assets/texture-sheet.png";

constexpr unsigned WINDOW_WIDTH = 800;
constexpr unsigned WINDOW_HEIGHT = 450;
constexpr unsigned WINDOW_HALF_WIDTH = WINDOW_WIDTH / 2;
constexpr unsigned WINDOW_HALF_HEIGHT = WINDOW_HEIGHT / 2;

constexpr float CAMERA_ZOOM = 2.0;

struct Inputs {
    bool left = false;
    bool right = false;
    bool up = false;
    bool attack = false;
    bool spawn_enemy = false;

private:
    Inputs() = default;

    friend class Game;
};

struct Coordinates {
    RVector2 m_pos;

    Coordinates(int x, int y);

    operator RVector2() const; // NOLINT(hicpp-explicit-conversions)
};

class Game
{
    EntityManager m_entity_manager;
    ComponentManager m_component_manager;
    Inputs m_inputs;
    RWindow m_window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
    RTexture m_texture_sheet{ TEXTURE_SHEET };
    RCamera2D m_camera{ RVector2(WINDOW_HALF_WIDTH, WINDOW_HALF_HEIGHT), RVector2(0.0, 0.0), 0.0, CAMERA_ZOOM };

    // Systems
    void poll_inputs();
    void render_sprites();
    void set_player_vel();
    void move_entities();
    void destroy_entities();
    void player_attack();
    void update_lifespans();
    void check_projectiles_hit();

    void spawn_player();
    void spawn_tile(Tile tile, RVector2 pos);
    [[nodiscard]] float dt() const;
    void resolve_tile_collisions(unsigned id, Entity entity, BBox prev_bbox);
    void spawn_projectile(RVector2 pos);
    [[nodiscard]] RVector2 get_mouse_pos() const;
    void spawn_enemy(RVector2 pos);
    void render_health_bars(unsigned id);

public:
    Game();

    void run();
    RWindow& window();
};

#endif

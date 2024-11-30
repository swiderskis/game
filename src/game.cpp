#include "game.hpp"

#include "components.hpp"
#include "entities.hpp"

#include <cassert>
#include <cmath>
#include <optional>

inline constexpr unsigned TARGET_FPS = 60;

inline constexpr float PLAYER_BBOX_SIZE_X = 20.0;
inline constexpr float PLAYER_BBOX_SIZE_Y = 29.0;
inline constexpr float PROJECTILE_SPEED = 500.0;
inline constexpr float PROJECTILE_LIFESPAN = 0.3;
inline constexpr float ENEMY_BBOX_SIZE_X = 30.0;
inline constexpr float ENEMY_BBOX_SIZE_Y = 24.0;

inline constexpr int PLAYER_HEALTH = 100;
inline constexpr int ENEMY_HEALTH = 100;

Coordinates::Coordinates(const int x, const int y) : m_pos(RVector2((float)x, (float)-y) * TILE_SIZE)
{
}

Coordinates::operator RVector2() const
{
    return m_pos;
}

void Game::spawn_player()
{
    unsigned id = m_entities.spawn(Entity::Player);
    auto& transform = m_components.transforms[id];
    transform.pos = Coordinates(0, 2);
    m_components.bounding_boxes[id].set(transform, RVector2(PLAYER_BBOX_SIZE_X, PLAYER_BBOX_SIZE_Y));
    m_components.health[id].set_health(PLAYER_HEALTH);
}

void Game::spawn_tile(const Tile tile, const RVector2 pos)
{
    const unsigned id = m_entities.spawn(Entity::Tile);
    auto& transform = m_components.transforms[id];
    transform.pos = pos;
    m_components.bounding_boxes[id].sync(transform);

    switch (tile)
    {
    case Tile::Brick:
        m_components.sprites[id].base.set(SpriteBase::TileBrick);
        break;
    }
}

float Game::dt() const
{
    return m_window.GetFrameTime();
}

void Game::spawn_projectile(const RVector2 pos)
{
    const unsigned id = m_entities.spawn(Entity::Projectile);
    auto& transform = m_components.transforms[id];
    transform.pos = pos;
    const auto diff = get_mouse_pos() - transform.pos;
    const float angle = atan2(diff.y, diff.x);
    transform.vel = RVector2(cos(angle), sin(angle)) * PROJECTILE_SPEED;

    m_components.sprites[id].base.set(SpriteBase::Projectile);
    m_components.bounding_boxes[id].set(transform, 4); // NOLINT
    m_components.lifespans[id].current = PROJECTILE_LIFESPAN;
}

RVector2 Game::get_mouse_pos() const
{
    return m_camera.GetScreenToWorld(RMouse::GetPosition()) - RVector2(TILE_SIZE, TILE_SIZE) / 2;
}

void Game::spawn_enemy(const RVector2 pos)
{
    const unsigned id = m_entities.spawn(Entity::Enemy);
    auto& transform = m_components.transforms[id];
    transform.pos = pos;
    m_components.sprites[id].base.set(SpriteBase::Enemy);
    m_components.bounding_boxes[id].set(transform, RVector2(ENEMY_BBOX_SIZE_X, ENEMY_BBOX_SIZE_Y));
    m_components.health[id].set_health(ENEMY_HEALTH);
}

Game::Game()
{
    m_window.SetTargetFPS(TARGET_FPS);
    m_window.SetExitKey(KEY_NULL);

    for (int i = 0; i < 10; i++) // NOLINT
    {
        spawn_tile(Tile::Brick, Coordinates(-3, i)); // NOLINT
    }

    for (int i = -10; i < 10; i++) // NOLINT
    {
        spawn_tile(Tile::Brick, Coordinates(i, 0)); // NOLINT
    }

    spawn_tile(Tile::Brick, Coordinates(-2, 7)); // NOLINT
    spawn_tile(Tile::Brick, Coordinates(1, 6));  // NOLINT
    spawn_tile(Tile::Brick, Coordinates(-1, 3)); // NOLINT

    spawn_player();
    spawn_enemy(Coordinates(2, 2));
}

void Game::run()
{
    m_window.BeginDrawing();
    m_window.ClearBackground(SKYBLUE);

    poll_inputs();
    set_player_vel();
    player_attack();
    move_entities();
    update_lifespans();
    check_projectiles_hit();
    destroy_entities();
    render();

    if (m_inputs.spawn_enemy)
    {
        spawn_enemy(Coordinates(2, 2));
    }

    m_window.EndDrawing();
}

RWindow& Game::window()
{
    return m_window;
}

#ifndef NDEBUG
#include <iostream>

void Game::reload_texture_sheet()
{
    m_texture_sheet.Unload();
    m_texture_sheet.Load(TEXTURE_SHEET);
    std::cout << "Texture sheet reloaded\n";
}

extern "C" __declspec(dllexport) void run(Game& game)
{
    game.run();
}

extern "C" __declspec(dllexport) bool check_reload_lib()
{
    return RKeyboard::IsKeyPressed(KEY_R);
}
#endif

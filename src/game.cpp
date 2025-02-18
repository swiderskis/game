#include "game.hpp"

#include "components.hpp"
#include "entities.hpp"
#include "settings.hpp"

#include <cassert>
#include <cmath>
#include <optional>
#include <ranges>

inline constexpr unsigned TARGET_FPS = 60;

void Game::spawn_player(const RVector2 pos)
{
    const unsigned id = m_entities.spawn(Entity::Player);
    m_components.init_player(id, pos);
}

void Game::spawn_enemy(const Enemy enemy, const RVector2 pos)
{
    const unsigned id = m_entities.spawn(Entity::Enemy);
    m_components.init_enemy(id, pos, enemy);
}

void Game::spawn_tile(const Tile tile, const RVector2 pos)
{
    const auto id = m_entities.spawn(Entity::Tile);
    m_components.init_tile(id, pos, tile);
}

float Game::dt() const
{
    return m_window.GetFrameTime();
}

RVector2 Game::get_mouse_pos() const
{
    return m_camera.GetScreenToWorld(RMouse::GetPosition()) - RVector2(SPRITE_SIZE, SPRITE_SIZE) / 2;
}

void Game::destroy_entity(const unsigned id)
{
    // destroy any child entities
    for (const auto [child_id, parent_id] : m_components.parents | std::views::enumerate | std::views::as_const)
    {
        if (parent_id != std::nullopt && parent_id.value() == id)
        {
            destroy_entity(child_id);
        }
    }

    m_components.uninit_destroyed_entity(id);
    m_entities.destroy_entity(id);
}

void Game::spawn_attack(const Attack attack, const unsigned parent_id)
{
    const auto details = entities::attack_details(attack);
    const auto source_pos = m_components.transforms[parent_id].pos;
    unsigned id = 0;
    switch (attack)
    {
    case Attack::Melee:
        id = m_entities.spawn(Entity::Melee);
        m_components.init_melee(id, source_pos, parent_id, details);
        break;
    case Attack::Projectile:
        const auto target_pos = (parent_id == PLAYER_ID ? get_mouse_pos() : m_components.transforms[PLAYER_ID].pos);
        id = m_entities.spawn(Entity::Projectile);
        m_components.init_projectile(id, source_pos, target_pos, details);
        break;
    }
}

Game::Game()
{
    m_window.SetTargetFPS(TARGET_FPS);
    m_window.SetExitKey(KEY_NULL);

    for (int i = 0; i < 10; i++) // NOLINT
    {
        spawn_tile(Tile::Brick, Coordinates(-3, i));
    }

    for (int i = -10; i < 10; i++) // NOLINT
    {
        spawn_tile(Tile::Brick, Coordinates(i, 0));
    }

    spawn_tile(Tile::Brick, Coordinates(-2, 7)); // NOLINT
    spawn_tile(Tile::Brick, Coordinates(1, 6));  // NOLINT
    spawn_tile(Tile::Brick, Coordinates(-1, 3));

    spawn_player(Coordinates(0, 2));
    spawn_enemy(Enemy::Duck, Coordinates(2, 2));
}

void Game::run()
{
    m_window.BeginDrawing();
    m_window.ClearBackground(SKYBLUE);

    poll_inputs();
    set_player_vel();
    player_attack();
    move_entities();
    sync_children();
    update_lifespans();
    update_invuln_times();
    damage_entities();
    destroy_entities();
    render();

    if (m_inputs.spawn_enemy)
    {
        spawn_enemy(Enemy::Duck, Coordinates(2, 2));
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

#include "game.hpp"

#include "components.hpp"
#include "entities.hpp"
#include "logging.hpp"
#include "settings.hpp"
#include "utils.hpp"

#include <cassert>
#include <cmath>
#include <optional>
#include <ranges>

inline constexpr unsigned TARGET_FPS = 60;

inline constexpr float DAMAGE_LINES_INV_FREQ = 5.0;

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
    const auto source_pos = m_components.transforms[parent_id].pos;
    LOG_TRC("Attack source pos ({}, {})", source_pos.x, source_pos.y);
    const auto target_pos = (parent_id == PLAYER_ID ? get_mouse_pos() : m_components.transforms[PLAYER_ID].pos);
    LOG_TRC("Attack target pos ({}, {})", target_pos.x, target_pos.y);
    switch (attack)
    {
    case Attack::Melee:
    {
        const unsigned id = m_entities.spawn(Entity::Melee);
        m_components.init_melee(id, source_pos, parent_id, attack);
        break;
    }
    case Attack::Projectile:
    {
        const unsigned id = m_entities.spawn(Entity::Projectile);
        m_components.init_projectile(id, source_pos, target_pos, attack);
        break;
    }
    case Attack::Sector:
    {
        const unsigned sector_id = m_entities.spawn(Entity::Sector);
        const auto diff = target_pos - source_pos;
        const auto sector_details = std::get<SectorDetails>(entities::attack_details(attack).details);
        const float angle = atan2(diff.y, diff.x);
        const float initial_angle = angle - (sector_details.ang / 2);
        m_components.init_sector(sector_id, parent_id, attack);
        const unsigned damage_lines
            = 1 + (unsigned)ceil(sector_details.radius * sector_details.ang / DAMAGE_LINES_INV_FREQ);
        LOG_TRC("Spawning {} damage lines", damage_lines);
        const float angle_diff = sector_details.ang / (float)(damage_lines - 1.0);
        LOG_TRC("Angle between damage lines: {}", utils::radians_to_degrees(angle_diff));
        const auto external_offset = RVector2(cos(angle), sin(angle)) * sector_details.external_offset;
        for (unsigned i = 0; i < damage_lines; i++)
        {
            const unsigned line_id = m_entities.spawn(Entity::DamageLine);
            const auto line_angle = initial_angle + (angle_diff * (float)i);
            m_components.init_damage_line(line_id, source_pos, line_angle, external_offset, attack, sector_id);
        }

        break;
    }
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

Entities& Game::entities()
{
    return m_entities;
}

Components& Game::components()
{
    return m_components;
}

#ifndef NDEBUG
#include "logging.hpp"

void Game::reload_texture_sheet()
{
    m_texture_sheet.Unload();
    m_texture_sheet.Load(TEXTURE_SHEET);
    LOG_INF("Texture sheet reloaded");
}

EXPORT void run(Game& game)
{
    game.run();
}

EXPORT bool check_reload_lib()
{
    return RKeyboard::IsKeyPressed(KEY_R);
}
#endif

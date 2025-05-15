#include "game.hpp"

#include "components.hpp"
#include "entities.hpp"
#include "misc.hpp"
#include "seblib.hpp"
#include "settings.hpp"

#include <cassert>
#include <cmath>
#include <optional>
#include <ranges>

namespace sl = seblib;
namespace sui = seblib::ui;
namespace slog = seblib::log;

inline constexpr unsigned TARGET_FPS = 60;

inline constexpr float DAMAGE_LINES_INV_FREQ = 5.0;
inline constexpr float PROJECTILE_SIZE = 4.0;

inline constexpr auto PLAYER_CBOX_SIZE = sl::SimpleVec2(20.0, 29.0);
inline constexpr auto ENEMY_CBOX_SIZE = sl::SimpleVec2(30.0, 24.0);
inline constexpr auto PLAYER_HITBOX_SIZE = sl::SimpleVec2(12.0, 21.0);
inline constexpr auto PLAYER_HITBOX_OFFSET = sl::SimpleVec2(0.0, 4.0);
inline constexpr auto ENEMY_HITBOX_SIZE = sl::SimpleVec2(22.0, 16.0);
inline constexpr auto ENEMY_HITBOX_OFFSET = sl::SimpleVec2(0.0, 4.0);
inline constexpr auto TILE_CBOX_SIZE = sl::SimpleVec2(TILE_SIZE, TILE_SIZE);
inline constexpr auto TILE_CBOX_OFFSET = sl::SimpleVec2(-8.0, 16.0);
inline constexpr auto MELEE_OFFSET = sl::SimpleVec2(24.0, 9.0);

inline constexpr int PLAYER_HEALTH = 100;
inline constexpr int ENEMY_HEALTH = 100;

namespace
{
sui::Screen pause_screen(Game& game);
} // namespace

void Game::spawn_player(const RVector2 pos)
{
    const unsigned id = m_entities.spawn(Entity::Player);
    m_components.get_by_id(id)
        .set_pos(pos)
        .set_cbox_size(PLAYER_CBOX_SIZE)
        .set_health(PLAYER_HEALTH)
        .set_hitbox_size(PLAYER_HITBOX_SIZE)
        .set_hitbox_offset(PLAYER_HITBOX_OFFSET);
}

void Game::spawn_enemy(const Enemy enemy, const RVector2 pos)
{
    auto sprite = SpriteBase::None;
    switch (enemy)
    {
    case Enemy::Duck:
        sprite = SpriteBase::EnemyDuck;
        break;
    }

    const unsigned id = m_entities.spawn(Entity::Enemy);
    m_components.get_by_id(id)
        .set_pos(pos)
        .set_cbox_size(ENEMY_CBOX_SIZE)
        .set_health(ENEMY_HEALTH)
        .set_hitbox_size(ENEMY_HITBOX_SIZE)
        .set_hitbox_offset(ENEMY_HITBOX_OFFSET)
        .set_sprite_base(sprite);
}

void Game::spawn_tile(const Tile tile, const RVector2 pos)
{
    auto sprite = SpriteBase::None;
    switch (tile)
    {
    case Tile::Brick:
        sprite = SpriteBase::TileBrick;
        break;
    }

    const auto id = m_entities.spawn(Entity::Tile);
    m_components.get_by_id(id)
        .set_pos(pos)
        .set_cbox_offset(TILE_CBOX_OFFSET)
        .set_cbox_size(TILE_CBOX_SIZE)
        .set_sprite_base(sprite);
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
    if (m_entities.entities()[id] == std::nullopt)
    {
        return;
    }

    m_entities.destroy_entity(id);
    m_components.uninit_destroyed_entity(id);
    // destroy any child entities
    for (const auto [child_id, parent_id] : m_components.parents | std::views::enumerate | std::views::as_const)
    {
        if (parent_id != std::nullopt && parent_id.value() == id)
        {
            destroy_entity(child_id);
        }
    }
}

void Game::spawn_attack(const Attack attack, const unsigned parent_id)
{
    const auto details = entities::attack_details(attack);
    const auto source_pos = m_components.transforms[parent_id].pos;
    slog::log(slog::TRC, "Attack source pos ({}, {})", source_pos.x, source_pos.y);
    const auto target_pos = (parent_id == PLAYER_ID ? get_mouse_pos() : m_components.transforms[PLAYER_ID].pos);
    slog::log(slog::TRC, "Attack target pos ({}, {})", target_pos.x, target_pos.y);
    const auto diff = target_pos - source_pos;
    const float angle = atan2(diff.y, diff.x);
    switch (attack)
    {
    case Attack::Melee:
    {
        const auto melee_details = std::get<MeleeDetails>(details.details);
        const unsigned id = m_entities.spawn(Entity::Melee);
        m_components.get_by_id(id)
            .set_pos(source_pos)
            .set_lifespan(details.lifespan)
            .set_hitbox_size(melee_details.size)
            .set_hitbox_offset(MELEE_OFFSET)
            .set_parent(parent_id)
            .set_hit_damage(details.damage);
        break;
    }
    case Attack::Projectile:
    {
        const auto proj_details = std::get<ProjectileDetails>(details.details);
        const auto vel = RVector2(cos(angle), sin(angle)) * proj_details.speed;
        const unsigned id = m_entities.spawn(Entity::Projectile);
        m_components.get_by_id(id)
            .set_pos(source_pos)
            .set_vel(vel)
            .set_sprite_base(SpriteBase::Projectile)
            .set_cbox_size(PROJECTILE_SIZE)
            .set_lifespan(details.lifespan)
            .set_hitbox_size(PROJECTILE_SIZE)
            .set_hit_damage(details.damage);
        break;
    }
    case Attack::Sector:
    {
        const auto sector_details = std::get<SectorDetails>(details.details);
        const float initial_angle = angle - (sector_details.ang / 2);
        const unsigned damage_lines
            = 1 + (unsigned)ceil(sector_details.radius * sector_details.ang / DAMAGE_LINES_INV_FREQ);
        slog::log(slog::TRC, "Spawning {} damage lines", damage_lines);
        const float angle_diff = sector_details.ang / (float)(damage_lines - 1.0);
        slog::log(slog::TRC, "Angle between damage lines: {}", sl::math::radians_to_degrees(angle_diff));
        const auto ext_offset = RVector2(cos(angle), sin(angle)) * sector_details.external_offset;
        const unsigned sector_id = m_entities.spawn(Entity::Sector);
        m_components.get_by_id(sector_id).set_lifespan(details.lifespan).set_parent(parent_id);
        for (unsigned i = 0; i < damage_lines; i++)
        {
            const unsigned line_id = m_entities.spawn(Entity::DamageLine);
            const auto line_ang = initial_angle + (angle_diff * (float)i);
            const auto offset = ext_offset + RVector2(cos(line_ang), sin(line_ang)) * sector_details.internal_offset;
            slog::log(slog::TRC, "Offsetting damage line by ({}, {})", offset.x, offset.y);
            m_components.get_by_id(line_id)
                .set_pos(source_pos)
                .set_hitbox_size(sector_details.radius, line_ang)
                .set_hitbox_offset(offset)
                .set_parent(sector_id)
                .set_hit_damage(details.damage);
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
    poll_inputs();
    check_pause_game();
    if (!m_paused)
    {
        set_player_vel();
        player_attack();
        move_entities();
        sync_children();
        update_lifespans();
        update_invuln_times();
        damage_entities();
        destroy_entities();
    }

    m_window.BeginDrawing();
    m_window.ClearBackground(::SKYBLUE);
    render();
    render_ui();
    m_window.EndDrawing();

    ui_click_action();

    if (m_inputs.spawn_enemy)
    {
        spawn_enemy(Enemy::Duck, Coordinates(2, 2));
    }
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

void Game::toggle_pause()
{
    m_paused = !m_paused;
    if (m_paused)
    {
        m_screen = pause_screen(*this);
        slog::log(slog::INF, "Game paused");
    }
    else
    {
        m_screen = std::nullopt;
        slog::log(slog::INF, "Game unpaused");
    }
}

#ifndef NDEBUG
void Game::reload_texture_sheet()
{
    m_texture_sheet.Unload();
    m_texture_sheet.Load(TEXTURE_SHEET);
    slog::log(slog::INF, "Texture sheet reloaded");
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

namespace
{

sui::Screen pause_screen(Game& game)
{
    sui::Screen screen;
    auto [_, resume_button] = screen.new_element<sui::Button>();
    resume_button->set_pos(sui::PercentSize{ .width = 50, .height = 40 });  // NOLINT(*magic-numbers)
    resume_button->set_size(sui::PercentSize{ .width = 20, .height = 10 }); // NOLINT(*magic-numbers)
    resume_button->color = ::WHITE;
    resume_button->text.text = "Resume";
    resume_button->text.set_percent_size(6); // NOLINT(*magic-numbers)
    resume_button->on_click = [&game]() { game.toggle_pause(); };

    auto [_, exit_button] = screen.new_element<sui::Button>();
    exit_button->set_pos(sui::PercentSize{ .width = 50, .height = 60 });  // NOLINT(*magic-numbers)
    exit_button->set_size(sui::PercentSize{ .width = 20, .height = 10 }); // NOLINT(*magic-numbers)
    exit_button->color = ::WHITE;
    exit_button->text.text = "Exit";
    exit_button->text.set_percent_size(6); // NOLINT(*magic-numbers)
    exit_button->on_click = [&game]() { game.window().Close(); };

    return screen;
}
} // namespace

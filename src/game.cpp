#include "game.hpp"

#include "components.hpp"
#include "entities.hpp"
#include "se-bbox.hpp"
#include "sl-extern.hpp"
#include "sl-log.hpp"
#include "sl-math.hpp"
#include "sprites.hpp"

#include <cassert>
#include <cmath>
#include <optional>
#include <ranges>

namespace rl = raylib;
namespace sl = seblib;
namespace slog = seblib::log;
namespace sm = seblib::math;
namespace se = seb_engine;
namespace sui = seb_engine::ui;

inline constexpr unsigned TARGET_FPS{ 60 };

inline constexpr float LINE_ANGLE_SPACING{ 5.0 };
inline constexpr float PROJECTILE_RADIUS{ 4.0 };

inline constexpr sm::Vec2 PLAYER_CBOX_OFFSET{ 6.0, 3.0 };
inline constexpr sm::Vec2 ENEMY_CBOX_OFFSET{ 1.0, 8.0 };
inline constexpr sm::Vec2 PLAYER_HITBOX_OFFSET{ 10.0, 7.0 };
inline constexpr sm::Vec2 ENEMY_HITBOX_OFFSET{ 5.0, 12.0 };

inline constexpr int PLAYER_HEALTH{ 100 };
inline constexpr int ENEMY_HEALTH{ 100 };

inline constexpr se::BBoxRect PLAYER_CBOX_SIZE{ 20.0, 29.0 };
inline constexpr se::BBoxRect ENEMY_CBOX_SIZE{ 30.0, 24.0 };
inline constexpr se::BBoxRect PLAYER_HITBOX_SIZE{ 12.0, 21.0 };
inline constexpr se::BBoxRect ENEMY_HITBOX_SIZE{ 22.0, 16.0 };

inline constexpr se::BBoxCircle PROJECTILE_BBOX{ PROJECTILE_RADIUS };

namespace
{
auto pause_screen(Game& game) -> sui::Screen;
auto spawn_melee(Game& game, rl::Vector2 source_pos, size_t parent_id) -> void;
auto spawn_projectile(Game& game, rl::Vector2 source_pos, rl::Vector2 target_pos) -> void;
auto spawn_sector(Game& game, rl::Vector2 source_pos, rl::Vector2 target_pos, size_t parent_id) -> void;
auto spawn_sector_lines(
    Game& game, unsigned line_count, rl::Vector2 source_pos, rl::Vector2 target_pos, size_t sector_id
) -> void;
} // namespace

Game::Game()
{
    window.SetTargetFPS(TARGET_FPS);
    window.SetExitKey(::KEY_NULL);

    components.reg<se::Pos>();
    components.reg<se::Vel>();
    components.reg<se::BBox>();
    components.reg<Flags>();
    components.reg<Combat>();
    components.reg<Parent>();

    for (size_t i{ 0 }; i < 10; i++) // NOLINT
    {
        spawn_tile(Tile::Brick, Coords{ 0, i });
    }

    for (size_t i{ 0 }; i < 20; i++) // NOLINT
    {
        spawn_tile(Tile::Brick, Coords{ i, 0 });
    }

    for (size_t i{ 0 }; i < 4; i++) // NOLINT
    {
        spawn_tile(Tile::Brick, Coords{ i, 2 });
        spawn_tile(Tile::Brick, Coords{ i, 3 });
    }

    spawn_tile(Tile::Brick, Coords{ 2, 7 }); // NOLINT
    spawn_tile(Tile::Brick, Coords{ 1, 6 }); // NOLINT
    spawn_tile(Tile::Brick, Coords{ 2, 4 }); // NOLINT
    world.calculate_cboxes();

    spawn_player(Coords{ 5, 2 });             // NOLINT
    spawn_enemy(Enemy::Duck, Coords{ 6, 6 }); // NOLINT
}

auto Game::run() -> void
{
    // input
    poll_inputs();
    check_pause_game();
    ui_click_action();
    if (!paused)
    {
        // movement
        set_player_vel();
        move();
        set_flipped();
        sync_children();

        // combat
        player_attack();
        update_invuln_times();
        damage_entities();
        update_lifespans();
        destroy_entities();
    }

    // render
    window.BeginDrawing();
    window.ClearBackground(::SKYBLUE);
    camera.SetTarget(components.get<se::Pos>(player_id) + (SPRITE_SIZE / 2));
    camera.BeginMode();
    render_damage_lines();
    render_sprites();
#ifdef SHOW_CBOXES
    render_cboxes();
#endif
#ifdef SHOW_HITBOXES
    render_hitboxes();
#endif

    camera.EndMode();
    render_ui();
    window.EndDrawing();

    if (inputs.spawn_enemy)
    {
        spawn_enemy(Enemy::Duck, Coords{ 6, 6 }); // NOLINT
    }
}

auto Game::spawn_player(const Coords coords) -> void
{
    const auto id{ entities.spawn(Entity::Player) };
    player_id = id;
    auto comps{ components.by_id(id) };
    comps.get<se::Pos>() = coords;
    comps.get<se::BBox>() = se::BBox{ PLAYER_CBOX_SIZE, PLAYER_CBOX_OFFSET };
    auto& combat{ comps.get<Combat>() };
    combat.health.set(PLAYER_HEALTH);
    combat.hitbox = se::BBox{ PLAYER_HITBOX_SIZE, PLAYER_HITBOX_OFFSET };
}

auto Game::spawn_enemy(const Enemy enemy, const Coords coords) -> void
{
    auto sprite_base{ SpriteBase::None };
    switch (enemy)
    {
    case Enemy::Duck:
        sprite_base = SpriteBase::EnemyDuck;
        break;
    }

    const auto id{ entities.spawn(Entity::Enemy) };
    slog::log(slog::TRC, "Spawning enemy with id {}", id);
    auto comps{ components.by_id(id) };
    comps.get<se::Pos>() = coords;
    comps.get<se::BBox>() = se::BBox{ ENEMY_CBOX_SIZE, ENEMY_CBOX_OFFSET };
    auto& combat{ comps.get<Combat>() };
    combat.health.set(ENEMY_HEALTH);
    combat.hitbox = se::BBox{ ENEMY_HITBOX_SIZE, ENEMY_HITBOX_OFFSET };
    sprites.set(id, sprite_base);
}

void Game::spawn_tile(const Tile tile, const Coords coords)
{
    auto sprite{ SpriteTile::None };
    switch (tile)
    {
    case Tile::None:
        sprite = SpriteTile::None;
        break;
    case Tile::Brick:
        sprite = SpriteTile::Brick;
        break;
    }

    world.spawn(coords, tile, sprite);
}

auto Game::dt() const -> float
{
    return window.GetFrameTime();
}

auto Game::get_mouse_pos() const -> rl::Vector2
{
    return camera.GetScreenToWorld(rl::Mouse::GetPosition()) - SPRITE_SIZE / 2;
}

auto Game::destroy_entity(const size_t id) -> void
{
    if (entities.vec()[id] == Entity::None)
    {
        return;
    }

    entities.destroy(id);
    components.uninit(id);
    sprites.unset(id);
    for (const auto [child_id, parent] : components.vec<Parent>() | std::views::enumerate)
    {
        if (parent.id != std::nullopt && parent.id.value() == id)
        {
            destroy_entity(child_id);
        }
    }
}

auto Game::spawn_attack(const Attack attack, const size_t parent_id) -> void
{
    const auto source_pos{ components.get<se::Pos>(parent_id) };
    slog::log(slog::TRC, "Attack source pos ({}, {})", source_pos.x, source_pos.y);
    const auto target_pos{ (parent_id == player_id ? get_mouse_pos() : components.get<se::Pos>(player_id)) };
    slog::log(slog::TRC, "Attack target pos ({}, {})", target_pos.x, target_pos.y);
    switch (attack)
    {
    case Attack::Melee:
        spawn_melee(*this, source_pos, parent_id);
        break;
    case Attack::Projectile:
        spawn_projectile(*this, source_pos, target_pos);
        break;
    case Attack::Sector:
        spawn_sector(*this, source_pos, target_pos, parent_id);
        break;
    }
}

auto Game::toggle_pause() -> void
{
    paused = !paused;
    if (paused)
    {
        screen = pause_screen(*this);
        slog::log(slog::INF, "Game paused");
    }
    else
    {
        screen = std::nullopt;
        slog::log(slog::INF, "Game unpaused");
    }
}

#ifndef NDEBUG
#include "hot-reload.hpp"

SLHR_EXPORT auto reload_texture_sheet(Game& game) -> void
{
    game.texture_sheet.Unload();
    game.texture_sheet.Load(TEXTURE_SHEET);
    slog::log(slog::INF, "Texture sheet reloaded");
}

SLHR_EXPORT auto run(Game& game) -> void
{
    game.run();
}

SLHR_EXPORT auto check_reload_lib() -> bool
{
    return rl::Keyboard::IsKeyPressed(::KEY_R);
}
#endif

namespace
{
auto pause_screen(Game& game) -> sui::Screen
{
    sui::Screen screen;
    auto resume{ screen.new_element<sui::Button>() };
    resume.element->set_pos(sui::PercentSize{ 50, 40 });  // NOLINT(*magic-numbers)
    resume.element->set_size(sui::PercentSize{ 20, 10 }); // NOLINT(*magic-numbers)
    resume.element->color = ::WHITE;
    resume.element->text.text = "Resume";
    resume.element->text.size = sui::TextPctSize{ 6 }; // NOLINT(*magic-numbers)
    resume.element->on_click = [&game]() { game.toggle_pause(); };

    auto exit{ screen.new_element<sui::Button>() };
    exit.element->set_pos(sui::PercentSize{ 50, 60 });  // NOLINT(*magic-numbers)
    exit.element->set_size(sui::PercentSize{ 20, 10 }); // NOLINT(*magic-numbers)
    exit.element->color = ::WHITE;
    exit.element->text.text = "Exit";
    exit.element->text.size = sui::TextPctSize{ 6 }; // NOLINT(*magic-numbers)
    exit.element->on_click = [&game]() { game.close = true; };

    return screen;
}

void spawn_melee(Game& game, const rl::Vector2 source_pos, const size_t parent_id)
{
    const auto details{ entities::attack_details(Attack::Melee) };
    const auto melee_details{ std::get<MeleeDetails>(details.details) };
    const auto id{ game.entities.spawn(Entity::Melee) };
    auto comps{ game.components.by_id(id) };
    comps.get<se::Pos>() = source_pos;
    auto& combat{ comps.get<Combat>() };
    combat.lifespan = details.lifespan;
    combat.hitbox = se::BBox{ melee_details.size, MELEE_OFFSET };
    combat.damage = details.damage;
    comps.get<Parent>().id = parent_id;
}

void spawn_projectile(Game& game, const rl::Vector2 source_pos, const rl::Vector2 target_pos)
{
    const auto diff{ target_pos - source_pos };
    const auto angle{ std::atan2(diff.y, diff.x) };
    const auto details{ entities::attack_details(Attack::Projectile) };
    const auto proj_details{ std::get<ProjectileDetails>(details.details) };
    const auto vel{ rl::Vector2{ std::cos(angle), std::sin(angle) } * proj_details.speed };
    const auto id{ game.entities.spawn(Entity::Projectile) };
    auto comps{ game.components.by_id(id) };
    comps.get<se::Pos>() = source_pos + (SPRITE_SIZE / 2) - rl::Vector2{ PROJECTILE_RADIUS, PROJECTILE_RADIUS };
    comps.get<se::Vel>() = vel;
    comps.get<se::BBox>() = se::BBox{ PROJECTILE_BBOX };
    game.sprites.set(id, SpriteBase::Projectile);
    auto& combat{ comps.get<Combat>() };
    combat.lifespan = details.lifespan;
    combat.hitbox = se::BBox{ PROJECTILE_BBOX };
    combat.damage = details.damage;
}

void spawn_sector(Game& game, const rl::Vector2 source_pos, const rl::Vector2 target_pos, const size_t parent_id)
{
    const auto details{ entities::attack_details(Attack::Sector) };
    const auto sector_det{ std::get<SectorDetails>(details.details) };
    const auto line_count{ static_cast<size_t>(ceil(sector_det.radius * sector_det.angle / LINE_ANGLE_SPACING)) + 1 };
    slog::log(slog::TRC, "Spawning {} damage lines", line_count);
    const auto sector_id{ game.entities.spawn(Entity::Sector) };
    auto comps{ game.components.by_id(sector_id) };
    comps.get<Combat>().lifespan = details.lifespan;
    comps.get<Parent>().id = parent_id;
    spawn_sector_lines(game, line_count, source_pos, target_pos, sector_id);
}

void spawn_sector_lines(
    Game& game,
    const unsigned line_count,
    const rl::Vector2 source_pos,
    const rl::Vector2 target_pos,
    const size_t sector_id
)
{
    const auto diff{ target_pos - source_pos };
    const auto angle{ std::atan2(diff.y, diff.x) };
    const auto details{ entities::attack_details(Attack::Sector) };
    const auto sector_details{ std::get<SectorDetails>(details.details) };
    const auto initial_angle{ angle - (sector_details.angle / 2) };
    const auto angle_diff{ sector_details.angle / static_cast<float>(line_count - 1) };
    slog::log(slog::TRC, "Angle between damage lines: {}", sl::math::radians_to_degrees(angle_diff));
    const auto sector_offset{ (sm::Vec2{ std::cos(angle), std::sin(angle) } * sector_details.sector_offset)
                              + (SPRITE_SIZE / 2) };
    for (size_t i{ 0 }; i < line_count; i++)
    {
        const auto line_id{ game.entities.spawn(Entity::DamageLine) };
        const auto line_ang{ initial_angle + (angle_diff * static_cast<float>(i)) };
        const auto offset{ sector_offset
                           + sm::Vec2{ std::cos(line_ang), std::sin(line_ang) } * sector_details.line_offset };
        slog::log(slog::TRC, "Offsetting damage line by ({}, {})", offset.x, offset.y);
        auto comps{ game.components.by_id(line_id) };
        comps.get<se::Pos>() = source_pos;
        auto& combat{ comps.get<Combat>() };
        combat.hitbox = se::BBox{ se::BBoxLine{ sector_details.radius, line_ang }, offset };
        combat.damage = details.damage;
        comps.get<Parent>().id = sector_id;
    }
}
} // namespace

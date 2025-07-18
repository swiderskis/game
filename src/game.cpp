#include "game.hpp"

#include "components.hpp"
#include "entities.hpp"
#include "seblib.hpp"
#include "sprites.hpp"

#include <cassert>
#include <cmath>
#include <optional>
#include <ranges>

namespace rl = raylib;
namespace sl = seblib;
namespace slog = seblib::log;
namespace se = seb_engine;
namespace sui = seb_engine::ui;

inline constexpr unsigned TARGET_FPS = 60;

inline constexpr float DAMAGE_LINES_INV_FREQ = 5.0;
inline constexpr float PROJECTILE_SIZE = 4.0;

inline constexpr auto PLAYER_CBOX_SIZE = sl::SimpleVec2(20.0, 29.0);
inline constexpr auto ENEMY_CBOX_SIZE = sl::SimpleVec2(30.0, 24.0);
inline constexpr auto PLAYER_HITBOX_SIZE = sl::SimpleVec2(12.0, 21.0);
inline constexpr auto PLAYER_HITBOX_OFFSET = sl::SimpleVec2(0.0, 4.0);
inline constexpr auto ENEMY_HITBOX_SIZE = sl::SimpleVec2(22.0, 16.0);
inline constexpr auto ENEMY_HITBOX_OFFSET = sl::SimpleVec2(0.0, 4.0);
inline constexpr auto MELEE_OFFSET = sl::SimpleVec2(24.0, 9.0);

inline constexpr int PLAYER_HEALTH = 100;
inline constexpr int ENEMY_HEALTH = 100;

namespace
{
sui::Screen pause_screen(Game& game);
} // namespace

Game::Game()
{
    window.SetTargetFPS(TARGET_FPS);
    window.SetExitKey(KEY_NULL);

    components.reg<Tform>();
    components.reg<Flags>();
    components.reg<Combat>();
    components.reg<Parent>();

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
    // input
    poll_inputs();
    check_pause_game();
    ui_click_action();
    if (!paused)
    {
        // movement
        set_player_vel();
        move_entities();
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
    render();
    render_ui();
    window.EndDrawing();

    if (inputs.spawn_enemy)
    {
        spawn_enemy(Enemy::Duck, Coordinates(2, 2));
    }
}

void Game::spawn_player(const rl::Vector2 pos)
{
    const unsigned id = entities.spawn(Entity::Player);
    player_id = id;
    auto comps = components.by_id(id);
    auto& transform = comps.get<Tform>();
    transform.pos = pos;
    transform.cbox.set(pos, PLAYER_CBOX_SIZE);
    auto& combat = comps.get<Combat>();
    combat.health.set(PLAYER_HEALTH);
    combat.hitbox.set(pos, PLAYER_HITBOX_SIZE);
    combat.hitbox.set_offset(pos, PLAYER_HITBOX_OFFSET);
}

void Game::spawn_enemy(const Enemy enemy, const rl::Vector2 pos)
{
    auto sprite_base = SpriteBase::None;
    switch (enemy)
    {
    case Enemy::Duck:
        sprite_base = SpriteBase::EnemyDuck;
        break;
    }

    const unsigned id = entities.spawn(Entity::Enemy);
    auto comps = components.by_id(id);
    auto& transform = comps.get<Tform>();
    transform.pos = pos;
    transform.cbox.set(pos, ENEMY_CBOX_SIZE);
    auto& combat = comps.get<Combat>();
    combat.health.set(ENEMY_HEALTH);
    combat.hitbox.set(pos, ENEMY_HITBOX_SIZE);
    combat.hitbox.set_offset(pos, ENEMY_HITBOX_OFFSET);
    sprites.set(id, sprite_base);
}

void Game::spawn_tile(const Tile tile, const rl::Vector2 pos)
{
    auto sprite = SpriteTile::None;
    switch (tile)
    {
    case Tile::None:
        sprite = SpriteTile::None;
        break;
    case Tile::Brick:
        sprite = SpriteTile::TileBrick;
        break;
    }

    tiles.spawn(tile, sprite, pos);
}

float Game::dt() const
{
    return window.GetFrameTime();
}

rl::Vector2 Game::get_mouse_pos() const
{
    return camera.GetScreenToWorld(rl::Mouse::GetPosition()) - rl::Vector2(se::SPRITE_SIZE, se::SPRITE_SIZE) / 2;
}

void Game::destroy_entity(const unsigned id)
{
    if (entities.entities()[id] == Entity::None)
    {
        return;
    }

    entities.destroy_entity(id);
    components.uninit_destroyed_entity(id);
    // destroy any child entities
    for (const auto [child_id, parent] : components.vec<Parent>() | std::views::enumerate | std::views::as_const)
    {
        if (parent.id != std::nullopt && parent.id.value() == id)
        {
            destroy_entity(child_id);
        }
    }
}

void Game::spawn_attack(const Attack attack, const unsigned parent_id)
{
    const auto details = entities::attack_details(attack);
    const auto source_pos = components.get<Tform>(parent_id).pos;
    slog::log(slog::TRC, "Attack source pos ({}, {})", source_pos.x, source_pos.y);
    const auto target_pos = (parent_id == player_id ? get_mouse_pos() : components.get<Tform>(player_id).pos);
    slog::log(slog::TRC, "Attack target pos ({}, {})", target_pos.x, target_pos.y);
    const auto diff = target_pos - source_pos;
    const float angle = atan2(diff.y, diff.x);
    switch (attack)
    {
    case Attack::Melee:
    {
        const auto melee_details = std::get<MeleeDetails>(details.details);
        const unsigned id = entities.spawn(Entity::Melee);
        auto comps = components.by_id(id);
        auto& transform = comps.get<Tform>();
        transform.pos = source_pos;
        auto combat = comps.get<Combat>();
        combat.lifespan = details.lifespan;
        combat.hitbox.set(source_pos, melee_details.size);
        combat.hitbox.set_offset(source_pos, MELEE_OFFSET);
        combat.damage = details.damage;
        comps.get<Parent>().id = parent_id;
        break;
    }
    case Attack::Projectile:
    {
        const auto proj_details = std::get<ProjectileDetails>(details.details);
        const auto vel = rl::Vector2(cos(angle), sin(angle)) * proj_details.speed;
        const unsigned id = entities.spawn(Entity::Projectile);
        auto comps = components.by_id(id);
        auto& transform = comps.get<Tform>();
        transform.pos = source_pos;
        transform.vel = vel;
        transform.cbox.set(source_pos, PROJECTILE_SIZE);
        sprites.set(id, SpriteBase::Projectile);
        auto& combat = comps.get<Combat>();
        combat.lifespan = details.lifespan;
        combat.hitbox.set(source_pos, PROJECTILE_SIZE);
        combat.damage = details.damage;
        break;
    }
    case Attack::Sector:
    {
        const auto sector_details = std::get<SectorDetails>(details.details);
        const float initial_angle = angle - (sector_details.ang / 2);
        const unsigned damage_lines
            = 1 + static_cast<unsigned>(ceil(sector_details.radius * sector_details.ang / DAMAGE_LINES_INV_FREQ));
        slog::log(slog::TRC, "Spawning {} damage lines", damage_lines);
        const float angle_diff = sector_details.ang / static_cast<float>(damage_lines - 1.0);
        slog::log(slog::TRC, "Angle between damage lines: {}", sl::math::radians_to_degrees(angle_diff));
        const auto ext_offset = rl::Vector2(cos(angle), sin(angle)) * sector_details.external_offset;
        const unsigned sector_id = entities.spawn(Entity::Sector);
        auto comps = components.by_id(sector_id);
        comps.get<Combat>().lifespan = details.lifespan;
        comps.get<Parent>().id = parent_id;
        for (unsigned i = 0; i < damage_lines; i++)
        {
            const unsigned line_id = entities.spawn(Entity::DamageLine);
            const auto line_ang = initial_angle + (angle_diff * static_cast<float>(i));
            const auto offset = ext_offset + rl::Vector2(cos(line_ang), sin(line_ang)) * sector_details.internal_offset;
            slog::log(slog::TRC, "Offsetting damage line by ({}, {})", offset.x, offset.y);
            comps = components.by_id(line_id);
            comps.get<Tform>().pos = source_pos;
            auto& combat = comps.get<Combat>();
            combat.hitbox.set(source_pos, sector_details.radius, line_ang);
            combat.hitbox.set_offset(source_pos, offset);
            combat.damage = details.damage;
            comps.get<Parent>().id = sector_id;
        }

        break;
    }
    }
}

void Game::toggle_pause()
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

void Game::reload_texture_sheet()
{
    texture_sheet.Unload();
    texture_sheet.Load(TEXTURE_SHEET);
    slog::log(slog::INF, "Texture sheet reloaded");
}

EXPORT void run(Game& game)
{
    game.run();
}

EXPORT bool check_reload_lib()
{
    return rl::Keyboard::IsKeyPressed(KEY_R);
}
#endif

namespace
{

sui::Screen pause_screen(Game& game)
{
    sui::Screen screen;
    auto resume = screen.new_element<sui::Button>();
    resume.element->set_pos(sui::PercentSize{ .width = 50, .height = 40 });  // NOLINT(*magic-numbers)
    resume.element->set_size(sui::PercentSize{ .width = 20, .height = 10 }); // NOLINT(*magic-numbers)
    resume.element->color = ::WHITE;
    resume.element->text.text = "Resume";
    resume.element->text.set_percent_size(6); // NOLINT(*magic-numbers)
    resume.element->on_click = [&game]() { game.toggle_pause(); };

    auto exit = screen.new_element<sui::Button>();
    exit.element->set_pos(sui::PercentSize{ .width = 50, .height = 60 });  // NOLINT(*magic-numbers)
    exit.element->set_size(sui::PercentSize{ .width = 20, .height = 10 }); // NOLINT(*magic-numbers)
    exit.element->color = ::WHITE;
    exit.element->text.text = "Exit";
    exit.element->text.set_percent_size(6); // NOLINT(*magic-numbers)
    exit.element->on_click = [&game]() { game.close = true; };

    return screen;
}
} // namespace

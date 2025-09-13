#include "components.hpp"
#include "entities.hpp"
#include "game.hpp"
#include "se-bbox.hpp"
#include "se-sprite.hpp"
#include "sl-log.hpp"
#include "sprites.hpp"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <optional>
#include <ranges>
#include <type_traits>

namespace rl = raylib;
namespace sm = seblib::math;
namespace slog = seblib::log;
namespace se = seb_engine;

static constexpr std::initializer_list DESTROY_ON_COLLISION{
    Entity::Projectile,
};
static constexpr std::initializer_list DAMAGING_ENTITIES{
    Entity::Projectile,
    Entity::Melee,
    Entity::DamageLine,
};
static constexpr std::initializer_list FLIP_ON_SYNC_WITH_PARENT{
    Entity::Melee,
};
static constexpr std::initializer_list ENTITY_RENDER_ORDER{
    Entity::DamageLine, Entity::Enemy, Entity::Player, Entity::Melee, Entity::Projectile,
};
static constexpr std::initializer_list NON_FLIPPABLE{
    Entity::Projectile,
};

inline constexpr sm::Vec2 HEALTH_BAR_SIZE{ 32.0, 4.0 };

inline constexpr float PLAYER_SPEED{ 100.0 };
inline constexpr float HEALTH_BAR_Y_OFFSET{ 8.0 };
inline constexpr float INVULN_TIME{ 0.5 };
inline constexpr float DAMAGE_LINE_THICKNESS{ 1.33 };

namespace
{
auto resolve_tile_collisions(Game& game) -> void;
auto draw_sprite(Game& game, size_t id) -> void;
template <typename SpriteEnum>
    requires IsEntitySpriteEnum<SpriteEnum>
auto draw_sprite_part(Game& game, size_t id) -> void;
} // namespace

auto Game::poll_inputs() -> void
{
    inputs.left = rl::Keyboard::IsKeyDown(::KEY_A);
    inputs.right = rl::Keyboard::IsKeyDown(::KEY_D);
    inputs.up = rl::Keyboard::IsKeyDown(::KEY_W);
    inputs.down = rl::Keyboard::IsKeyDown(::KEY_S);
    inputs.click = rl::Mouse::IsButtonPressed(::MOUSE_LEFT_BUTTON);
    inputs.spawn_enemy = rl::Keyboard::IsKeyPressed(::KEY_P);
    inputs.pause = rl::Keyboard::IsKeyPressed(::KEY_ESCAPE);
}

auto Game::render_sprites() -> void
{
    world.draw(texture_sheet, dt());
    for (const auto entity : ENTITY_RENDER_ORDER)
    {
        for (const auto id : entities.ids(entity))
        {
            auto comps{ components.by_id(id) };
            const auto vel{ comps.get<se::Vel>() };
            const auto pos{ comps.get<se::Pos>() };
            sprites::lookup_set_movement_sprites(sprites, id, entity, vel);
            draw_sprite(*this, id);

            const auto health{ comps.get<Combat>().health };
            if (health.max != std::nullopt && health.current != health.max)
            {
                const auto hp_bar_pos{ pos - rl::Vector2{ 0.0, HEALTH_BAR_Y_OFFSET } };
                const auto current_bar_width{ HEALTH_BAR_SIZE.x * health.percentage() };
                rl::Rectangle{ hp_bar_pos, HEALTH_BAR_SIZE }.Draw(::RED);
                rl::Rectangle{ hp_bar_pos, { current_bar_width, HEALTH_BAR_SIZE.y } }.Draw(::GREEN);
            }
        }
    }
}

#ifdef SHOW_CBOXES
auto Game::render_cboxes() -> void
{
    world.draw_cboxes();
    for (const auto entity : ENTITY_RENDER_ORDER)
    {
        for (const auto id : entities.ids(entity))
        {
            const auto pos{ components.get<se::Pos>(id) };
            const auto cbox{ components.get<se::BBox>(id).val(pos) };
            sl::match(
                cbox,
                [](const rl::Rectangle bbox)
                {
                    slog::log(slog::TRC, "CBox pos ({}, {})", bbox.x, bbox.y);
                    bbox.DrawLines(::RED);
                },
                [](const sm::Circle bbox)
                {
                    slog::log(slog::TRC, "CBox pos ({}, {})", bbox.pos.x, bbox.pos.y);
                    bbox.draw_lines(::RED);
                },
                [](const sm::Line bbox)
                {
                    slog::log(slog::TRC, "CBox pos ({}, {})", bbox.pos1.x, bbox.pos2.y);
                    bbox.draw(::RED);
                }
            );
        }
    }
}
#endif

#ifdef SHOW_HITBOXES
auto Game::render_hitboxes() -> void
{
    for (const auto entity : ENTITY_RENDER_ORDER)
    {
        for (const auto id : entities.ids(entity))
        {
            const auto pos{ components.get<se::Pos>(id) };
            const auto hitbox{ components.get<Combat>(id).hitbox.val(pos) };
            sl::match(
                hitbox,
                [](const rl::Rectangle bbox) { bbox.DrawLines(::GREEN); },
                [](const sm::Circle bbox) { bbox.draw_lines(::GREEN); },
                [](const sm::Line bbox) { bbox.draw(::GREEN); }
            );
        }
    }
}
#endif

auto Game::set_player_vel() -> void
{
    auto& player_vel{ components.get<se::Vel>(player_id) };
    player_vel.x = 0.0;
    player_vel.x += (inputs.right ? PLAYER_SPEED : 0.0F);
    player_vel.x -= (inputs.left ? PLAYER_SPEED : 0.0F);
    player_vel.y = 0.0;
    player_vel.y -= (inputs.up ? PLAYER_SPEED : 0.0F);
    player_vel.y += (inputs.down ? PLAYER_SPEED : 0.0F);
}

auto Game::move() -> void
{
    auto& pos{ components.vec<se::Pos>() };
    auto& vel{ components.vec<se::Vel>() };
    std::ranges::transform(
        pos, vel, pos.begin(), [this](const auto pos, const auto vel) { return pos + (vel * dt()); }
    );
    resolve_tile_collisions(*this);
}

auto Game::destroy_entities() -> void
{
    for (const auto id : to_destroy)
    {
        destroy_entity(id);
    }

    to_destroy.clear();
}

auto Game::player_attack() -> void
{
    auto comps{ components.by_id(player_id) };
    auto& attack_cooldown{ comps.get<Combat>().attack_cooldown };
    if (attack_cooldown > 0.0)
    {
        attack_cooldown -= dt();
    }

    if (inputs.click && attack_cooldown <= 0.0)
    {
        const auto attack{ Attack::Sector };
        const auto attack_details{ entities::attack_details(attack) };
        sprites.set(player_id, SpriteArms::PlayerAttack, attack_details.lifespan);
        comps.get<Combat>().attack_cooldown = attack_details.cooldown;
        spawn_attack(attack, player_id);
    }
}

auto Game::update_lifespans() -> void
{
    for (const auto& [id, combat] : components.vec<Combat>() | std::views::enumerate)
    {
        auto& lifespan{ combat.lifespan };
        if (lifespan == std::nullopt)
        {
            continue;
        }

        lifespan.value() -= dt();
        if (lifespan.value() < 0.0)
        {
            to_destroy.push_back(id);
        }
    }
}

auto Game::damage_entities() -> void
{
    for (const auto entity : DAMAGING_ENTITIES)
    {
        for (const auto id : entities.ids(entity))
        {
            auto comps{ components.by_id(id) };
            const auto pos{ comps.get<se::Pos>() };
            const auto projectile_bbox{ comps.get<Combat>().hitbox.val(pos) };
            for (const auto enemy_id : entities.ids(Entity::Enemy))
            {
                auto& enemy_combat_comps{ components.by_id(enemy_id).get<Combat>() };
                const auto enemy_pos{ components.by_id(enemy_id).get<se::Pos>() };
                const auto enemy_bbox{ enemy_combat_comps.hitbox.val(enemy_pos) };
                auto& invuln_time{ enemy_combat_comps.invuln_time };
                if (invuln_time > 0.0 || !se::bbox::collides(enemy_bbox, projectile_bbox))
                {
                    continue;
                }

                auto& enemy_current_health{ enemy_combat_comps.health.current };
                enemy_current_health -= static_cast<int>(comps.get<Combat>().damage);
                if (enemy_current_health <= 0)
                {
                    to_destroy.push_back(enemy_id);
                }

                if (entity == Entity::Projectile)
                {
                    to_destroy.push_back(id);
                    break;
                }

                invuln_time = INVULN_TIME;
            }
        }
    }
}

// child entities are assumed to have no velocity, this system will override it
auto Game::sync_children() -> void
{
    for (const auto [id, entity] : entities.vec() | std::views::enumerate)
    {
        auto comps{ components.by_id(id) };
        if (comps.get<Parent>().id == std::nullopt)
        {
            continue;
        }

        const auto parent_id{ comps.get<Parent>().id.value() };
        auto parent_comps{ components.by_id(parent_id) };
        if (std::ranges::contains(FLIP_ON_SYNC_WITH_PARENT, entity))
        {
            const auto is_parent_flipped{ parent_comps.get<Flags>().is_enabled(Flags::FLIPPED) };
            auto& flags{ comps.get<Flags>() };
            flags.set(Flags::FLIPPED, is_parent_flipped);
            if (entity == Entity::Melee)
            {
                auto& combat{ comps.get<Combat>() };
                const auto details{ std::get<MeleeDetails>(entities::attack_details(Attack::Melee).details) };
                combat.hitbox = se::BBox{ details.size,
                                          (flags.is_enabled(Flags::FLIPPED) ? MELEE_OFFSET_FLIPPED : MELEE_OFFSET) };
            }
        }

        auto& pos{ comps.get<se::Pos>() };
        const auto parent_pos{ parent_comps.get<se::Pos>() };
        pos = parent_pos;
        slog::log(slog::TRC, "Child pos: ({}, {})", pos.x, pos.y);
        slog::log(slog::TRC, "Parent pos: ({}, {})", parent_pos.x, parent_pos.y);
    }
}

auto Game::update_invuln_times() -> void
{
    for (const auto& [id, combat] : components.vec<Combat>() | std::views::enumerate)
    {
        auto& invuln_time{ combat.invuln_time };
        if (invuln_time > 0.0)
        {
            invuln_time -= dt();
        }
    }
}

auto Game::render_ui() -> void
{
    if (screen != std::nullopt)
    {
        screen.value().render();
    }
}

auto Game::check_pause_game() -> void
{
    if (inputs.pause)
    {
        toggle_pause();
    }
}

auto Game::ui_click_action() -> void
{
    if (inputs.click && screen != std::nullopt)
    {
        screen->click_action(rl::Mouse::GetPosition());
    }
}

auto Game::set_flipped() -> void
{
    for (const auto [id, entity] : entities.vec() | std::views::enumerate)
    {
        auto comps{ components.by_id(id) };
        auto& vel{ comps.get<se::Vel>() };
        if (vel.x != 0.0 && !std::ranges::contains(NON_FLIPPABLE, entity))
        {
            components.get<Flags>(id).set(Flags::FLIPPED, vel.x < 0.0);
        }
    }
}

auto Game::render_damage_lines() -> void
{
    for (const auto id : entities.ids(Entity::DamageLine))
    {
        auto comps{ components.by_id(id) };
        const auto pos{ comps.get<se::Pos>() };
        const auto line{ std::get<sm::Line>(comps.get<Combat>().hitbox.val(pos)) };
        ::DrawLineEx(line.pos1, line.pos2, DAMAGE_LINE_THICKNESS, ::LIGHTGRAY);
    }
}

namespace
{
auto resolve_tile_collisions(Game& game) -> void
{
    for (const auto [id, entity] : game.entities.vec() | std::views::enumerate)
    {
        auto& pos{ game.components.get<se::Pos>(id) };
        const auto vel{ game.components.get<se::Vel>(id) };
        for (const auto tile_cbox : game.world.cboxes())
        {
            const auto cbox{ game.components.get<se::BBox>(id).val(pos) };
            if (se::bbox::collides(cbox, tile_cbox))
            {
                pos += se::bbox::resolve_collision(cbox, tile_cbox);
            }
        }
    }
}

auto draw_sprite(Game& game, const size_t id) -> void
{
    draw_sprite_part<SpriteBase>(game, id);
    draw_sprite_part<SpriteHead>(game, id);
    draw_sprite_part<SpriteArms>(game, id);
    draw_sprite_part<SpriteLegs>(game, id);
    draw_sprite_part<SpriteExtra>(game, id);
}

template <typename SpriteEnum>
    requires IsEntitySpriteEnum<SpriteEnum>
auto draw_sprite_part(Game& game, const size_t id) -> void
{
    const auto pos{ game.components.get<se::Pos>(id) };
    const auto flags{ game.components.get<Flags>(id) };
    auto& sprites{ game.sprites };
    const auto flipped{ flags.is_enabled(Flags::FLIPPED) };
    if constexpr (std::is_same_v<SpriteEnum, SpriteLegs>)
    {
        sprites.draw_part<SpriteEnum>(game.texture_sheet, pos, id, game.dt(), flipped);
    }
    else
    {
        const auto sprite_size{ game.sprites.details<SpriteEnum>(id).size };
        const auto x_offset{ (flipped ? sprites::flipped_x_offset(sprite_size) : 0.0F) };
        const auto legs{ sprites.sprite<SpriteLegs>(id) };
        const auto legs_frame{ sprites.current_frame<SpriteLegs>(id) };
        const auto y_offset{ (legs_frame % 2 == 0 ? 0.0F : sprites::alternate_frame_y_offset(legs)) };
        const rl::Vector2 offset{ x_offset, y_offset };
        sprites.draw_part<SpriteEnum>(game.texture_sheet, pos + offset, id, game.dt(), flipped);
    }
}
} // namespace

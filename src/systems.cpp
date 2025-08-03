#include "components.hpp"
#include "entities.hpp"
#include "game.hpp"
#include "seblib-log.hpp"
#include "seblib.hpp"
#include "sprites.hpp"

#include <algorithm>
#include <cassert>
#include <optional>
#include <ranges>

#ifndef NDEBUG
#define SHOW_CBOXES
#undef SHOW_CBOXES
#define SHOW_HITBOXES
#undef SHOW_HITBOXES
#endif

namespace rl = raylib;
namespace sl = seblib;
namespace smath = seblib::math;
namespace slog = seblib::log;
namespace se = seb_engine;

static constexpr auto DESTROY_ON_COLLISION = {
    Entity::Projectile,
};
static constexpr auto DAMAGING_ENTITIES = {
    Entity::Projectile,
    Entity::Melee,
    Entity::DamageLine,
};
static constexpr auto FLIP_ON_SYNC_WITH_PARENT = {
    Entity::Melee,
};
static constexpr auto ENTITY_RENDER_ORDER = {
    Entity::DamageLine, Entity::Enemy, Entity::Player, Entity::Melee, Entity::Projectile,
};

inline constexpr sl::SimpleVec2 HEALTH_BAR_SIZE{ 32.0, 4.0 };

inline constexpr float PLAYER_SPEED = 100.0;
inline constexpr float HEALTH_BAR_Y_OFFSET = 8.0;
inline constexpr float INVULN_TIME = 0.5;
inline constexpr float DAMAGE_LINE_THICKNESS = 1.33;

namespace
{
void resolve_tile_collisions(Game& game, unsigned id, se::BBox prev_cbox);
void draw_sprite(Game& game, unsigned id);
} // namespace

void Game::poll_inputs()
{
    inputs.left = rl::Keyboard::IsKeyDown(KEY_A);
    inputs.right = rl::Keyboard::IsKeyDown(KEY_D);
    inputs.up = rl::Keyboard::IsKeyDown(KEY_W);
    inputs.down = rl::Keyboard::IsKeyDown(KEY_S);
    inputs.click = rl::Mouse::IsButtonPressed(MOUSE_LEFT_BUTTON);
    inputs.spawn_enemy = rl::Keyboard::IsKeyPressed(KEY_P);
    inputs.pause = rl::Keyboard::IsKeyPressed(KEY_ESCAPE);
}

void Game::render()
{
    const auto player_pos = components.get<Tform>(player_id).pos;
    camera.SetTarget(player_pos + rl::Vector2(se::SPRITE_SIZE, se::SPRITE_SIZE) / 2);
    camera.BeginMode();
    world.draw(texture_sheet, dt());
    for (const auto entity : ENTITY_RENDER_ORDER)
    {
        for (const unsigned id : entities.entity_ids(entity))
        {
            auto comps = components.by_id(id);
            if (entity == Entity::DamageLine)
            {
                const auto line = std::get<smath::Line>(comps.get<Combat>().hitbox.bbox());
                line.pos1.DrawLine(line.pos2, DAMAGE_LINE_THICKNESS, ::LIGHTGRAY);
                continue;
            }

            const auto transform = comps.get<Tform>();
            sprites::lookup_set_movement_sprites(sprites, id, entity, transform.vel);
            draw_sprite(*this, id);

            const auto health = comps.get<Combat>().health;
            if (health.max != std::nullopt && health.current != health.max)
            {
                const auto pos = transform.pos - rl::Vector2(0.0, HEALTH_BAR_Y_OFFSET);
                const float current_bar_width = HEALTH_BAR_SIZE.x * health.percentage();
                rl::Rectangle(pos, HEALTH_BAR_SIZE).Draw(::RED);
                rl::Rectangle(pos, rl::Vector2(current_bar_width, HEALTH_BAR_SIZE.y)).Draw(::GREEN);
            }
        }
    }

#ifdef SHOW_CBOXES
    world.draw_cboxes();
    for (const auto entity : ENTITY_RENDER_ORDER)
    {
        for (const unsigned id : entities.entity_ids(entity))
        {
            sl::match(
                components.get<Tform>(id).cbox.bbox(),
                [](const rl::Rectangle bbox) { bbox.DrawLines(::RED); },
                [](const smath::Circle bbox) { bbox.draw_lines(::RED); },
                [](const smath::Line bbox) { bbox.draw_line(::RED); });
        }
    }
#endif

#ifdef SHOW_HITBOXES
    for (const auto entity : ENTITY_RENDER_ORDER)
    {
        for (const unsigned id : entities.entity_ids(entity))
        {
            sl::match(
                components.get<Combat>(id).hitbox.bbox(),
                [](const rl::Rectangle bbox) { bbox.DrawLines(::GREEN); },
                [](const smath::Circle bbox) { bbox.draw_lines(::GREEN); },
                [](const smath::Line bbox) { bbox.draw_line(::GREEN); });
        }
    }
#endif

    camera.EndMode();
}

void Game::set_player_vel()
{
    auto& player_vel = components.get<Tform>(player_id).vel;
    player_vel.x = 0.0;
    player_vel.x += (inputs.right ? PLAYER_SPEED : 0.0F);
    player_vel.x -= (inputs.left ? PLAYER_SPEED : 0.0F);
    player_vel.y = 0.0;
    player_vel.y -= (inputs.up ? PLAYER_SPEED : 0.0F);
    player_vel.y += (inputs.down ? PLAYER_SPEED : 0.0F);
}

void Game::move_entities()
{
    for (const auto [id, entity] : entities.entities() | std::views::enumerate)
    {
        if (entity == Entity::None)
        {
            continue;
        }

        auto comps = components.by_id(id);
        auto& transform = comps.get<Tform>();
        transform.pos += transform.vel * dt();
        if (transform.vel.x != 0.0)
        {
            components.get<Flags>(id).set(Flags::FLIPPED, transform.vel.x < 0.0);
        }

        auto& cbox = comps.get<Tform>().cbox;
        const auto prev_cbox = cbox;
        cbox.sync(transform.pos);
        resolve_tile_collisions(*this, id, prev_cbox);
        comps.get<Combat>().hitbox.sync(transform.pos);
        if (id == player_id)
        {
            slog::log(slog::TRC, "Player pos ({}, {})", transform.pos.x, transform.pos.y);
        }
    }
}

void Game::destroy_entities()
{
    for (const unsigned id : entities.to_destroy())
    {
        destroy_entity(id);
    }

    entities.clear_to_destroy();
}

void Game::player_attack()
{
    auto comps = components.by_id(player_id);
    auto& attack_cooldown = comps.get<Combat>().attack_cooldown;
    if (attack_cooldown > 0.0)
    {
        attack_cooldown -= dt();
    }

    if (!inputs.click || attack_cooldown > 0.0)
    {
        return;
    }

    const auto attack = Attack::Melee;
    const auto attack_details = entities::attack_details(attack);
    sprites.set(player_id, SpriteArms::PlayerAttack, attack_details.lifespan);
    comps.get<Combat>().attack_cooldown = attack_details.cooldown;
    spawn_attack(attack, player_id);
}

void Game::update_lifespans()
{
    for (const auto& [id, combat] : components.vec<Combat>() | std::views::enumerate)
    {
        auto& lifespan = combat.lifespan;
        if (lifespan == std::nullopt)
        {
            continue;
        }

        lifespan.value() -= dt();
        if (lifespan.value() < 0.0)
        {
            entities.queue_destroy(id);
        }
    }
}

void Game::damage_entities()
{
    for (const auto entity : DAMAGING_ENTITIES)
    {
        for (const unsigned id : entities.entity_ids(entity))
        {
            auto comps = components.by_id(id);
            const auto projectile_bbox = comps.get<Combat>().hitbox;
            for (const unsigned enemy_id : entities.entity_ids(Entity::Enemy))
            {
                auto& enemy_combat_comps = components.by_id(enemy_id).get<Combat>();
                const auto enemy_bbox = enemy_combat_comps.hitbox;
                float& invuln_time = enemy_combat_comps.invuln_time;
                if (invuln_time > 0.0 || !enemy_bbox.collides(projectile_bbox))
                {
                    continue;
                }

                int& enemy_current_health = enemy_combat_comps.health.current;
                enemy_current_health -= static_cast<int>(comps.get<Combat>().damage);
                if (enemy_current_health <= 0)
                {
                    entities.queue_destroy(enemy_id);
                }

                if (entity == Entity::Projectile)
                {
                    entities.queue_destroy(id);
                    break;
                }

                invuln_time = INVULN_TIME;
            }
        }
    }
}

// child entities are assumed to have no velocity, this system will override it
void Game::sync_children()
{
    for (const auto [id, entity] : entities.entities() | std::views::enumerate)
    {
        auto comps = components.by_id(id);
        if (entity == Entity::None || comps.get<Parent>().id == std::nullopt)
        {
            continue;
        }

        const unsigned parent_id = comps.get<Parent>().id.value();
        auto parent_comps = components.by_id(parent_id);
        if (std::ranges::contains(FLIP_ON_SYNC_WITH_PARENT, entity))
        {
            const bool is_parent_flipped = parent_comps.get<Flags>().is_enabled(Flags::FLIPPED);
            auto& flipped = comps.get<Flags>();
            flipped.set(Flags::FLIPPED, is_parent_flipped);
            if (entity == Entity::Melee)
            {
                auto& tform = comps.get<Tform>();
                auto& combat = comps.get<Combat>();
                const auto details = std::get<MeleeDetails>(entities::attack_details(Attack::Melee).details);
                combat.hitbox = se::BBox{ rl::Rectangle{ tform.pos, details.size },
                                          (flipped.is_enabled(Flags::FLIPPED) ? MELEE_OFFSET_FLIPPED : MELEE_OFFSET) };
            }
        }

        auto& pos = comps.get<Tform>().pos;
        const auto parent_pos = parent_comps.get<Tform>().pos;
        pos = parent_pos;
        comps.get<Tform>().cbox.sync(pos);
        comps.get<Combat>().hitbox.sync(pos);
        slog::log(slog::TRC, "Child pos: ({}, {})", pos.x, pos.y);
        slog::log(slog::TRC, "Parent pos: ({}, {})", parent_pos.x, parent_pos.y);
    }
}

void Game::update_invuln_times()
{
    for (const auto& [id, combat] : components.vec<Combat>() | std::views::enumerate)
    {
        auto& invuln_time = combat.invuln_time;
        if (invuln_time > 0.0)
        {
            invuln_time -= dt();
        }
    }
}

void Game::render_ui()
{
    if (screen != std::nullopt)
    {
        screen.value().render();
    }
}

void Game::check_pause_game()
{
    if (inputs.pause)
    {
        toggle_pause();
    }
}

void Game::ui_click_action()
{
    if (inputs.click && screen != std::nullopt)
    {
        screen->click_action(rl::Mouse::GetPosition());
    }
}

namespace
{
void resolve_tile_collisions(Game& game, const unsigned id, const se::BBox prev_cbox)
{
    auto comps = game.components.by_id(id);
    const auto entity = game.entities.entities()[id];
    auto& transform = comps.get<Tform>();
    auto& cbox = transform.cbox;
    for (const auto tile_cbox : game.world.cboxes())
    {
        if (!cbox.collides(tile_cbox))
        {
            continue;
        }

        if (std::ranges::contains(DESTROY_ON_COLLISION, entity))
        {
            game.entities.queue_destroy(id);
            continue;
        }

        const auto tile_rcbox = std::get<rl::Rectangle>(tile_cbox.bbox());
        const float x_adjust = sl::match(
            cbox.bbox(),
            [tile_rcbox](const rl::Rectangle cbox)
            { return tile_rcbox.x - cbox.x + (tile_rcbox.x > cbox.x ? -cbox.width : tile_rcbox.width); },
            [tile_rcbox](const smath::Circle cbox)
            { return tile_rcbox.x - cbox.pos.x + (cbox.pos.x > tile_rcbox.x ? cbox.radius : -cbox.radius); },
            [tile_rcbox, prev_cbox](const auto) { return 0.0F; });
        if (tile_cbox.y_overlaps(prev_cbox) && tile_cbox.x_overlaps(cbox))
        {
            transform.pos.x += x_adjust;
            cbox.sync(transform.pos);
        }

        const float y_adjust = sl::match(
            cbox.bbox(),
            [tile_rcbox](const rl::Rectangle cbox)
            { return tile_rcbox.y - cbox.y + (tile_rcbox.y > cbox.y ? -cbox.height : tile_rcbox.height); },
            [tile_rcbox](const smath::Circle cbox)
            { return tile_rcbox.y - cbox.pos.y + (cbox.pos.y > tile_rcbox.y ? cbox.radius : -cbox.radius); },
            [tile_rcbox, prev_cbox](const auto) { return 0.0F; });
        if (tile_cbox.x_overlaps(prev_cbox) && tile_cbox.y_overlaps(cbox))
        {
            transform.pos.y += y_adjust;
            cbox.sync(transform.pos);
        }

        if (id == game.player_id)
        {
            slog::log(slog::TRC, "Player pos adjusted by ({}, {})", x_adjust, y_adjust);
        }
    }
}

void draw_sprite(Game& game, const unsigned id)
{
    const auto transform = game.components.get<Tform>(id);
    const auto flags = game.components.get<Flags>(id);
    auto& sprites = game.sprites;
    const auto legs = sprites.sprite<SpriteLegs>(id);
    const auto legs_frame = sprites.current_frame<SpriteLegs>(id);
    const auto y_offset = (legs_frame % 2 == 0 ? 0.0 : sprites::alternate_frame_y_offset(legs));
    const auto offset = rl::Vector2(0.0, static_cast<float>(y_offset));
    const auto flipped = flags.is_enabled(Flags::FLIPPED);
    sprites.draw_part<SpriteBase>(game.texture_sheet, transform.pos + offset, id, game.dt(), flipped);
    sprites.draw_part<SpriteHead>(game.texture_sheet, transform.pos + offset, id, game.dt(), flipped);
    sprites.draw_part<SpriteArms>(game.texture_sheet, transform.pos + offset, id, game.dt(), flipped);
    sprites.draw_part<SpriteLegs>(game.texture_sheet, transform.pos, id, game.dt(), flipped);
    sprites.draw_part<SpriteExtra>(game.texture_sheet, transform.pos + offset, id, game.dt(), flipped);
}
} // namespace

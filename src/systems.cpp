#include "components.hpp"
#include "entities.hpp"
#include "game.hpp"
#include "settings.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <optional>
#include <ranges>
#include <utility>

#define SHOW_CBOXES
#define SHOW_HITBOXES
#undef SHOW_CBOXES
#undef SHOW_HITBOXES

static constexpr auto GRAVITY_AFFECTED_ENTITIES = {
    Entity::Player,
    Entity::Enemy,
};
static constexpr auto DESTROY_ON_COLLISION = {
    Entity::Projectile,
};
static constexpr auto DAMAGING_ENTITIES = {
    Entity::Projectile,
    Entity::Melee,
};

inline constexpr auto HEALTH_BAR_SIZE = SimpleVec2(32.0, 4.0);

inline constexpr float PLAYER_SPEED = 100.0;
inline constexpr float JUMP_SPEED = 450.0;
inline constexpr float GRAVITY_ACCELERATION = 1000.0;
inline constexpr float MAX_FALL_SPEED = 500.0;
inline constexpr float HEALTH_BAR_Y_OFFSET = 8.0;

namespace
{
int damage(Entity entity);
} // namespace

void Game::poll_inputs()
{
    m_inputs.left = RKeyboard::IsKeyDown(KEY_A);
    m_inputs.right = RKeyboard::IsKeyDown(KEY_D);
    m_inputs.up = RKeyboard::IsKeyPressed(KEY_W) || RKeyboard::IsKeyPressed(KEY_SPACE);
    m_inputs.attack = RMouse::IsButtonPressed(MOUSE_LEFT_BUTTON);
    m_inputs.spawn_enemy = RKeyboard::IsKeyPressed(KEY_P);
}

void Game::render()
{
    const auto player_pos = m_components.transforms[PLAYER_ID].pos;
    m_camera.SetTarget(player_pos + RVector2(SPRITE_SIZE, SPRITE_SIZE) / 2);

    m_camera.BeginMode();

    for (const auto [id, entity] : m_entities.entities() | std::views::enumerate | std::views::reverse)
    {
        if (entity == std::nullopt)
        {
            continue;
        }

        const auto transform = m_components.transforms[id];
        auto& sprite = m_components.sprites[id];
        sprite.check_update_frames(dt());
        sprite.lookup_set_movement_parts(entity.value(), transform.vel);
        sprite.draw(m_texture_sheet, transform, m_components.flags[id][flag::FLIPPED]);

        const auto health = m_components.healths[id];
        if (health.max != std::nullopt && health.current != health.max)
        {
            const auto pos = m_components.transforms[id].pos - RVector2(0.0, HEALTH_BAR_Y_OFFSET);
            const float current_bar_width = HEALTH_BAR_SIZE.x * health.percentage();
            RRectangle(pos, HEALTH_BAR_SIZE).Draw(RED);
            RRectangle(pos, RVector2(current_bar_width, HEALTH_BAR_SIZE.y)).Draw(GREEN);
        }

#ifdef SHOW_HITBOXES
        std::visit(
            overloaded{
                [](const RRectangle bbox) { bbox.DrawLines(GREEN); },
                [](const Circle bbox) { bbox.draw_lines(GREEN); },
            },
            m_components.hitboxes[id].bounding_box());
#endif

#ifdef SHOW_CBOXES
        std::visit(
            overloaded{
                [](const RRectangle bbox) { bbox.DrawLines(RED); },
                [](const Circle bbox) { bbox.draw_lines(RED); },
            },
            m_components.collision_boxes[id].bounding_box());
#endif
    }

    m_camera.EndMode();
}

void Game::set_player_vel()
{
    auto& player_vel = m_components.transforms[PLAYER_ID].vel;
    player_vel.x = 0.0;
    if (m_inputs.right)
    {
        player_vel.x += PLAYER_SPEED;
    }
    if (m_inputs.left)
    {
        player_vel.x -= PLAYER_SPEED;
    }

    if (m_inputs.up && m_components.flags[PLAYER_ID][flag::GROUNDED])
    {
        player_vel.y = -JUMP_SPEED;
        m_components.flags[PLAYER_ID][flag::GROUNDED] = false;
    }
}

void Game::move_entities()
{
    for (const auto [id, entity] : m_entities.entities() | std::views::enumerate)
    {
        if (entity == std::nullopt || entity == Entity::Tile)
        {
            continue;
        }

        auto& transform = m_components.transforms[id];
        if (std::ranges::contains(GRAVITY_AFFECTED_ENTITIES, entity))
        {
            transform.vel.y = std::min(MAX_FALL_SPEED, transform.vel.y + GRAVITY_ACCELERATION * dt());
            m_components.flags[id][flag::GROUNDED] = false;
        }

        transform.pos += transform.vel * dt();
        if (transform.vel.x != 0.0)
        {
            m_components.flags[id][flag::FLIPPED] = transform.vel.x < 0;
        }

        auto& cbox = m_components.collision_boxes[id];
        const auto prev_cbox = cbox;
        const auto flipped = m_components.flags[id][flag::FLIPPED];
        cbox.sync(transform, flipped);
        for (const unsigned tile_id : m_entities.entity_ids(Entity::Tile)) // resolve tile collisions
        {
            const auto tile_cbox = m_components.collision_boxes[tile_id];
            if (!cbox.collides(tile_cbox))
            {
                continue;
            }

            if (std::ranges::contains(DESTROY_ON_COLLISION, entity))
            {
                m_entities.queue_destroy(id);
                continue;
            }

            const auto tile_rcbox = std::get<RRectangle>(tile_cbox.bounding_box());
            const float x_adjust = std::visit(
                overloaded{
                    [tile_rcbox](const RRectangle cbox)
                    { return tile_rcbox.x - cbox.x + (tile_rcbox.x > cbox.x ? -cbox.width : tile_rcbox.width); },
                    [tile_rcbox](const Circle cbox)
                    { return tile_rcbox.x - cbox.pos.x + (cbox.pos.x > tile_rcbox.x ? cbox.radius : -cbox.radius); },
                },
                cbox.bounding_box());
            if (tile_cbox.y_overlaps(prev_cbox) && tile_cbox.x_overlaps(cbox))
            {
                transform.pos.x += x_adjust;
                cbox.sync(transform, flipped);
            }

            const float y_adjust = std::visit(
                overloaded{
                    [tile_rcbox](const RRectangle cbox)
                    { return tile_rcbox.y - cbox.y + (tile_rcbox.y > cbox.y ? -cbox.height : tile_rcbox.height); },
                    [tile_rcbox](const Circle cbox)
                    { return tile_rcbox.y - cbox.pos.y + (cbox.pos.y > tile_rcbox.y ? cbox.radius : -cbox.radius); },
                },
                cbox.bounding_box());
            if (tile_cbox.x_overlaps(prev_cbox) && tile_cbox.y_overlaps(cbox))
            {
                transform.pos.y += y_adjust;
                cbox.sync(transform, flipped);

                if (y_adjust != 0.0)
                {
                    transform.vel.y = 0.0;
                }

                if (y_adjust < 0.0)
                {
                    m_components.flags[id][flag::GROUNDED] = true;
                }
            }
        }

        m_components.hitboxes[id].sync(transform, flipped);
    }
}

void Game::destroy_entities()
{
    for (const unsigned id : m_entities.to_destroy())
    {
        destroy_entity(id);
    }

    m_entities.clear_to_destroy();
}

void Game::player_attack()
{
    if (!m_inputs.attack)
    {
        return;
    }

    m_components.sprites[PLAYER_ID].arms.set(SpriteArms::PlayerAttack);
    spawn_attack(Attack::Melee, PLAYER_ID);
}

void Game::update_lifespans()
{
    for (const auto [id, entity] : m_entities.entities() | std::views::enumerate)
    {
        auto& lifespan = m_components.lifespans[id];
        if (entity == std::nullopt || lifespan == std::nullopt)
        {
            continue;
        }

        lifespan.value() -= dt();
        if (lifespan.value() < 0.0)
        {
            m_entities.queue_destroy(id);
        }
    }
}

void Game::damage_entities()
{
    for (const auto entity : DAMAGING_ENTITIES)
    {
        for (const unsigned id : m_entities.entity_ids(entity))
        {
            const auto projectile_bbox = m_components.hitboxes[id];
            for (const unsigned enemy_id : m_entities.entity_ids(Entity::Enemy))
            {
                const auto enemy_bbox = m_components.hitboxes[enemy_id];
                if (!enemy_bbox.collides(projectile_bbox))
                {
                    continue;
                }

                int& current_health = m_components.healths[enemy_id].current;
                current_health -= damage(entity);
                m_entities.queue_destroy(id);
                if (current_health <= 0)
                {
                    m_entities.queue_destroy(enemy_id);
                }

                break;
            }
        }
    }
}

void Game::sync_children()
{
    for (const auto [id, entity] : m_entities.entities() | std::views::enumerate)
    {
        if (m_components.parents[id] == std::nullopt)
        {
            continue;
        }

        const unsigned parent_id = m_components.parents[id].value();
        m_components.flags[id][flag::FLIPPED] = m_components.flags[PLAYER_ID][flag::FLIPPED];
        const bool flipped = m_components.flags[id][flag::FLIPPED];
        auto& transform = m_components.transforms[id];
        transform.pos = m_components.transforms[parent_id].pos;
        m_components.collision_boxes[id].sync(transform, flipped);
        m_components.hitboxes[id].sync(transform, flipped);
    }
}

namespace
{
int damage(Entity entity)
{
    assert(std::ranges::contains(DAMAGING_ENTITIES, entity));

    switch (entity)
    { // NOLINTBEGIN(*magic-numbers)
    case Entity::Projectile:
        return 25;
    case Entity::Melee:
        return 34;
    case Entity::Player:
    case Entity::Tile:
    case Entity::Enemy:
        std::unreachable();
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
} // namespace

#include "components.hpp"
#include "entities.hpp"
#include "game.hpp"
#include "seblib.hpp"

#include <algorithm>
#include <cassert>
#include <optional>
#include <ranges>

#define SHOW_CBOXES
#undef SHOW_CBOXES
#define SHOW_HITBOXES
#undef SHOW_HITBOXES

namespace sl = seblib;

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
    Entity::DamageLine, Entity::Tile, Entity::Enemy, Entity::Player, Entity::Projectile,
};

inline constexpr auto HEALTH_BAR_SIZE = sl::SimpleVec2(32.0, 4.0);

inline constexpr float PLAYER_SPEED = 100.0;
inline constexpr float HEALTH_BAR_Y_OFFSET = 8.0;
inline constexpr float INVULN_TIME = 0.5;
inline constexpr float DAMAGE_LINE_THICKNESS = 1.33;

namespace
{
void resolve_tile_collisions(Game& game, unsigned id, BBox prev_cbox);
} // namespace

void Game::poll_inputs()
{
    m_inputs.left = RKeyboard::IsKeyDown(KEY_A);
    m_inputs.right = RKeyboard::IsKeyDown(KEY_D);
    m_inputs.up = RKeyboard::IsKeyDown(KEY_W);
    m_inputs.down = RKeyboard::IsKeyDown(KEY_S);
    m_inputs.click = RMouse::IsButtonPressed(MOUSE_LEFT_BUTTON);
    m_inputs.spawn_enemy = RKeyboard::IsKeyPressed(KEY_P);
    m_inputs.pause = RKeyboard::IsKeyPressed(KEY_ESCAPE);
}

void Game::render()
{
    const auto player_pos = m_components.transforms[m_player_id].pos;
    m_camera.SetTarget(player_pos + RVector2(SPRITE_SIZE, SPRITE_SIZE) / 2);
    m_camera.BeginMode();
    for (const auto entity : ENTITY_RENDER_ORDER)
    {
        for (const unsigned id : m_entities.entity_ids(entity))
        {
            if (entity == Entity::DamageLine)
            {
                const auto line = std::get<Line>(m_components.hitboxes[id].bounding_box());
                line.pos1.DrawLine(line.pos2, DAMAGE_LINE_THICKNESS, ::LIGHTGRAY);
                continue;
            }

            const auto transform = m_components.transforms[id];
            auto& sprite = m_components.sprites[id];
            sprite.check_update_frames(dt());
            sprite.lookup_set_movement_parts(entity, transform.vel);
            sprite.draw(m_texture_sheet, transform, m_components.flags[id][flag::FLIPPED]);

            const auto health = m_components.healths[id];
            if (health.max != std::nullopt && health.current != health.max)
            {
                const auto pos = m_components.transforms[id].pos - RVector2(0.0, HEALTH_BAR_Y_OFFSET);
                const float current_bar_width = HEALTH_BAR_SIZE.x * health.percentage();
                RRectangle(pos, HEALTH_BAR_SIZE).Draw(::RED);
                RRectangle(pos, RVector2(current_bar_width, HEALTH_BAR_SIZE.y)).Draw(::GREEN);
            }
        }
    }

#ifdef SHOW_CBOXES
    for (const auto entity : ENTITY_RENDER_ORDER)
    {
        for (const unsigned id : m_entities.entity_ids(entity))
        {
            sl::match(
                m_components.collision_boxes[id].bounding_box(),
                [](const RRectangle bbox) { bbox.DrawLines(::RED); },
                [](const Circle bbox) { bbox.draw_lines(::RED); },
                [](const Line bbox) { bbox.draw_line(::RED); });
        }
    }
#endif

#ifdef SHOW_HITBOXES
    for (const auto entity : ENTITY_RENDER_ORDER)
    {
        for (const unsigned id : m_entities.entity_ids(entity))
        {
            sl::match(
                m_components.hitboxes[id].bounding_box(),
                [](const RRectangle bbox) { bbox.DrawLines(::GREEN); },
                [](const Circle bbox) { bbox.draw_lines(::GREEN); },
                [](const Line bbox) { bbox.draw_line(::GREEN); });
        }
    }
#endif

    m_camera.EndMode();
}

void Game::set_player_vel()
{
    auto& player_vel = m_components.transforms[m_player_id].vel;
    player_vel.x = 0.0;
    player_vel.x += (m_inputs.right ? PLAYER_SPEED : 0.0F);
    player_vel.x -= (m_inputs.left ? PLAYER_SPEED : 0.0F);
    player_vel.y = 0.0;
    player_vel.y -= (m_inputs.up ? PLAYER_SPEED : 0.0F);
    player_vel.y += (m_inputs.down ? PLAYER_SPEED : 0.0F);
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
        transform.pos += transform.vel * dt();
        if (transform.vel.x != 0.0)
        {
            m_components.flags[id][flag::FLIPPED] = transform.vel.x < 0.0;
        }

        auto& cbox = m_components.collision_boxes[id];
        const auto prev_cbox = cbox;
        const auto flipped = m_components.flags[id][flag::FLIPPED];
        cbox.sync(transform, flipped);
        resolve_tile_collisions(*this, id, prev_cbox);
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
    auto& attack_cooldown = m_components.attack_cooldowns[m_player_id];
    if (attack_cooldown > 0.0)
    {
        attack_cooldown -= dt();
    }

    if (!m_inputs.click || attack_cooldown > 0.0)
    {
        return;
    }

    const auto attack = Attack::Sector;
    const auto attack_details = entities::attack_details(attack);
    m_components.sprites[m_player_id].arms.set(SpriteArms::PlayerAttack, attack_details.lifespan);
    m_components.attack_cooldowns[m_player_id] = attack_details.cooldown;
    spawn_attack(attack, m_player_id);
}

void Game::update_lifespans()
{
    for (const auto& [id, lifespan] : m_components.lifespans | std::views::enumerate)
    {
        if (lifespan == std::nullopt)
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
                float& invuln_time = m_components.invuln_times[enemy_id];
                if (invuln_time > 0.0 || !enemy_bbox.collides(projectile_bbox))
                {
                    continue;
                }

                int& enemy_current_health = m_components.healths[enemy_id].current;
                enemy_current_health -= (int)m_components.hit_damage[id];
                if (enemy_current_health <= 0)
                {
                    m_entities.queue_destroy(enemy_id);
                }

                if (entity == Entity::Projectile)
                {
                    m_entities.queue_destroy(id);
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
    for (const auto [id, entity] : m_entities.entities() | std::views::enumerate)
    {
        if (entity == std::nullopt || m_components.parents[id] == std::nullopt)
        {
            continue;
        }

        const unsigned parent_id = m_components.parents[id].value();
        if (std::ranges::contains(FLIP_ON_SYNC_WITH_PARENT, entity))
        {
            m_components.flags[id][flag::FLIPPED] = m_components.flags[m_player_id][flag::FLIPPED];
        }

        const bool flipped = m_components.flags[id][flag::FLIPPED];
        auto& transform = m_components.transforms[id];
        transform.pos = m_components.transforms[parent_id].pos;
        m_components.collision_boxes[id].sync(transform, flipped);
        m_components.hitboxes[id].sync(transform, flipped);
    }
}

void Game::update_invuln_times()
{
    for (const auto& [id, invuln_time] : m_components.invuln_times | std::views::enumerate)
    {
        if (invuln_time > 0.0)
        {
            invuln_time -= dt();
        }
    }
}

void Game::render_ui()
{
    if (m_screen != std::nullopt)
    {
        m_screen.value().render();
    }
}

void Game::check_pause_game()
{
    if (m_inputs.pause)
    {
        toggle_pause();
    }
}

void Game::ui_click_action()
{
    if (m_inputs.click && m_screen != std::nullopt)
    {
        m_screen->click_action(RMouse::GetPosition());
    }
}

namespace
{
void resolve_tile_collisions(Game& game, const unsigned id, const BBox prev_cbox)
{
    const auto entity = game.entities().entities()[id];
    auto& transform = game.components().transforms[id];
    auto& cbox = game.components().collision_boxes[id];
    const auto flipped = game.components().flags[id][flag::FLIPPED];
    for (const unsigned tile_id : game.entities().entity_ids(Entity::Tile))
    {
        const auto tile_cbox = game.components().collision_boxes[tile_id];
        if (!cbox.collides(tile_cbox))
        {
            continue;
        }

        if (std::ranges::contains(DESTROY_ON_COLLISION, entity))
        {
            game.entities().queue_destroy(id);
            continue;
        }

        const auto tile_rcbox = std::get<RRectangle>(tile_cbox.bounding_box());
        const float x_adjust = sl::match(
            cbox.bounding_box(),
            [tile_rcbox](const RRectangle cbox)
            { return tile_rcbox.x - cbox.x + (tile_rcbox.x > cbox.x ? -cbox.width : tile_rcbox.width); },
            [tile_rcbox](const Circle cbox)
            { return tile_rcbox.x - cbox.pos.x + (cbox.pos.x > tile_rcbox.x ? cbox.radius : -cbox.radius); },
            [tile_rcbox, prev_cbox](const auto) { return 0.0F; });
        if (tile_cbox.y_overlaps(prev_cbox) && tile_cbox.x_overlaps(cbox))
        {
            transform.pos.x += x_adjust;
            cbox.sync(transform, flipped);
        }

        const float y_adjust = sl::match(
            cbox.bounding_box(),
            [tile_rcbox](const RRectangle cbox)
            { return tile_rcbox.y - cbox.y + (tile_rcbox.y > cbox.y ? -cbox.height : tile_rcbox.height); },
            [tile_rcbox](const Circle cbox)
            { return tile_rcbox.y - cbox.pos.y + (cbox.pos.y > tile_rcbox.y ? cbox.radius : -cbox.radius); },
            [tile_rcbox, prev_cbox](const auto) { return 0.0F; });
        if (tile_cbox.x_overlaps(prev_cbox) && tile_cbox.y_overlaps(cbox))
        {
            transform.pos.y += y_adjust;
            cbox.sync(transform, flipped);
        }
    }
}
} // namespace

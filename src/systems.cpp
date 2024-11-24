#include "components.hpp"
#include "entities.hpp"
#include "game.hpp"

#include <algorithm>
#include <optional>
#include <ranges>

#define SHOW_BBOXES
#undef SHOW_BBOXES

#ifdef SHOW_BBOXES
#include "overloaded.hpp"
#endif

static constexpr auto GRAVITY_AFFECTED_ENTITIES = { Entity::Player, Entity::Enemy };

constexpr float PLAYER_SPEED = 100.0;
constexpr float JUMP_SPEED = 450.0;
constexpr float GRAVITY_ACCELERATION = 1000.0;
constexpr float MAX_FALL_SPEED = 2000.0;

constexpr int PROJECTILE_DAMAGE = 25;

void Game::poll_inputs()
{
    m_inputs.left = RKeyboard::IsKeyDown(KEY_A);
    m_inputs.right = RKeyboard::IsKeyDown(KEY_D);
    m_inputs.up = RKeyboard::IsKeyPressed(KEY_W) || RKeyboard::IsKeyPressed(KEY_SPACE);
    m_inputs.attack = RMouse::IsButtonPressed(MOUSE_LEFT_BUTTON);
    m_inputs.spawn_enemy = RKeyboard::IsKeyPressed(KEY_P);
}

void Game::render_sprites()
{
    const auto player_pos = m_component_manager.transforms[PLAYER_ID].pos;
    m_camera.SetTarget(player_pos + RVector2(TILE_SIZE, TILE_SIZE) / 2);

    m_camera.BeginMode();

    for (const auto [id, entity] : m_entity_manager.entities | std::views::enumerate | std::views::as_const) {
        if (entity == std::nullopt) {
            continue;
        }

        const auto transform = m_component_manager.transforms[id];
        auto& sprite = m_component_manager.sprites[id];
        if (transform.vel.x != 0) {
            sprite.flipped = transform.vel.x < 0;
        }

        const auto new_sprite = lookup_movement_sprite(entity.value(), transform.vel);
        if (new_sprite != std::nullopt) {
            sprite.set(new_sprite.value());
        }

        sprite.check_update_frame(dt());
        m_texture_sheet.Draw(sprite.sprite(), transform.pos);
        render_health_bars(id);

#ifdef SHOW_BBOXES
        std::visit(overloaded{
                       [](const RRectangle bbox) { bbox.DrawLines(RED); },
                       [](const Circle bbox) { bbox.draw_lines(RED); },
                   },
                   m_component_manager.bounding_boxes[id].bounding_box);
#endif
    }

    m_camera.EndMode();
}

void Game::set_player_vel()
{
    auto& player_vel = m_component_manager.transforms[PLAYER_ID].vel;
    player_vel.x = 0.0;
    if (m_inputs.right) {
        player_vel.x += PLAYER_SPEED;
    }
    if (m_inputs.left) {
        player_vel.x -= PLAYER_SPEED;
    }

    if (m_inputs.up && m_component_manager.grounded[PLAYER_ID].grounded) {
        player_vel.y = -JUMP_SPEED;
        m_component_manager.grounded[PLAYER_ID].grounded = false;
    }
}

void Game::move_entities()
{
    for (const auto [id, entity] : m_entity_manager.entities | std::views::enumerate | std::views::as_const) {
        if (entity == std::nullopt || entity == Entity::Tile) {
            continue;
        }

        auto& transform = m_component_manager.transforms[id];
        auto& bbox = m_component_manager.bounding_boxes[id];
        const float vel_y = transform.vel.y;

        if (std::ranges::contains(GRAVITY_AFFECTED_ENTITIES, entity)) {
            transform.vel.y = std::min(MAX_FALL_SPEED, vel_y + GRAVITY_ACCELERATION * dt());
            m_component_manager.grounded[id].grounded = false;
        }

        const auto prev_bbox = bbox;
        transform.pos += transform.vel * dt();
        bbox.sync(transform);
        resolve_tile_collisions(id, entity.value(), prev_bbox);
    }
}

void Game::destroy_entities()
{
    for (const unsigned id : m_entity_manager.entities_to_destroy) {
        auto& entity = m_entity_manager.entities[id];
        if (entity == std::nullopt) { // possible for an entity to be queued for destruction multiple times,
            continue;                 // leads to already being nullopt
        }

        auto& entity_ids = m_entity_manager.entity_ids[entity.value()];
        entity_ids.erase(std::ranges::find(entity_ids, id));
        entity = std::nullopt;

        m_component_manager.transforms[id].vel = RVector2(0.0, 0.0);
        m_component_manager.lifespans[id].current = std::nullopt;
        m_component_manager.health[id].max = std::nullopt;
    }

    m_entity_manager.entities_to_destroy.clear();
}

void Game::player_attack()
{
    if (!m_inputs.attack) {
        return;
    }

    spawn_projectile(m_component_manager.transforms[PLAYER_ID].pos);
}

void Game::update_lifespans()
{
    for (const auto [id, entity] : m_entity_manager.entities | std::views::enumerate | std::views::as_const) {
        auto& lifespan = m_component_manager.lifespans[id].current;
        if (entity == std::nullopt || lifespan == std::nullopt) {
            continue;
        }

        lifespan.value() -= dt();
        if (lifespan.value() < 0.0) {
            m_entity_manager.queue_destroy_entity(id);
        }
    }
}

void Game::check_projectiles_hit()
{
    for (const unsigned projectile_id : m_entity_manager.entity_ids[Entity::Projectile]) {
        const auto projectile_bbox = m_component_manager.bounding_boxes[projectile_id];
        for (const unsigned enemy_id : m_entity_manager.entity_ids[Entity::Enemy]) {
            const auto enemy_bbox = m_component_manager.bounding_boxes[enemy_id];
            if (!enemy_bbox.collides(projectile_bbox)) {
                continue;
            }

            int& current_health = m_component_manager.health[enemy_id].current;
            current_health -= PROJECTILE_DAMAGE;
            m_entity_manager.queue_destroy_entity(projectile_id);
            if (current_health <= 0) {
                m_entity_manager.queue_destroy_entity(enemy_id);
            }

            break;
        }
    }
}

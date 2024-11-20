#include "components.hpp"
#include "entities.hpp"
#include "game.hpp"

#include <algorithm>
#include <optional>

#define SHOW_BBOXES
#undef SHOW_BBOXES

#ifdef SHOW_BBOXES
#include "overloaded.hpp"
#endif

constexpr float PLAYER_SPEED = 100.0;
constexpr float JUMP_SPEED = 450.0;
constexpr float GRAVITY_ACCELERATION = 1000.0;
constexpr float MAX_FALL_SPEED = 2000.0;
constexpr float HEALTH_BAR_WIDTH = 32.0;
constexpr float HEALTH_BAR_HEIGHT = 4.0;
constexpr float HEALTH_BAR_Y_OFFSET = 8.0;

constexpr auto GRAVITY_AFFECTED_ENTITIES = { EntityType::Player, EntityType::Enemy };

constexpr int PROJECTILE_DAMAGE = 25;

void Game::poll_inputs()
{
    m_inputs.left = RKeyboard::IsKeyDown(KEY_A);
    m_inputs.right = RKeyboard::IsKeyDown(KEY_D);
    m_inputs.up = RKeyboard::IsKeyPressed(KEY_W) || RKeyboard::IsKeyPressed(KEY_SPACE);
    m_inputs.attack = RMouse::IsButtonPressed(MOUSE_LEFT_BUTTON);
}

void Game::render_sprites()
{
    const auto player_pos = m_component_manager.m_transforms[PLAYER_ID].pos;
    m_camera.SetTarget(player_pos + RVector2(TILE_SIZE, TILE_SIZE) / 2);

    m_camera.BeginMode();

    for (const auto entity : m_entity_manager.m_entities) {
        if (entity.type() == std::nullopt) {
            continue;
        }

        const unsigned id = entity.id();
        const auto tform = m_component_manager.m_transforms[id];
        auto& sprite = m_component_manager.m_sprites[id];
        if (tform.vel.x < 0) {
            sprite.flip();
        } else if (tform.vel.x > 0) {
            sprite.unflip();
        }

        m_texture_sheet.Draw(sprite.sprite, tform.pos);

        const auto health = m_component_manager.m_health[id];
        if (health.max != std::nullopt && health.current < health.max) {
            auto pos = tform.pos;
            pos.y -= HEALTH_BAR_Y_OFFSET;
            const auto full_bar = RRectangle(pos, RVector2(HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT));
            const float current_bar_width = HEALTH_BAR_WIDTH * health.percentage();
            const auto current_bar = RRectangle(pos, RVector2(current_bar_width, HEALTH_BAR_HEIGHT));
            full_bar.Draw(RED);
            current_bar.Draw(GREEN);
        }

#ifdef SHOW_BBOXES
        std::visit(overloaded{
                       [](RRectangle bbox) { bbox.DrawLines(RED); },
                       [](Circle bbox) { bbox.draw_lines(RED); },
                   },
                   m_component_manager.m_bounding_boxes[id].bounding_box);
#endif
    }

    m_camera.EndMode();
}

void Game::set_player_vel()
{
    auto& player_vel = m_component_manager.m_transforms[PLAYER_ID].vel;
    player_vel.x = 0.0;
    if (m_inputs.right) {
        player_vel.x += PLAYER_SPEED;
    }
    if (m_inputs.left) {
        player_vel.x -= PLAYER_SPEED;
    }

    if (m_inputs.up && m_component_manager.m_grounded[PLAYER_ID].grounded) {
        player_vel.y = -JUMP_SPEED;
        m_component_manager.m_grounded[PLAYER_ID].grounded = false;
    }
}

void Game::move_entities()
{
    for (const auto entity : m_entity_manager.m_entities) {
        if (entity.type() == std::nullopt || entity.type() == EntityType::Tile) {
            continue;
        }

        const unsigned id = entity.id();
        auto& transform = m_component_manager.m_transforms[id];
        auto& bbox = m_component_manager.m_bounding_boxes[id];
        const float vel_y = transform.vel.y;

        if (std::ranges::contains(GRAVITY_AFFECTED_ENTITIES, entity.type())) {
            transform.vel.y = std::min(MAX_FALL_SPEED, vel_y + GRAVITY_ACCELERATION * dt());
            m_component_manager.m_grounded[id].grounded = false;
        }

        const auto prev_bbox = bbox;
        transform.move(dt());
        bbox.sync(transform);
        resolve_tile_collisions(entity, prev_bbox);
    }
}

void Game::destroy_entities()
{
    for (const unsigned id : m_entity_manager.m_entities_to_destroy) {
        auto& entity = m_entity_manager.m_entities[id];
        if (entity.type() == std::nullopt) { // Possible for an entity to be queued for destruction multiple times,
            continue;                        // leads to type already being nullopt
        }

        auto& entity_ids = m_entity_manager.m_entity_ids[entity.type().value()];
        entity_ids.erase(std::ranges::find(entity_ids, id));
        entity.clear_type();

        m_component_manager.m_lifespans[id].current = std::nullopt;
        m_component_manager.m_health[id].max = std::nullopt;
    }

    m_entity_manager.m_entities_to_destroy.clear();
}

void Game::player_attack()
{
    if (!m_inputs.attack) {
        return;
    }

    spawn_projectile(m_component_manager.m_transforms[PLAYER_ID].pos);
}

void Game::update_lifespans()
{
    for (const auto entity : m_entity_manager.m_entities) {
        const unsigned id = entity.id();
        auto& lifespan = m_component_manager.m_lifespans[id].current;
        if (entity.type() == std::nullopt || lifespan == std::nullopt) {
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
    for (const unsigned projectile_id : m_entity_manager.m_entity_ids[EntityType::Projectile]) {
        const auto projectile_bbox = m_component_manager.m_bounding_boxes[projectile_id];
        for (const unsigned enemy_id : m_entity_manager.m_entity_ids[EntityType::Enemy]) {
            const auto enemy_bbox = m_component_manager.m_bounding_boxes[enemy_id];
            if (enemy_bbox.collides(projectile_bbox)) {
                int& current_health = m_component_manager.m_health[enemy_id].current;
                current_health -= PROJECTILE_DAMAGE;
                m_entity_manager.queue_destroy_entity(projectile_id);

                if (current_health <= 0) {
                    m_entity_manager.queue_destroy_entity(enemy_id);
                }
            }
        }
    }
}

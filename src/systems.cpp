#include "game.hpp"

#include <algorithm>

constexpr float PLAYER_SPEED = 100.0;
constexpr float JUMP_SPEED = 450.0;
constexpr float GRAVITY_ACCELERATION = 1000.0;
constexpr float MAX_FALL_SPEED = 2000.0;

constexpr auto GRAVITY_AFFECTED_ENTITIES = { EntityType::Player, EntityType::Enemy };

void Game::poll_inputs()
{
    m_inputs.left = RKeyboard::IsKeyDown(KEY_A);
    m_inputs.right = RKeyboard::IsKeyDown(KEY_D);
    m_inputs.up = RKeyboard::IsKeyPressed(KEY_W);
}

void Game::render_sprites()
{
    const auto player_pos = m_component_manager.m_transforms[PLAYER_ID].pos;
    m_camera.SetTarget(player_pos);

    m_camera.BeginMode();

    for (const auto entity : m_entity_manager.m_entities) {
        if (entity.type() == std::nullopt) {
            continue;
        }

        const unsigned id = entity.id();
        const auto entity_tform = m_component_manager.m_transforms[id];
        auto& sprite = m_component_manager.m_sprites[id];
        if (entity_tform.vel.x < 0) {
            sprite.flip();
        } else if (entity_tform.vel.x > 0) {
            sprite.unflip();
        }

        m_texture_sheet.Draw(sprite.sprite, entity_tform.pos);
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
    if ((m_inputs.left ^ m_inputs.right) == 0) {
        player_vel.x = 0.0;
    } else if (m_inputs.right) {
        player_vel.x = PLAYER_SPEED * dt();
    } else if (m_inputs.left) {
        player_vel.x = -PLAYER_SPEED * dt();
    }

    if (m_inputs.up && m_component_manager.m_grounded[PLAYER_ID].grounded) {
        player_vel.y = -JUMP_SPEED * dt();
        m_component_manager.m_grounded[PLAYER_ID].grounded = false;
    }
}

void Game::move_entities()
{
    for (auto entity : m_entity_manager.m_entities) {
        if (entity.type() == std::nullopt || entity.type() == EntityType::Tile) {
            continue;
        }

        const unsigned id = entity.id();
        auto& transform = m_component_manager.m_transforms[id];
        auto& bbox = m_component_manager.m_bounding_boxes[id];
        const float vel_y = transform.vel.y;

        if (std::ranges::contains(GRAVITY_AFFECTED_ENTITIES, entity.type())) {
            transform.vel.y = std::min(MAX_FALL_SPEED * dt(), vel_y + GRAVITY_ACCELERATION * dt() * dt());
            m_component_manager.m_grounded[id].grounded = false;
        }

        const auto prev_bbox = bbox;
        transform.move();
        bbox.sync(transform);
        resolve_collisions(entity, prev_bbox);
    }
}

void Game::destroy_entities()
{
    m_entity_manager.destroy_entities();
}

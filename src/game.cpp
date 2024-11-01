#include "game.hpp"

#include "entities.hpp"

#include <algorithm>
#include <cassert>
#include <span>

#define SHOW_BBOXES
#undef SHOW_BBOXES

constexpr unsigned TARGET_FPS = 60;
constexpr unsigned PLAYER_ID = 0;

constexpr float PLAYER_SPEED = 100.0;
constexpr float JUMP_SPEED = 7.5;
constexpr float GRAVITY_ACCELERATION = 15.0;
constexpr float MAX_FALL_SPEED = 10.0;
constexpr float PLAYER_BBOX_SIZE_X = 20.0;
constexpr float PLAYER_BBOX_SIZE_Y = 29.0;

constexpr auto GRAVITY_AFFECTED_ENTITIES = { EntityType::Player };

EntityManager::EntityManager()
{
    m_entities.reserve(MAX_ENTITIES);
    for (unsigned id = 0; id < MAX_ENTITIES; id++) {
        m_entities.push_back(Entity(id));
    }
}

void EntityManager::spawn_player()
{
    assert(m_entities[PLAYER_ID].m_type == std::nullopt);

    m_entities[PLAYER_ID].m_type = EntityType::Player;
}

unsigned EntityManager::spawn_entity(EntityType type)
{
    assert(type != EntityType::Player);

    unsigned id = 1;
    for (auto& entity : std::span(m_entities).subspan(1)) {
        if (entity.m_type != std::nullopt) {
            continue;
        }

        entity.m_type = type;
        id = entity.m_id;
        m_entity_ids[type].push_back(id);
        break;
    }

    assert(id != PLAYER_ID);

    return id;
}

void ComponentManager::set_player_components()
{
    m_transforms[PLAYER_ID].pos = RVector2(WINDOW_HALF_WIDTH, WINDOW_HALF_HEIGHT);
    m_transforms[PLAYER_ID].vel = RVector2(0.0, 0.0);
    m_sprites[PLAYER_ID].set_pos(RVector2(0.0, 0.0));
    m_bounding_boxes[PLAYER_ID].set_size(RVector2(PLAYER_BBOX_SIZE_X, PLAYER_BBOX_SIZE_Y));
    m_bounding_boxes[PLAYER_ID].sync(m_transforms[PLAYER_ID]);
}

void Game::poll_inputs()
{
    m_inputs.m_left = RKeyboard::IsKeyDown(KEY_A);
    m_inputs.m_right = RKeyboard::IsKeyDown(KEY_D);
    m_inputs.m_up = RKeyboard::IsKeyPressed(KEY_W);
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
        const auto bounding_box = m_component_manager.m_bounding_boxes[id].bounding_box;
        bounding_box.DrawLines(RED);
#endif
    }

    m_camera.EndMode();
}

void Game::set_player_vel()
{
    auto& player_vel = m_component_manager.m_transforms[PLAYER_ID].vel;
    if ((m_inputs.m_left ^ m_inputs.m_right) == 0) {
        player_vel.x = 0.0;
    } else if (m_inputs.m_right) {
        player_vel.x = PLAYER_SPEED * dt();
    } else if (m_inputs.m_left) {
        player_vel.x = -PLAYER_SPEED * dt();
    }

    if (m_inputs.m_up && m_component_manager.m_grounded[PLAYER_ID].grounded) {
        player_vel.y = -JUMP_SPEED;
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
            transform.vel.y = std::min(MAX_FALL_SPEED, vel_y + GRAVITY_ACCELERATION * dt());
            m_component_manager.m_grounded[id].grounded = false;
        }

        const auto prev_bbox = bbox;
        transform.move();
        bbox.sync(transform);
        correct_collisions(id, prev_bbox);
    }
}

void Game::spawn_player()
{
    m_entity_manager.spawn_player();
    m_component_manager.set_player_components();
}

void Game::spawn_tile(Tile tile, RVector2 pos)
{
    RVector2 sprite_pos;
    switch (tile) {
    case Tile::Brick:
        sprite_pos = RVector2(0.0, 32.0); // NOLINT
        break;
    }

    const unsigned id = m_entity_manager.spawn_entity(EntityType::Tile);
    auto& transform = m_component_manager.m_transforms[id];
    transform.pos = pos;
    m_component_manager.m_bounding_boxes[id].sync(transform);
    m_component_manager.m_sprites[id].set_pos(sprite_pos);
}

float Game::dt()
{
    return m_window.GetFrameTime();
}

void Game::correct_collisions(unsigned id, BBox prev_bbox)
{
    auto& bbox = m_component_manager.m_bounding_boxes[id];
    auto& transform = m_component_manager.m_transforms[id];

    for (const unsigned tile_id : m_entity_manager.m_entity_ids[EntityType::Tile]) {
        const auto tile_bbox = m_component_manager.m_bounding_boxes[tile_id];
        if (!bbox.collides(tile_bbox)) {
            continue;
        }

        float x_adjust = 0.0;
        if (prev_bbox.y_overlaps(tile_bbox) && bbox.x_overlaps(tile_bbox)) {
            x_adjust = tile_bbox.bounding_box.x > bbox.bounding_box.x
                           ? tile_bbox.bounding_box.x - bbox.bounding_box.x - bbox.bounding_box.width
                           : tile_bbox.bounding_box.x - bbox.bounding_box.x + tile_bbox.bounding_box.width;
        }

        transform.pos.x += x_adjust;
        bbox.sync(transform);

        float y_adjust = 0.0;
        if (prev_bbox.x_overlaps(tile_bbox) && bbox.y_overlaps(tile_bbox)) {
            y_adjust = tile_bbox.bounding_box.y > bbox.bounding_box.y
                           ? tile_bbox.bounding_box.y - bbox.bounding_box.y - bbox.bounding_box.height
                           : tile_bbox.bounding_box.y - bbox.bounding_box.y + tile_bbox.bounding_box.height;
        }

        if (y_adjust != 0.0) {
            transform.vel.y = 0.0;
        }

        if (y_adjust < 0.0) {
            m_component_manager.m_grounded[id].grounded = true;
        }

        transform.pos.y += y_adjust;
        bbox.sync(transform);
    }
}

void Game::run()
{
    m_window.SetTargetFPS(TARGET_FPS);
    // window.SetExitKey(KEY_NULL);

    spawn_player();

    for (float i = 0.0; i < 300.0; i += 32.0) {      // NOLINT
        spawn_tile(Tile::Brick, RVector2(480.0, i)); // NOLINT
    }
    for (float i = 0.0; i < 300.0; i += 32.0) {      // NOLINT
        spawn_tile(Tile::Brick, RVector2(320.0, i)); // NOLINT
    }
    for (float i = 0.0; i < 800.0; i += 32.0) {      // NOLINT
        spawn_tile(Tile::Brick, RVector2(i, 320.0)); // NOLINT
    }

    spawn_tile(Tile::Brick, RVector2(352.0, 256.0)); // NOLINT
    spawn_tile(Tile::Brick, RVector2(448.0, 224.0)); // NOLINT
    spawn_tile(Tile::Brick, RVector2(384.0, 128.0)); // NOLINT

    while (!m_window.ShouldClose()) {
        m_window.BeginDrawing();
        m_window.ClearBackground(SKYBLUE);

        poll_inputs();
        set_player_vel();
        move_entities();
        render_sprites();

        m_window.EndDrawing();
    }
}

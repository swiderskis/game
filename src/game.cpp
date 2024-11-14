#include "game.hpp"

#include "components.hpp"
#include "entities.hpp"
#include "overloaded.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <optional>
#include <span>

#define SHOW_BBOXES
#undef SHOW_BBOXES

constexpr unsigned TARGET_FPS = 60;

constexpr float PLAYER_BBOX_SIZE_X = 20.0;
constexpr float PLAYER_BBOX_SIZE_Y = 29.0;
constexpr float PROJECTILE_SPEED = 500.0;
constexpr float PROJECTILE_LIFESPAN = 0.3;

constexpr auto DESTROY_ON_COLLISION = { EntityType::Projectile };

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

void EntityManager::queue_destroy_entity(unsigned id)
{
    m_entities_to_destroy.push_back(id);
}

void EntityManager::destroy_entities()
{
    for (const auto id : m_entities_to_destroy) {
        m_entities[id].m_type = std::nullopt;
    }

    m_entities_to_destroy.clear();
}

void ComponentManager::set_player_components()
{
    m_transforms[PLAYER_ID].pos = Coordinates(0, 2);
    m_transforms[PLAYER_ID].vel = RVector2(0.0, 0.0);
    m_sprites[PLAYER_ID].set_pos(RVector2(0.0, 0.0));
    m_bounding_boxes[PLAYER_ID].set_size(RVector2(PLAYER_BBOX_SIZE_X, PLAYER_BBOX_SIZE_Y));
    m_bounding_boxes[PLAYER_ID].sync(m_transforms[PLAYER_ID]);
}

void ComponentManager::set_circular_bounding_box(unsigned id, RVector2 pos, float radius)
{
    m_bounding_boxes[id].bounding_box = Circle(pos, radius);
}

Coordinates::Coordinates(int x, int y) : m_pos(RVector2(static_cast<float>(x), -static_cast<float>(y)) * TILE_SIZE)
{
}

Coordinates::operator RVector2() const
{
    return m_pos;
}

void Game::spawn_player()
{
    m_entity_manager.spawn_player();
    m_component_manager.set_player_components();
}

void Game::spawn_tile(Tile tile, Coordinates coordinates)
{
    RVector2 sprite_pos;
    switch (tile) {
    case Tile::Brick:
        sprite_pos = RVector2(0.0, 32.0); // NOLINT
        break;
    }

    const unsigned id = m_entity_manager.spawn_entity(EntityType::Tile);
    auto& transform = m_component_manager.m_transforms[id];
    transform.pos = coordinates;
    m_component_manager.m_bounding_boxes[id].sync(transform);
    m_component_manager.m_sprites[id].set_pos(sprite_pos);
}

float Game::dt()
{
    return m_window.GetFrameTime();
}

void Game::resolve_collisions(Entity entity, BBox prev_bbox)
{
    const auto id = entity.id();
    auto& bbox_comp = m_component_manager.m_bounding_boxes[id];
    auto& transform = m_component_manager.m_transforms[id];

    for (const unsigned tile_id : m_entity_manager.m_entity_ids[EntityType::Tile]) {
        const auto tile_bbox_comp = m_component_manager.m_bounding_boxes[tile_id];
        if (!bbox_comp.collides(tile_bbox_comp)) {
            continue;
        }

        if (std::ranges::contains(DESTROY_ON_COLLISION, entity.type())) {
            m_entity_manager.queue_destroy_entity(id);
            continue;
        }

        const auto tile_bbox = std::get<RRectangle>(tile_bbox_comp.bounding_box);
        const auto calculate_x_adjust = std::visit(
            overloaded{
                [tile_bbox](RRectangle bbox) {
                    return tile_bbox.x - bbox.x + (tile_bbox.x > bbox.x ? -bbox.width : tile_bbox.width);
                },
                [tile_bbox](Circle bbox) {
                    return tile_bbox.x - bbox.pos.x + (bbox.pos.x > tile_bbox.x ? bbox.radius : -bbox.radius);
                },
            },
            bbox_comp.bounding_box);
        const auto calculate_y_adjust = std::visit(
            overloaded{
                [tile_bbox](RRectangle bbox) {
                    return tile_bbox.y - bbox.y + (tile_bbox.y > bbox.y ? -bbox.height : tile_bbox.height);
                },
                [tile_bbox](Circle bbox) {
                    return tile_bbox.y - bbox.pos.y + (bbox.pos.y > tile_bbox.y ? bbox.radius : -bbox.radius);
                },
            },
            bbox_comp.bounding_box);

        if (tile_bbox_comp.y_overlaps(prev_bbox) && tile_bbox_comp.x_overlaps(bbox_comp)) {
            const float x_adjust = calculate_x_adjust;
            transform.pos.x += x_adjust;
            bbox_comp.sync(transform);
        }

        if (tile_bbox_comp.x_overlaps(prev_bbox) && tile_bbox_comp.y_overlaps(bbox_comp)) {
            const float y_adjust = calculate_y_adjust;
            transform.pos.y += y_adjust;
            bbox_comp.sync(transform);

            if (y_adjust != 0.0) {
                transform.vel.y = 0.0;
            }

            if (y_adjust < 0.0) {
                m_component_manager.m_grounded[id].grounded = true;
            }
        }
    }
}

void Game::spawn_projectile(RVector2 pos)
{
    const unsigned id = m_entity_manager.spawn_entity(EntityType::Projectile);
    auto& transform = m_component_manager.m_transforms[id];
    transform.pos = m_component_manager.m_transforms[PLAYER_ID].pos;
    const auto diff = get_mouse_pos() - transform.pos;
    const auto angle = atan2(diff.y, diff.x);
    transform.vel = RVector2(cos(angle), sin(angle)) * PROJECTILE_SPEED;

    m_component_manager.m_sprites[id].set_pos(RVector2(0.0, 64.0)); // NOLINT
    m_component_manager.set_circular_bounding_box(id, pos, 4.0);    // NOLINT
    m_component_manager.m_lifespans[id].current_lifespan = PROJECTILE_LIFESPAN;
}

RVector2 Game::get_mouse_pos()
{
    return m_camera.GetScreenToWorld(RMouse::GetPosition()) - RVector2(TILE_SIZE, TILE_SIZE) / 2;
}

Game::Game()
{
    m_window.SetTargetFPS(TARGET_FPS);
    m_window.SetExitKey(KEY_NULL);

    for (int i = 0; i < 10; i++) {                   // NOLINT
        spawn_tile(Tile::Brick, Coordinates(-3, i)); // NOLINT
    }
    for (int i = -10; i < 10; i++) {                // NOLINT
        spawn_tile(Tile::Brick, Coordinates(i, 0)); // NOLINT
    }

    spawn_tile(Tile::Brick, Coordinates(-2, 7)); // NOLINT
    spawn_tile(Tile::Brick, Coordinates(1, 6));  // NOLINT
    spawn_tile(Tile::Brick, Coordinates(-1, 3)); // NOLINT

    spawn_player();
}

void Game::run()
{
    m_window.BeginDrawing();
    m_window.ClearBackground(SKYBLUE);

    poll_inputs();
    set_player_vel();
    player_attack();
    move_entities();
    update_lifespans();
    destroy_entities();
    render_sprites();

    m_window.EndDrawing();
}

RWindow& Game::window()
{
    return m_window;
}

#ifndef NDEBUG
extern "C" __declspec(dllexport) void run(Game& game)
{
    game.run();
}

extern "C" __declspec(dllexport) bool check_reload_lib()
{
    return RKeyboard::IsKeyPressed(KEY_R);
}
#endif

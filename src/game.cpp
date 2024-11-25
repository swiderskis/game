#include "game.hpp"

#include "components.hpp"
#include "entities.hpp"
#include "overloaded.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <optional>

static constexpr auto DESTROY_ON_COLLISION = { Entity::Projectile };

constexpr unsigned TARGET_FPS = 60;

constexpr float PLAYER_BBOX_SIZE_X = 20.0;
constexpr float PLAYER_BBOX_SIZE_Y = 29.0;
constexpr float PROJECTILE_SPEED = 500.0;
constexpr float PROJECTILE_LIFESPAN = 0.3;
constexpr float ENEMY_BBOX_SIZE_X = 30.0;
constexpr float ENEMY_BBOX_SIZE_Y = 24.0;
constexpr float HEALTH_BAR_WIDTH = 32.0;
constexpr float HEALTH_BAR_HEIGHT = 4.0;
constexpr float HEALTH_BAR_Y_OFFSET = 8.0;

constexpr int PLAYER_HEALTH = 100;
constexpr int ENEMY_HEALTH = 100;

Coordinates::Coordinates(const int x, const int y) : m_pos(RVector2((float)x, (float)-y) * TILE_SIZE)
{
}

Coordinates::operator RVector2() const
{
    return m_pos;
}

void Game::spawn_player()
{
    unsigned id = m_entity_manager.spawn_entity(Entity::Player);
    auto& transform = m_component_manager.transforms[id];
    transform.pos = Coordinates(0, 2);
    transform.vel = RVector2(0.0, 0.0);

    m_component_manager.sprites[id].set(SpriteType::PlayerIdle);
    m_component_manager.bounding_boxes[id].set(transform, RVector2(PLAYER_BBOX_SIZE_X, PLAYER_BBOX_SIZE_Y));
    m_component_manager.health[id].set_health(PLAYER_HEALTH);
}

void Game::spawn_tile(const Tile tile, const RVector2 pos)
{
    const unsigned id = m_entity_manager.spawn_entity(Entity::Tile);
    auto& transform = m_component_manager.transforms[id];
    transform.pos = pos;
    m_component_manager.bounding_boxes[id].sync(transform);

    switch (tile) {
    case Tile::Brick:
        m_component_manager.sprites[id].set(SpriteType::TileBrick);
        break;
    }
}

float Game::dt() const
{
    return m_window.GetFrameTime();
}

void Game::resolve_tile_collisions(const unsigned id, const Entity entity, const BBox prev_bbox)
{
    auto& bbox_comp = m_component_manager.bounding_boxes[id];
    auto& transform = m_component_manager.transforms[id];

    for (const unsigned tile_id : m_entity_manager.entity_ids[Entity::Tile]) {
        const auto tile_bbox_comp = m_component_manager.bounding_boxes[tile_id];
        if (!bbox_comp.collides(tile_bbox_comp)) {
            continue;
        }

        if (std::ranges::contains(DESTROY_ON_COLLISION, entity)) {
            m_entity_manager.queue_destroy_entity(id);
            continue;
        }

        const auto tile_bbox = std::get<RRectangle>(tile_bbox_comp.bounding_box());
        const float x_adjust = std::visit(
            overloaded{
                [tile_bbox](const RRectangle bbox) {
                    return tile_bbox.x - bbox.x + (tile_bbox.x > bbox.x ? -bbox.width : tile_bbox.width);
                },
                [tile_bbox](const Circle bbox) {
                    return tile_bbox.x - bbox.pos.x + (bbox.pos.x > tile_bbox.x ? bbox.radius : -bbox.radius);
                },
            },
            bbox_comp.bounding_box());
        if (tile_bbox_comp.y_overlaps(prev_bbox) && tile_bbox_comp.x_overlaps(bbox_comp)) {
            transform.pos.x += x_adjust;
            bbox_comp.sync(transform);
        }

        const float y_adjust = std::visit(
            overloaded{
                [tile_bbox](const RRectangle bbox) {
                    return tile_bbox.y - bbox.y + (tile_bbox.y > bbox.y ? -bbox.height : tile_bbox.height);
                },
                [tile_bbox](const Circle bbox) {
                    return tile_bbox.y - bbox.pos.y + (bbox.pos.y > tile_bbox.y ? bbox.radius : -bbox.radius);
                },
            },
            bbox_comp.bounding_box());
        if (tile_bbox_comp.x_overlaps(prev_bbox) && tile_bbox_comp.y_overlaps(bbox_comp)) {
            transform.pos.y += y_adjust;
            bbox_comp.sync(transform);

            if (y_adjust != 0.0) {
                transform.vel.y = 0.0;
            }

            if (y_adjust < 0.0) {
                m_component_manager.grounded[id].grounded = true;
            }
        }
    }
}

void Game::spawn_projectile(const RVector2 pos)
{
    const unsigned id = m_entity_manager.spawn_entity(Entity::Projectile);
    auto& transform = m_component_manager.transforms[id];
    transform.pos = pos;
    const auto diff = get_mouse_pos() - transform.pos;
    const float angle = atan2(diff.y, diff.x);
    transform.vel = RVector2(cos(angle), sin(angle)) * PROJECTILE_SPEED;

    m_component_manager.sprites[id].set(SpriteType::Projectile);
    m_component_manager.bounding_boxes[id].set(transform, 4); // NOLINT
    m_component_manager.lifespans[id].current = PROJECTILE_LIFESPAN;
}

RVector2 Game::get_mouse_pos() const
{
    return m_camera.GetScreenToWorld(RMouse::GetPosition()) - RVector2(TILE_SIZE, TILE_SIZE) / 2;
}

void Game::spawn_enemy(const RVector2 pos)
{
    const unsigned id = m_entity_manager.spawn_entity(Entity::Enemy);
    auto& transform = m_component_manager.transforms[id];
    transform.pos = pos;
    m_component_manager.sprites[id].set(SpriteType::Enemy);
    m_component_manager.bounding_boxes[id].set(transform, RVector2(ENEMY_BBOX_SIZE_X, ENEMY_BBOX_SIZE_Y));
    m_component_manager.health[id].set_health(ENEMY_HEALTH);
}

void Game::render_health_bars(unsigned id)
{
    const auto health = m_component_manager.health[id];
    if (health.max == std::nullopt || health.current == health.max) {
        return;
    }

    const auto pos = m_component_manager.transforms[id].pos - RVector2(0.0, HEALTH_BAR_Y_OFFSET);
    const auto full_bar = RRectangle(pos, RVector2(HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT));
    const float current_bar_width = HEALTH_BAR_WIDTH * health.percentage();
    const auto current_bar = RRectangle(pos, RVector2(current_bar_width, HEALTH_BAR_HEIGHT));
    full_bar.Draw(RED);
    current_bar.Draw(GREEN);
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
    spawn_enemy(Coordinates(2, 2));
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
    check_projectiles_hit();
    destroy_entities();
    render_sprites();

    if (m_inputs.spawn_enemy) {
        spawn_enemy(Coordinates(2, 2));
    }

    m_window.EndDrawing();
}

RWindow& Game::window()
{
    return m_window;
}

#ifndef NDEBUG
#include <iostream>

void Game::reload_texture_sheet()
{
    m_texture_sheet.Unload();
    m_texture_sheet.Load(TEXTURE_SHEET);
    std::cout << "Texture sheet reloaded\n";
}

extern "C" {
__declspec(dllexport) void run(Game& game)
{
    game.run();
}

__declspec(dllexport) bool check_reload_lib()
{
    return RKeyboard::IsKeyPressed(KEY_R);
}
}
#endif

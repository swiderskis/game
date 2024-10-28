#include "game.hpp"

#include <cassert>

constexpr int TARGET_FPS = 60;

constexpr float WINDOW_HALF_WIDTH = WINDOW_WIDTH / 2.0;
constexpr float WINDOW_HALF_HEIGHT = WINDOW_HEIGHT / 2.0;
constexpr float SPRITE_DEFAULT_SIZE = 32.0;
constexpr float SIN45 = 0.70710678119; // cmath is not constexpr yet, cannot use sin(45) directly :(
constexpr float PLAYER_SPEED = 100.0 / TARGET_FPS;
constexpr float PLAYER_SPEED_DIAG = PLAYER_SPEED * SIN45;

constexpr size_t MAX_ENTITIES = 1024;
constexpr size_t PLAYER_ID = 0;

Entity::Entity(size_t id) : m_id(id), m_type(std::nullopt) {};

Entity::Entity(size_t id, EntityType type) : m_id(id), m_type(type) {};

size_t Entity::id() const
{
    return m_id;
}

std::optional<EntityType> Entity::type() const
{
    return m_type;
}

EntityManager::EntityManager()
{
    m_entities.reserve(MAX_ENTITIES);
    for (size_t id = 0; id < MAX_ENTITIES; id++) {
        m_entities.push_back(Entity(id));
    }
}

void EntityManager::spawn_player()
{
    if (m_entities[PLAYER_ID].m_type == EntityType::Player) {
        return;
    }

    m_entities[PLAYER_ID].m_type = EntityType::Player;
}

Sprite::Sprite(RVector2 pos) : pos(pos), size(RVector2(SPRITE_DEFAULT_SIZE, SPRITE_DEFAULT_SIZE)) {};

Sprite::Sprite(RVector2 pos, RVector2 size) : pos(pos), size(size) {};

ComponentManager::ComponentManager()
{
    m_transforms.reserve(MAX_ENTITIES);
    m_sprites.reserve(MAX_ENTITIES);
}

void ComponentManager::set_player_components()
{
    m_transforms[PLAYER_ID].pos = RVector2(WINDOW_HALF_WIDTH, WINDOW_HALF_HEIGHT);
    m_transforms[PLAYER_ID].vel = RVector2(0.0, 0.0);
    m_sprites[PLAYER_ID] = Sprite(RVector2(0.0, 0.0));
}

void Game::poll_inputs()
{
    m_inputs.m_up = RKeyboard::IsKeyDown(KEY_W);
    m_inputs.m_down = RKeyboard::IsKeyDown(KEY_S);
    m_inputs.m_left = RKeyboard::IsKeyDown(KEY_A);
    m_inputs.m_right = RKeyboard::IsKeyDown(KEY_D);
}

void Game::render_sprites()
{
    for (auto entity : m_entity_manager.m_entities) {
        if (entity.type() == std::nullopt) {
            continue;
        }

        const size_t id = entity.id();
        const auto texture_pos = m_component_manager.m_sprites[id].pos;
        const auto size = m_component_manager.m_sprites[id].size;
        const auto entity_pos = m_component_manager.m_transforms[id].pos - size / 2;

        m_texture_sheet.Draw(RRectangle(texture_pos, size), entity_pos);
    }
}

void Game::spawn_entity(EntityType type)
{
    switch (type) {
    case EntityType::Player:
        m_entity_manager.spawn_player();
        m_component_manager.set_player_components();
        break;
    default:
        break;
    }
}

void Game::set_player_vel()
{
    const bool vertical_movement = m_inputs.m_up ^ m_inputs.m_down;
    const bool horizontal_movement = m_inputs.m_left ^ m_inputs.m_right;
    const bool diagonal_movement = vertical_movement && horizontal_movement;
    const float player_speed = diagonal_movement ? PLAYER_SPEED_DIAG : PLAYER_SPEED;
    RVector2 player_vel;

    if (m_inputs.m_right) {
        player_vel.x += player_speed;
    }
    if (m_inputs.m_left) {
        player_vel.x -= player_speed;
    }
    if (m_inputs.m_down) {
        player_vel.y += player_speed;
    }
    if (m_inputs.m_up) {
        player_vel.y -= player_speed;
    }

    m_component_manager.m_transforms[PLAYER_ID].vel = player_vel;
}

void Game::move_entities()
{
    for (auto entity : m_entity_manager.m_entities) {
        if (entity.type() == std::nullopt) {
            continue;
        }

        m_component_manager.m_transforms[entity.id()].pos += m_component_manager.m_transforms[entity.id()].vel;
    }
}

void Game::run()
{
    m_window.SetTargetFPS(TARGET_FPS);
    // window.SetExitKey(KEY_NULL);

    spawn_entity(EntityType::Player);

    while (!m_window.ShouldClose()) {
        m_window.BeginDrawing();
        m_window.ClearBackground(GRAY);

        poll_inputs();
        set_player_vel();
        move_entities();
        render_sprites();

        m_window.EndDrawing();
    }
}

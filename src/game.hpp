#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <cstddef>
#include <optional>
#include <vector>

constexpr auto WINDOW_TITLE = "Game Title";
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 450;

const auto TEXTURE_SHEET = "assets/pingwin.png";

enum class EntityType {
    Player
};

class Entity
{
    size_t m_id;
    std::optional<EntityType> m_type;

    explicit Entity(size_t id);
    Entity(size_t id, EntityType type);

    friend class EntityManager;

public:
    Entity() = delete;

    [[nodiscard]] size_t id() const;
    [[nodiscard]] std::optional<EntityType> type() const;
};

class EntityManager
{
    std::vector<Entity> m_entities;
    size_t m_min_empty_id = 1;
    size_t m_max_occupied_id = 0;

    EntityManager(); // NOLINT

    void spawn_player();
    void spawn_entity(EntityType type);

    friend class Game;
};

struct Tform {
    RVector2 pos;
    RVector2 vel;

private:
    Tform() = default;

    friend class ComponentManager;
};

struct Sprite {
    RVector2 pos;
    RVector2 size;

    Sprite() = delete;
    explicit Sprite(RVector2 pos);
    Sprite(RVector2 pos, RVector2 size);

private:
    friend class ComponentManager;
};

class ComponentManager
{
    std::vector<Tform> m_transforms;
    std::vector<Sprite> m_sprites;

    ComponentManager(); // NOLINT

    void set_player_components();
    void set_entity_components(EntityType type);

    friend class Game;
};

struct Inputs {
    bool m_up = false;
    bool m_down = false;
    bool m_left = false;
    bool m_right = false;

private:
    Inputs() = default;

    friend class Game;
};

class Game
{
    EntityManager m_entity_manager;
    ComponentManager m_component_manager;
    Inputs m_inputs;
    RWindow m_window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
    RTexture m_texture_sheet{ TEXTURE_SHEET };

    void poll_inputs();
    void render_sprites();
    void spawn_entity(EntityType type);
    void set_player_vel();
    void move_entities();

public:
    void run();
};

#endif

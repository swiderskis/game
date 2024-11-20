#ifndef GAME_HPP_
#define GAME_HPP_

#include "components.hpp"
#include "entities.hpp"
#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <unordered_map>
#include <vector>

constexpr auto WINDOW_TITLE = "Game Title";
constexpr auto TEXTURE_SHEET = "assets/texture-sheet.png";

constexpr unsigned WINDOW_WIDTH = 800;
constexpr unsigned WINDOW_HEIGHT = 450;
constexpr unsigned WINDOW_HALF_WIDTH = WINDOW_WIDTH / 2;
constexpr unsigned WINDOW_HALF_HEIGHT = WINDOW_HEIGHT / 2;
constexpr unsigned MAX_ENTITIES = 1024;
constexpr unsigned PLAYER_ID = 0;

constexpr float CAMERA_ZOOM = 2.0;

class EntityManager
{
    std::vector<Entity> m_entities;
    std::unordered_map<EntityType, std::vector<unsigned>> m_entity_ids;
    std::vector<unsigned> m_entities_to_destroy;

    EntityManager();

    [[nodiscard]] unsigned spawn_entity(const EntityType type);
    void queue_destroy_entity(const unsigned id);

    friend class Game;
};

class ComponentManager
{
    std::vector<Tform> m_transforms{ MAX_ENTITIES, Tform() };
    std::vector<Sprite> m_sprites{ MAX_ENTITIES, Sprite() };
    std::vector<BBox> m_bounding_boxes{ MAX_ENTITIES, BBox() };
    std::vector<Grounded> m_grounded{ MAX_ENTITIES, Grounded() };
    std::vector<Lifespan> m_lifespans{ MAX_ENTITIES, Lifespan() };
    std::vector<Health> m_health{ MAX_ENTITIES, Health() };

    ComponentManager() = default;

    friend class Game;
};

struct Inputs {
    bool left = false;
    bool right = false;
    bool up = false;
    bool attack = false;

private:
    Inputs() = default;

    friend class Game;
};

struct Coordinates {
    RVector2 m_pos;

    Coordinates(const int x, const int y);

    operator RVector2() const; // NOLINT
};

class Game
{
    EntityManager m_entity_manager;
    ComponentManager m_component_manager;
    Inputs m_inputs;
    RWindow m_window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
    RTexture m_texture_sheet{ TEXTURE_SHEET };
    RCamera2D m_camera{ RVector2(WINDOW_HALF_WIDTH, WINDOW_HALF_HEIGHT), RVector2(0.0, 0.0), 0.0, CAMERA_ZOOM };

    // Systems
    void poll_inputs();
    void render_sprites();
    void set_player_vel();
    void move_entities();
    void destroy_entities();
    void player_attack();
    void update_lifespans();
    void check_projectiles_hit();

    void spawn_player();
    void spawn_tile(const Tile tile, const RVector2 pos);
    [[nodiscard]] float dt() const;
    void resolve_tile_collisions(const Entity entity, const BBox prev_bbox);
    void spawn_projectile(const RVector2 pos);
    [[nodiscard]] RVector2 get_mouse_pos() const;
    void spawn_enemy(const RVector2 pos);

public:
    Game();

    void run();
    RWindow& window();
};

#endif

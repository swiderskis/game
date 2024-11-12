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

constexpr float CAMERA_ZOOM = 2.0;

class EntityManager
{
    std::vector<Entity> m_entities;
    std::unordered_map<EntityType, std::vector<unsigned>> m_entity_ids;

    EntityManager(); // NOLINT

    void spawn_player();
    [[nodiscard]] unsigned spawn_entity(EntityType type);

    friend class Game;
};

class ComponentManager
{
    std::vector<Tform> m_transforms{ MAX_ENTITIES, Tform() };
    std::vector<Sprite> m_sprites{ MAX_ENTITIES, Sprite() };
    std::vector<BBox> m_bounding_boxes{ MAX_ENTITIES, BBox() };
    std::vector<Grounded> m_grounded{ MAX_ENTITIES, Grounded() };

    ComponentManager() = default;

    void set_player_components();
    void set_circular_bounding_box(unsigned id, RVector2 pos, float radius);

    friend class Game;
};

struct Inputs {
    bool left = false;
    bool right = false;
    bool up = false;

private:
    Inputs() = default;

    friend class Game;
};

struct Coordinates {
    RVector2 m_pos;

    Coordinates(int x, int y);

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

    void spawn_player();
    void spawn_tile(Tile tile, Coordinates coordinates);
    float dt();
    void correct_collisions(unsigned id, BBox prev_bbox);
    void spawn_projectile(Coordinates coordinates);

public:
    Game();

    void run();
    RWindow& window();
};

#endif

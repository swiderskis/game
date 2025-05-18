#ifndef SPRITE_HPP_
#define SPRITE_HPP_

#include "components.hpp"
#include "entities.hpp"
#include "seb-engine-sprite.hpp"

#include <cstdint>

enum class SpriteBase : int8_t
{
    None = -1,

    PlayerIdle,
    Projectile,
    EnemyDuck,

    // tiles
    TileBrick,
};

enum class SpriteHead : int8_t
{
    None = -1,

    PlayerIdle,
};

enum class SpriteArms : int8_t
{
    None = -1,

    PlayerIdle,
    PlayerJump,
    PlayerAttack,
};

enum class SpriteLegs : int8_t
{
    None = -1,

    PlayerIdle,
    PlayerWalk,
    PlayerJump,
};

enum class SpriteExtra : int8_t
{
    None = -1,

    PlayerScarfWalk,
    PlayerScarfFall,
};

namespace sprites
{
seb_engine::SpriteDetails sprite_details(SpriteBase sprite);
seb_engine::SpriteDetails sprite_details(SpriteHead sprite);
seb_engine::SpriteDetails sprite_details(SpriteArms sprite);
seb_engine::SpriteDetails sprite_details(SpriteLegs sprite);
seb_engine::SpriteDetails sprite_details(SpriteExtra sprite);
} // namespace sprites

struct Sprites
{
    seb_engine::Sprite<SpriteBase> base{ SpriteBase::None,
                                         [](SpriteBase sprite) { return sprites::sprite_details(sprite); } };
    seb_engine::Sprite<SpriteHead> head{ SpriteHead::None,
                                         [](SpriteHead sprite) { return sprites::sprite_details(sprite); } };
    seb_engine::Sprite<SpriteArms> arms{ SpriteArms::None,
                                         [](SpriteArms sprite) { return sprites::sprite_details(sprite); } };
    seb_engine::Sprite<SpriteLegs> legs{ SpriteLegs::None,
                                         [](SpriteLegs sprite) { return sprites::sprite_details(sprite); } };
    seb_engine::Sprite<SpriteExtra> extra{ SpriteExtra::None,
                                           [](SpriteExtra sprite) { return sprites::sprite_details(sprite); } };

    void check_update_frames(float dt);
    void draw(raylib::Texture const& texture_sheet, Tform transform, bool flipped);
    void lookup_set_movement_sprites(Entity entity, raylib::Vector2 vel);

private:
    [[nodiscard]] float alternate_frame_y_offset() const;
    void lookup_set_fall_sprites(Entity entity);
    void lookup_set_jump_sprites(Entity entity);
    void lookup_set_walk_sprites(Entity entity);
    void lookup_set_idle_sprites(Entity entity);
    template <typename SpriteEnum>
    [[nodiscard]] raylib::Vector2 render_pos(seb_engine::Sprite<SpriteEnum> sprite,
                                             raylib::Vector2 pos,
                                             bool flipped) const;
};

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

template <typename SpriteEnum>
raylib::Vector2 Sprites::render_pos(const seb_engine::Sprite<SpriteEnum> sprite,
                                    const raylib::Vector2 pos,
                                    const bool flipped) const
{
    // sprite draw pos needs to be offset if it is wider than default sprite size and the sprite is flipped
    const float x_offset = (sprites::sprite_details(sprite.sprite()).size.x - SPRITE_SIZE) * flipped;

    return pos - raylib::Vector2(x_offset, 0.0);
}

#endif

#ifndef SEB_ENGINE_SPRITE_HPP_
#define SEB_ENGINE_SPRITE_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

namespace seb_engine
{
struct SpriteDetails
{
    raylib::Vector2 pos;
    raylib::Vector2 size;
    unsigned frames = 0;
    float frame_duration = 0.0;
    bool allow_movement_override = false;
};

template <typename SpriteEnum>
class Sprite
{
    SpriteEnum m_sprite = (SpriteEnum)-1;
    float m_frame_update_dt = 0.0;
    unsigned m_current_frame = 0;
    float m_frame_duration = 0.0;

public:
    Sprite() = delete;
    explicit Sprite(SpriteEnum sprite);

    void set(SpriteEnum sprite, SpriteDetails details);
    void set(SpriteEnum sprite, float frame_duration);
    void check_update_frame(float dt, SpriteDetails details);
    [[nodiscard]] raylib::Rectangle sprite(bool flipped, SpriteDetails details) const;
    [[nodiscard]] SpriteEnum sprite() const;
    [[nodiscard]] unsigned current_frame() const;
    void movement_set(SpriteEnum sprite, SpriteDetails details);
    void unset();
};
} // namespace seb_engine

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
template <typename SpriteEnum>
Sprite<SpriteEnum>::Sprite(const SpriteEnum sprite) : m_sprite(sprite)
{
}

template <typename SpriteEnum>
void Sprite<SpriteEnum>::set(const SpriteEnum sprite, const SpriteDetails details)
{
    if (m_sprite == sprite)
    {
        return;
    }

    m_sprite = sprite;
    m_current_frame = 0;
    m_frame_update_dt = details.frame_duration;
    m_frame_duration = details.frame_duration;
}

template <typename SpriteEnum>
void Sprite<SpriteEnum>::set(const SpriteEnum sprite, const float frame_duration)
{
    if (m_sprite == sprite)
    {
        return;
    }

    m_sprite = sprite;
    m_current_frame = 0;
    m_frame_update_dt = frame_duration;
    m_frame_duration = frame_duration;
}

template <typename SpriteEnum>
void Sprite<SpriteEnum>::check_update_frame(const float dt, const SpriteDetails details)
{
    if (details.frame_duration == 0.0)
    {
        return;
    }

    m_frame_update_dt -= dt;
    if (m_frame_update_dt > 0)
    {
        return;
    }

    m_frame_update_dt = m_frame_duration;
    m_current_frame += 1;
    if (m_current_frame == details.frames)
    {
        unset();
    }
}

template <typename SpriteEnum>
raylib::Rectangle Sprite<SpriteEnum>::sprite(const bool flipped, const SpriteDetails details) const
{
    const auto pos = raylib::Vector2(details.size.x * m_current_frame, 0.0) + details.pos;
    const auto size = raylib::Vector2((flipped ? -1.0F : 1.0F), 1.0) * details.size;

    return { pos, size };
}

template <typename SpriteEnum>
SpriteEnum Sprite<SpriteEnum>::sprite() const
{
    return m_sprite;
}

template <typename SpriteEnum>
unsigned Sprite<SpriteEnum>::current_frame() const
{
    return m_current_frame;
}

template <typename SpriteEnum>
void Sprite<SpriteEnum>::movement_set(const SpriteEnum sprite, const SpriteDetails details)
{
    if (details.allow_movement_override)
    {
        set(sprite, details);
    }
}

template <typename SpriteEnum>
void Sprite<SpriteEnum>::unset()
{
    m_sprite = (SpriteEnum)-1;
}
} // namespace seb_engine

#endif

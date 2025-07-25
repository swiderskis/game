#ifndef SEB_ENGINE_SPRITE_HPP_
#define SEB_ENGINE_SPRITE_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "seb-engine.hpp"

#include <ranges>
#include <tuple>

namespace seb_engine
{
inline constexpr float SPRITE_SIZE = 32.0;

namespace rl = raylib;

struct SpriteDetails
{
    rl::Vector2 pos;
    rl::Vector2 size;
    unsigned frames = 1;
    float frame_duration = 0.0;
    bool allow_movement_override = true;
};

// register sprite details by specialising this template in program
template <typename SpriteEnum>
struct SpriteDetailsLookup
{
    static SpriteDetails get(SpriteEnum);
};

// assumes SpriteEnum has a "no sprite" value of -1
template <typename SpriteEnum>
class SpritePart
{
    SpriteEnum m_sprite = (SpriteEnum)-1;
    float m_frame_update_dt = 0.0;
    unsigned m_current_frame = 0;
    float m_frame_duration = 0.0;

    static SpriteDetailsLookup<SpriteEnum> s_details_lookup;

    [[nodiscard]] rl::Rectangle rect(bool flipped) const;

public:
    SpritePart() = default;
    explicit SpritePart(SpriteEnum sprite);

    void set(SpriteEnum sprite);
    void set(SpriteEnum sprite, float frame_duration);
    void check_update_frame(float dt);
    [[nodiscard]] SpriteEnum sprite() const;
    [[nodiscard]] unsigned current_frame() const;
    void movement_set(SpriteEnum sprite);
    void unset();
    void draw(rl::Texture const& texture_sheet, rl::Vector2 pos, float dt, bool flipped);
    [[nodiscard]] rl::Vector2 render_pos(rl::Vector2 pos, bool flipped) const;
};

template <typename... SpriteEnums>
class EntitySprites;

template <typename... SpriteEnums>
class Sprites
{
    std::vector<std::tuple<SpritePart<SpriteEnums>...>> m_sprites{ MAX_ENTITIES };

    template <typename SpriteEnum>
    [[nodiscard]] rl::Vector2 render_pos(rl::Vector2 pos, unsigned id, bool flipped) const;
    template <typename SpriteEnum>
    [[nodiscard]] SpritePart<SpriteEnum>& part_mut(unsigned id);
    template <typename SpriteEnum>
    [[nodiscard]] SpritePart<SpriteEnum> const& part(unsigned id) const;

public:
    void draw_all(rl::Texture const& texture_sheet, rl::Vector2 pos, float dt, bool flipped);
    void draw(rl::Texture const& texture_sheet, rl::Vector2 pos, unsigned id, float dt, bool flipped);
    template <typename SpriteEnum>
    void draw_part(rl::Texture const& texture_sheet, rl::Vector2 pos, unsigned id, float dt, bool flipped);
    template <typename SpriteEnum>
    void set(unsigned id, SpriteEnum sprite);
    template <typename SpriteEnum>
    void set(unsigned id, SpriteEnum sprite, float duration);
    template <typename SpriteEnum>
    void movement_set(unsigned id, SpriteEnum sprite);
    [[nodiscard]] EntitySprites<SpriteEnums...> by_id(unsigned id);
    template <typename SpriteEnum>
    [[nodiscard]] SpriteEnum sprite(unsigned id) const;
    template <typename SpriteEnum>
    [[nodiscard]] unsigned current_frame(unsigned id) const;
};

template <typename... SpriteEnums>
class EntitySprites
{
    Sprites<SpriteEnums...>* m_sprites;
    unsigned m_id;

    EntitySprites(Sprites<SpriteEnums...>* sprites, unsigned id);

    friend class Sprites<SpriteEnums...>;

public:
    EntitySprites() = delete;

    template <typename SpriteEnum>
    [[maybe_unused]] EntitySprites<SpriteEnums...>& set(SpriteEnum sprite);
    template <typename SpriteEnum>
    [[maybe_unused]] EntitySprites<SpriteEnums...>& movement_set(SpriteEnum sprite);
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
rl::Rectangle SpritePart<SpriteEnum>::rect(const bool flipped) const
{
    const auto details = s_details_lookup.get(m_sprite);
    const auto pos = rl::Vector2(details.size.x * m_current_frame, 0.0) + details.pos;
    const auto size = rl::Vector2((flipped ? -1.0F : 1.0F), 1.0) * details.size;

    return { pos, size };
}

template <typename SpriteEnum>
SpritePart<SpriteEnum>::SpritePart(const SpriteEnum sprite) : m_sprite(sprite)
{
}

template <typename SpriteEnum>
void SpritePart<SpriteEnum>::set(const SpriteEnum sprite)
{
    if (m_sprite == sprite)
    {
        return;
    }

    const auto details = s_details_lookup.get(sprite);
    m_sprite = sprite;
    m_current_frame = 0;
    m_frame_update_dt = details.frame_duration;
    m_frame_duration = details.frame_duration;
}

template <typename SpriteEnum>
void SpritePart<SpriteEnum>::set(const SpriteEnum sprite, const float frame_duration)
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
void SpritePart<SpriteEnum>::check_update_frame(const float dt)
{
    const auto details = s_details_lookup.get(m_sprite);
    if (m_frame_duration == 0.0)
    {
        return;
    }

    m_frame_update_dt -= dt;
    if (m_frame_update_dt > 0.0)
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
SpriteEnum SpritePart<SpriteEnum>::sprite() const
{
    return m_sprite;
}

template <typename SpriteEnum>
unsigned SpritePart<SpriteEnum>::current_frame() const
{
    return m_current_frame;
}

template <typename SpriteEnum>
void SpritePart<SpriteEnum>::movement_set(const SpriteEnum sprite)
{
    if (s_details_lookup.get(m_sprite).allow_movement_override)
    {
        set(sprite);
    }
}

template <typename SpriteEnum>
void SpritePart<SpriteEnum>::unset()
{
    set((SpriteEnum)-1);
}

template <typename SpriteEnum>
void SpritePart<SpriteEnum>::draw(rl::Texture const& texture_sheet,
                                  const rl::Vector2 pos,
                                  const float dt,
                                  const bool flipped)
{
    texture_sheet.Draw(rect(flipped), render_pos(pos, flipped));
    check_update_frame(dt);
}

template <typename SpriteEnum>
rl::Vector2 SpritePart<SpriteEnum>::render_pos(const rl::Vector2 pos, const bool flipped) const
{
    const auto details = s_details_lookup.get(m_sprite);
    // sprite draw pos needs to be offset if it is wider than default sprite size and the sprite is flipped
    const float x_offset = (details.size.x - SPRITE_SIZE) * flipped;

    return pos - rl::Vector2(x_offset, 0.0);
}

template <typename... SpriteEnums>
template <typename SpriteEnum>
SpritePart<SpriteEnum>& Sprites<SpriteEnums...>::part_mut(const unsigned id)
{
    return std::get<SpritePart<SpriteEnum>>(m_sprites[id]);
}

template <typename... SpriteEnums>
template <typename SpriteEnum>
SpritePart<SpriteEnum> const& Sprites<SpriteEnums...>::part(const unsigned id) const
{
    return std::get<SpritePart<SpriteEnum>>(m_sprites[id]);
}

template <typename... SpriteEnums>
void Sprites<SpriteEnums...>::draw_all(rl::Texture const& texture_sheet,
                                       const rl::Vector2 pos,
                                       const float dt,
                                       const bool flipped)
{
    for (const auto [id, sprite] : m_sprites | std::views::enumerate)
    {
        draw(texture_sheet, pos, id, dt, flipped);
    }
}

template <typename... SpriteEnums>
void Sprites<SpriteEnums...>::draw(
    rl::Texture const& texture_sheet, const rl::Vector2 pos, const unsigned id, const float dt, const bool flipped)
{
    (draw_part<SpriteEnums>(texture_sheet, pos, id, dt, flipped), ...);
}

template <typename... SpriteEnums>
template <typename SpriteEnum>
void Sprites<SpriteEnums...>::draw_part(
    rl::Texture const& texture_sheet, const rl::Vector2 pos, const unsigned id, const float dt, const bool flipped)
{
    part_mut<SpriteEnum>(id).draw(texture_sheet, pos, dt, flipped);
}

template <typename... SpriteEnums>
template <typename SpriteEnum>
void Sprites<SpriteEnums...>::set(const unsigned id, const SpriteEnum sprite)
{
    part_mut<SpriteEnum>(id).set(sprite);
}

template <typename... SpriteEnums>
template <typename SpriteEnum>
void Sprites<SpriteEnums...>::set(const unsigned id, const SpriteEnum sprite, const float duration)
{
    part_mut<SpriteEnum>(id).set(sprite, duration);
}

template <typename... SpriteEnums>
EntitySprites<SpriteEnums...> Sprites<SpriteEnums...>::by_id(const unsigned id)
{
    return { this, id };
}

template <typename... SpriteEnums>
template <typename SpriteEnum>
SpriteEnum Sprites<SpriteEnums...>::sprite(const unsigned id) const
{
    return part<SpriteEnum>(id).sprite();
}

template <typename... SpriteEnums>
template <typename SpriteEnum>
unsigned Sprites<SpriteEnums...>::current_frame(const unsigned id) const
{
    return part<SpriteEnum>(id).current_frame();
}

template <typename... SpriteEnums>
template <typename SpriteEnum>
void Sprites<SpriteEnums...>::movement_set(const unsigned id, const SpriteEnum sprite)
{
    part_mut<SpriteEnum>(id).movement_set(sprite);
}

template <typename... SpriteEnums>
EntitySprites<SpriteEnums...>::EntitySprites(Sprites<SpriteEnums...>* const sprites, const unsigned id) :
    m_sprites(sprites), m_id(id)
{
}

template <typename... SpriteEnums>
template <typename SpriteEnum>
EntitySprites<SpriteEnums...>& EntitySprites<SpriteEnums...>::set(const SpriteEnum sprite)
{
    m_sprites->set(m_id, sprite);

    return *this;
}

template <typename... SpriteEnums>
template <typename SpriteEnum>
EntitySprites<SpriteEnums...>& EntitySprites<SpriteEnums...>::movement_set(const SpriteEnum sprite)
{
    m_sprites->movement_set(m_id, sprite);

    return *this;
}
} // namespace seb_engine

#endif

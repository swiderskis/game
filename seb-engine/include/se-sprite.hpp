#ifndef SE_SPRITE_HPP_
#define SE_SPRITE_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "seblib.hpp"
#include "sl-log.hpp"

#include <ranges>
#include <tuple>

namespace seb_engine
{
namespace rl = raylib;
namespace sl = seblib;

struct SpriteDetails
{
    rl::Vector2 pos;
    rl::Vector2 size;
    unsigned frames = 1;
    float frame_duration = 0.0;
    bool allow_movement_override = true;
};

// register sprite details by specialising this template
template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
struct SpriteDetailsLookup
{
    static SpriteDetails get(SpriteEnum);
};

// assumes SpriteEnum has a "no sprite" value of -1
template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
class SpritePart
{
    SpriteEnum m_sprite = (SpriteEnum)-1;
    float m_frame_update_dt = 0.0;
    unsigned m_current_frame = 0;
    float m_frame_duration = 0.0;

    [[nodiscard]] rl::Rectangle rect(bool flipped) const;

public:
    static SpriteDetailsLookup<SpriteEnum> s_details;

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
};

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
class EntitySprites;

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
class Sprites
{
    std::vector<std::tuple<SpritePart<SpriteEnums>...>> m_sprites{ MaxEntities };

    template <typename SpriteEnum>
    [[nodiscard]] rl::Vector2 render_pos(rl::Vector2 pos, unsigned id, bool flipped) const;
    template <typename SpriteEnum>
    [[nodiscard]] SpritePart<SpriteEnum>& part_mut(unsigned id);
    template <typename SpriteEnum>
    [[nodiscard]] SpritePart<SpriteEnum> const& part(unsigned id) const;

public:
    Sprites() = default;

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
    [[nodiscard]] EntitySprites<MaxEntities, SpriteEnums...> by_id(unsigned id);
    template <typename SpriteEnum>
    [[nodiscard]] SpriteEnum sprite(unsigned id) const;
    template <typename SpriteEnum>
    [[nodiscard]] unsigned current_frame(unsigned id) const;
    void unset(unsigned id);
    template <typename SpriteEnum>
    [[nodiscard]] SpriteDetails details(unsigned id) const;
};

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
class EntitySprites
{
    Sprites<MaxEntities, SpriteEnums...>* m_sprites;
    unsigned m_id;

    EntitySprites(Sprites<MaxEntities, SpriteEnums...>* sprites, unsigned id);

    friend class Sprites<MaxEntities, SpriteEnums...>;

public:
    EntitySprites() = delete;

    template <typename SpriteEnum>
    [[maybe_unused]] EntitySprites<MaxEntities, SpriteEnums...>& set(SpriteEnum sprite);
    template <typename SpriteEnum>
    [[maybe_unused]] EntitySprites<MaxEntities, SpriteEnums...>& movement_set(SpriteEnum sprite);
};
} // namespace seb_engine

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
namespace slog = seblib::log;

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
rl::Rectangle SpritePart<SpriteEnum>::rect(const bool flipped) const
{
    const auto details = s_details.get(m_sprite);
    const auto pos = rl::Vector2(details.size.x * m_current_frame, 0.0) + details.pos;
    const auto size = rl::Vector2((flipped ? -1.0F : 1.0F), 1.0) * details.size;

    return { pos, size };
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
SpritePart<SpriteEnum>::SpritePart(const SpriteEnum sprite) : m_sprite(sprite)
{
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
void SpritePart<SpriteEnum>::set(const SpriteEnum sprite)
{
    if (m_sprite == sprite)
    {
        return;
    }

    const auto details = s_details.get(sprite);
    m_sprite = sprite;
    m_current_frame = 0;
    m_frame_update_dt = details.frame_duration;
    m_frame_duration = details.frame_duration;
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
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
    requires sl::Enumerable<SpriteEnum>
void SpritePart<SpriteEnum>::check_update_frame(const float dt)
{
    const auto details = s_details.get(m_sprite);
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
    requires sl::Enumerable<SpriteEnum>
SpriteEnum SpritePart<SpriteEnum>::sprite() const
{
    return m_sprite;
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
unsigned SpritePart<SpriteEnum>::current_frame() const
{
    return m_current_frame;
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
void SpritePart<SpriteEnum>::movement_set(const SpriteEnum sprite)
{
    if (s_details.get(m_sprite).allow_movement_override)
    {
        set(sprite);
    }
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
void SpritePart<SpriteEnum>::unset()
{
    set((SpriteEnum)-1);
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
void SpritePart<SpriteEnum>::draw(rl::Texture const& texture_sheet,
                                  const rl::Vector2 pos,
                                  const float dt,
                                  const bool flipped)
{
    texture_sheet.Draw(rect(flipped), pos);
    check_update_frame(dt);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
SpritePart<SpriteEnum>& Sprites<MaxEntities, SpriteEnums...>::part_mut(const unsigned id)
{
    return std::get<SpritePart<SpriteEnum>>(m_sprites[id]);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
SpritePart<SpriteEnum> const& Sprites<MaxEntities, SpriteEnums...>::part(const unsigned id) const
{
    return std::get<SpritePart<SpriteEnum>>(m_sprites[id]);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
void Sprites<MaxEntities, SpriteEnums...>::draw_all(rl::Texture const& texture_sheet,
                                                    const rl::Vector2 pos,
                                                    const float dt,
                                                    const bool flipped)
{
    for (const auto [id, sprite] : m_sprites | std::views::enumerate)
    {
        draw(texture_sheet, pos, id, dt, flipped);
    }
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
void Sprites<MaxEntities, SpriteEnums...>::draw(
    rl::Texture const& texture_sheet, const rl::Vector2 pos, const unsigned id, const float dt, const bool flipped)
{
    (draw_part<SpriteEnums>(texture_sheet, pos, id, dt, flipped), ...);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
void Sprites<MaxEntities, SpriteEnums...>::draw_part(
    rl::Texture const& texture_sheet, const rl::Vector2 pos, const unsigned id, const float dt, const bool flipped)
{
    part_mut<SpriteEnum>(id).draw(texture_sheet, pos, dt, flipped);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
void Sprites<MaxEntities, SpriteEnums...>::set(const unsigned id, const SpriteEnum sprite)
{
    part_mut<SpriteEnum>(id).set(sprite);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
void Sprites<MaxEntities, SpriteEnums...>::set(const unsigned id, const SpriteEnum sprite, const float duration)
{
    part_mut<SpriteEnum>(id).set(sprite, duration);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
void Sprites<MaxEntities, SpriteEnums...>::movement_set(const unsigned id, const SpriteEnum sprite)
{
    part_mut<SpriteEnum>(id).movement_set(sprite);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
EntitySprites<MaxEntities, SpriteEnums...> Sprites<MaxEntities, SpriteEnums...>::by_id(const unsigned id)
{
    return { this, id };
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
SpriteEnum Sprites<MaxEntities, SpriteEnums...>::sprite(const unsigned id) const
{
    return part<SpriteEnum>(id).sprite();
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
unsigned Sprites<MaxEntities, SpriteEnums...>::current_frame(const unsigned id) const
{
    return part<SpriteEnum>(id).current_frame();
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
void Sprites<MaxEntities, SpriteEnums...>::unset(const unsigned id)
{
    slog::log(slog::TRC, "Unsetting sprite parts for entity id {}", id);
    (part_mut<SpriteEnums>(id).unset(), ...);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
SpriteDetails Sprites<MaxEntities, SpriteEnums...>::details(unsigned id) const
{
    const auto sprite_part{ part<SpriteEnum>(id) };
    const auto sprite{ sprite_part.sprite() };

    return sprite_part.s_details.get(sprite);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
EntitySprites<MaxEntities, SpriteEnums...>::EntitySprites(Sprites<MaxEntities, SpriteEnums...>* const sprites,
                                                          const unsigned id) : m_sprites(sprites), m_id(id)
{
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
EntitySprites<MaxEntities, SpriteEnums...>& EntitySprites<MaxEntities, SpriteEnums...>::set(const SpriteEnum sprite)
{
    m_sprites->set(m_id, sprite);

    return *this;
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
EntitySprites<MaxEntities, SpriteEnums...>& EntitySprites<MaxEntities, SpriteEnums...>::movement_set(
    const SpriteEnum sprite)
{
    m_sprites->movement_set(m_id, sprite);

    return *this;
}
} // namespace seb_engine

#endif

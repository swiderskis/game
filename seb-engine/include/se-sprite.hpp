#ifndef SE_SPRITE_HPP_
#define SE_SPRITE_HPP_

#include "seblib.hpp"
#include "sl-log.hpp"
#include "sl-math.hpp"

#include <ranges>
#include <tuple>

namespace seb_engine
{
namespace rl = raylib;
namespace sl = seblib;
namespace sm = seblib::math;

struct SpriteDetails
{
    sm::Vec2 pos;
    sm::Vec2 size;
    unsigned frames{ 1 };
    float frame_duration{ 0.0 };
    bool allow_movement_override{ true };
};

// register sprite details by specialising this template
template <typename Sprite>
    requires sl::enumerable<Sprite>
struct SpriteDetailsLookup
{
    static auto get(Sprite) -> SpriteDetails;
};

// assumes Sprite has a "no sprite" value of 0
template <typename Sprite>
    requires sl::enumerable<Sprite>
class SpritePart
{
public:
    static SpriteDetailsLookup<Sprite> s_details;

    SpritePart() = default;
    explicit SpritePart(Sprite sprite);

    auto set(Sprite sprite) -> void;
    auto set(Sprite sprite, float frame_duration) -> void;
    auto check_update_frame(float dt) -> void;
    [[nodiscard]] auto sprite() const -> Sprite;
    [[nodiscard]] auto current_frame() const -> unsigned;
    auto movement_set(Sprite sprite) -> void;
    auto unset() -> void;
    auto draw(rl::Texture const& texture_sheet, sm::Vec2 pos, float dt, bool flipped) -> void;

private:
    Sprite m_sprite{ static_cast<Sprite>(0) };
    float m_frame_update_dt{ 0.0 };
    unsigned m_current_frame{ 0 };
    float m_frame_duration{ 0.0 };

    [[nodiscard]] auto rect(bool flipped) const -> rl::Rectangle;
};

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
class EntitySprites;

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
class Sprites
{
public:
    Sprites() = default;

    auto draw_all(rl::Texture const& texture_sheet, sm::Vec2 pos, float dt, bool flipped) -> void;
    auto draw(rl::Texture const& texture_sheet, sm::Vec2 pos, unsigned id, float dt, bool flipped) -> void;
    template <typename S>
    auto draw_part(rl::Texture const& texture_sheet, sm::Vec2 pos, unsigned id, float dt, bool flipped) -> void;
    template <typename S>
    auto set(unsigned id, S sprite) -> void;
    template <typename S>
    auto set(unsigned id, S sprite, float duration) -> void;
    template <typename S>
    auto movement_set(unsigned id, S sprite) -> void;
    [[nodiscard]] auto by_id(unsigned id) -> EntitySprites<MaxEntities, Sprite...>;
    template <typename S>
    [[nodiscard]] auto sprite(unsigned id) const -> S;
    template <typename S>
    [[nodiscard]] auto current_frame(unsigned id) const -> unsigned;
    auto unset(unsigned id) -> void;
    template <typename S>
    [[nodiscard]] auto details(unsigned id) const -> SpriteDetails;

private:
    std::vector<std::tuple<SpritePart<Sprite>...>> m_sprites{ MaxEntities };

    template <typename S>
    [[nodiscard]] auto part_mut(unsigned id) -> SpritePart<S>&;
    template <typename S>
    [[nodiscard]] auto part(unsigned id) const -> SpritePart<S> const&;
};

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
class EntitySprites
{
public:
    EntitySprites() = delete;

    template <typename S>
    [[maybe_unused]] auto set(S sprite) -> EntitySprites&;
    template <typename S>
    [[maybe_unused]] auto movement_set(S sprite) -> EntitySprites&;

    friend Sprites<MaxEntities, Sprite...>;

private:
    Sprites<MaxEntities, Sprite...>* m_sprites;
    unsigned m_id;

    EntitySprites(Sprites<MaxEntities, Sprite...>& sprites, unsigned id);
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

template <typename Sprite>
    requires sl::enumerable<Sprite>
SpritePart<Sprite>::SpritePart(const Sprite sprite)
    : m_sprite{ sprite }
{
}

template <typename Sprite>
    requires sl::enumerable<Sprite>
auto SpritePart<Sprite>::set(const Sprite sprite) -> void
{
    if (m_sprite == sprite)
    {
        return;
    }

    const auto details{ s_details.get(sprite) };
    m_sprite = sprite;
    m_current_frame = 0;
    m_frame_update_dt = details.frame_duration;
    m_frame_duration = details.frame_duration;
}

template <typename Sprite>
    requires sl::enumerable<Sprite>
auto SpritePart<Sprite>::set(const Sprite sprite, const float frame_duration) -> void
{
    if (m_sprite != sprite)
    {
        m_sprite = sprite;
        m_current_frame = 0;
        m_frame_update_dt = frame_duration;
        m_frame_duration = frame_duration;
    }
}

template <typename Sprite>
    requires sl::enumerable<Sprite>
auto SpritePart<Sprite>::check_update_frame(const float dt) -> void
{
    if (m_frame_duration == 0.0)
    {
        return;
    }

    const auto details{ s_details.get(m_sprite) };
    m_frame_update_dt -= dt;
    if (m_frame_update_dt <= 0.0)
    {
        m_frame_update_dt = m_frame_duration;
        m_current_frame += 1;
        if (m_current_frame == details.frames)
        {
            unset();
        }
    }
}

template <typename Sprite>
    requires sl::enumerable<Sprite>
auto SpritePart<Sprite>::sprite() const -> Sprite
{
    return m_sprite;
}

template <typename Sprite>
    requires sl::enumerable<Sprite>
auto SpritePart<Sprite>::current_frame() const -> unsigned
{
    return m_current_frame;
}

template <typename Sprite>
    requires sl::enumerable<Sprite>
auto SpritePart<Sprite>::movement_set(const Sprite sprite) -> void
{
    if (s_details.get(m_sprite).allow_movement_override)
    {
        set(sprite);
    }
}

template <typename Sprite>
    requires sl::enumerable<Sprite>
auto SpritePart<Sprite>::unset() -> void
{
    set(static_cast<Sprite>(0));
}

template <typename Sprite>
    requires sl::enumerable<Sprite>
auto SpritePart<Sprite>::draw(rl::Texture const& texture_sheet, const sm::Vec2 pos, const float dt, const bool flipped)
    -> void
{
    texture_sheet.Draw(rect(flipped), pos);
    check_update_frame(dt);
}

template <typename Sprite>
    requires sl::enumerable<Sprite>
auto SpritePart<Sprite>::rect(const bool flipped) const -> rl::Rectangle
{
    const auto details{ s_details.get(m_sprite) };
    const auto pos{ sm::Vec2{ details.size.x * m_current_frame, 0.0 } + details.pos };
    const auto size{ sm::Vec2{ (flipped ? -1.0F : 1.0F), 1.0 } * details.size };

    return { pos, size };
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
auto Sprites<MaxEntities, Sprite...>::draw_all(
    rl::Texture const& texture_sheet, const sm::Vec2 pos, const float dt, const bool flipped
) -> void
{
    for (const auto [id, sprite] : m_sprites | std::views::enumerate)
    {
        draw(texture_sheet, pos, id, dt, flipped);
    }
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
auto Sprites<MaxEntities, Sprite...>::draw(
    rl::Texture const& texture_sheet, const sm::Vec2 pos, const unsigned id, const float dt, const bool flipped
) -> void
{
    (draw_part<Sprite>(texture_sheet, pos, id, dt, flipped), ...);
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
template <typename S>
auto Sprites<MaxEntities, Sprite...>::draw_part(
    rl::Texture const& texture_sheet, const sm::Vec2 pos, const unsigned id, const float dt, const bool flipped
) -> void
{
    part_mut<S>(id).draw(texture_sheet, pos, dt, flipped);
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
template <typename S>
auto Sprites<MaxEntities, Sprite...>::set(const unsigned id, const S sprite) -> void
{
    part_mut<S>(id).set(sprite);
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
template <typename S>
auto Sprites<MaxEntities, Sprite...>::set(const unsigned id, const S sprite, const float duration) -> void
{
    part_mut<S>(id).set(sprite, duration);
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
template <typename S>
auto Sprites<MaxEntities, Sprite...>::movement_set(const unsigned id, const S sprite) -> void
{
    part_mut<S>(id).movement_set(sprite);
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
auto Sprites<MaxEntities, Sprite...>::by_id(const unsigned id) -> EntitySprites<MaxEntities, Sprite...>
{
    return { *this, id };
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
template <typename S>
auto Sprites<MaxEntities, Sprite...>::sprite(const unsigned id) const -> S
{
    return part<S>(id).sprite();
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
template <typename S>
auto Sprites<MaxEntities, Sprite...>::current_frame(const unsigned id) const -> unsigned
{
    return part<S>(id).current_frame();
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
auto Sprites<MaxEntities, Sprite...>::unset(const unsigned id) -> void
{
    slog::log(slog::TRC, "Unsetting sprite parts for entity id {}", id);
    (part_mut<Sprite>(id).unset(), ...);
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
template <typename S>
auto Sprites<MaxEntities, Sprite...>::details(unsigned id) const -> SpriteDetails
{
    const auto sprite_part{ part<S>(id) };
    const auto sprite{ sprite_part.sprite() };

    return sprite_part.s_details.get(sprite);
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
template <typename S>
auto Sprites<MaxEntities, Sprite...>::part_mut(const unsigned id) -> SpritePart<S>&
{
    return std::get<SpritePart<S>>(m_sprites[id]);
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
template <typename S>
auto Sprites<MaxEntities, Sprite...>::part(const unsigned id) const -> SpritePart<S> const&
{
    return std::get<SpritePart<S>>(m_sprites[id]);
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
EntitySprites<MaxEntities, Sprite...>::EntitySprites(Sprites<MaxEntities, Sprite...>& sprites, const unsigned id)
    : m_sprites{ &sprites }
    , m_id{ id }
{
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
template <typename S>
auto EntitySprites<MaxEntities, Sprite...>::set(const S sprite) -> EntitySprites&
{
    m_sprites->set(m_id, sprite);

    return *this;
}

template <size_t MaxEntities, typename... Sprite>
    requires(sl::enumerable<Sprite>, ...)
template <typename S>
auto EntitySprites<MaxEntities, Sprite...>::movement_set(const S sprite) -> EntitySprites&
{
    m_sprites->movement_set(m_id, sprite);

    return *this;
}
} // namespace seb_engine

#endif

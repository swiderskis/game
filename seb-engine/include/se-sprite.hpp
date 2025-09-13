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
    unsigned frames{ 1 };
    float frame_duration{ 0.0 };
    bool allow_movement_override{ true };
};

// register sprite details by specialising this template
template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
struct SpriteDetailsLookup
{
    static auto get(SpriteEnum) -> SpriteDetails;
};

// assumes SpriteEnum has a "no sprite" value of 0
template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
class SpritePart
{
public:
    static SpriteDetailsLookup<SpriteEnum> s_details;

    SpritePart() = default;
    explicit SpritePart(SpriteEnum sprite);

    auto set(SpriteEnum sprite) -> void;
    auto set(SpriteEnum sprite, float frame_duration) -> void;
    auto check_update_frame(float dt) -> void;
    [[nodiscard]] auto sprite() const -> SpriteEnum;
    [[nodiscard]] auto current_frame() const -> unsigned;
    auto movement_set(SpriteEnum sprite) -> void;
    auto unset() -> void;
    auto draw(rl::Texture const& texture_sheet, rl::Vector2 pos, float dt, bool flipped) -> void;

private:
    SpriteEnum m_sprite{ static_cast<SpriteEnum>(0) };
    float m_frame_update_dt{ 0.0 };
    unsigned m_current_frame{ 0 };
    float m_frame_duration{ 0.0 };

    [[nodiscard]] auto rect(bool flipped) const -> rl::Rectangle;
};

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
class EntitySprites;

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
class Sprites
{
public:
    Sprites() = default;

    auto draw_all(rl::Texture const& texture_sheet, rl::Vector2 pos, float dt, bool flipped) -> void;
    auto draw(rl::Texture const& texture_sheet, rl::Vector2 pos, unsigned id, float dt, bool flipped) -> void;
    template <typename SpriteEnum>
    auto draw_part(rl::Texture const& texture_sheet, rl::Vector2 pos, unsigned id, float dt, bool flipped) -> void;
    template <typename SpriteEnum>
    auto set(unsigned id, SpriteEnum sprite) -> void;
    template <typename SpriteEnum>
    auto set(unsigned id, SpriteEnum sprite, float duration) -> void;
    template <typename SpriteEnum>
    auto movement_set(unsigned id, SpriteEnum sprite) -> void;
    [[nodiscard]] auto by_id(unsigned id) -> EntitySprites<MaxEntities, SpriteEnums...>;
    template <typename SpriteEnum>
    [[nodiscard]] auto sprite(unsigned id) const -> SpriteEnum;
    template <typename SpriteEnum>
    [[nodiscard]] auto current_frame(unsigned id) const -> unsigned;
    auto unset(unsigned id) -> void;
    template <typename SpriteEnum>
    [[nodiscard]] auto details(unsigned id) const -> SpriteDetails;

private:
    std::vector<std::tuple<SpritePart<SpriteEnums>...>> m_sprites{ MaxEntities };

    template <typename SpriteEnum>
    [[nodiscard]] auto part_mut(unsigned id) -> SpritePart<SpriteEnum>&;
    template <typename SpriteEnum>
    [[nodiscard]] auto part(unsigned id) const -> SpritePart<SpriteEnum> const&;
};

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
class EntitySprites
{
public:
    EntitySprites() = delete;

    template <typename SpriteEnum>
    [[maybe_unused]] auto set(SpriteEnum sprite) -> EntitySprites<MaxEntities, SpriteEnums...>&;
    template <typename SpriteEnum>
    [[maybe_unused]] auto movement_set(SpriteEnum sprite) -> EntitySprites<MaxEntities, SpriteEnums...>&;

    friend class Sprites<MaxEntities, SpriteEnums...>;

private:
    Sprites<MaxEntities, SpriteEnums...>* m_sprites;
    unsigned m_id;

    EntitySprites(Sprites<MaxEntities, SpriteEnums...>* sprites, unsigned id);
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
SpritePart<SpriteEnum>::SpritePart(const SpriteEnum sprite)
    : m_sprite{ sprite }
{
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
auto SpritePart<SpriteEnum>::set(const SpriteEnum sprite) -> void
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

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
auto SpritePart<SpriteEnum>::set(const SpriteEnum sprite, const float frame_duration) -> void
{
    if (m_sprite != sprite)
    {
        m_sprite = sprite;
        m_current_frame = 0;
        m_frame_update_dt = frame_duration;
        m_frame_duration = frame_duration;
    }
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
auto SpritePart<SpriteEnum>::check_update_frame(const float dt) -> void
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

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
auto SpritePart<SpriteEnum>::sprite() const -> SpriteEnum
{
    return m_sprite;
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
auto SpritePart<SpriteEnum>::current_frame() const -> unsigned
{
    return m_current_frame;
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
auto SpritePart<SpriteEnum>::movement_set(const SpriteEnum sprite) -> void
{
    if (s_details.get(m_sprite).allow_movement_override)
    {
        set(sprite);
    }
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
auto SpritePart<SpriteEnum>::unset() -> void
{
    set(static_cast<SpriteEnum>(0));
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
auto SpritePart<SpriteEnum>::draw(rl::Texture const& texture_sheet,
                                  const rl::Vector2 pos,
                                  const float dt,
                                  const bool flipped) -> void
{
    texture_sheet.Draw(rect(flipped), pos);
    check_update_frame(dt);
}

template <typename SpriteEnum>
    requires sl::Enumerable<SpriteEnum>
auto SpritePart<SpriteEnum>::rect(const bool flipped) const -> rl::Rectangle
{
    const auto details{ s_details.get(m_sprite) };
    const auto pos{ rl::Vector2(details.size.x * m_current_frame, 0.0) + details.pos };
    const auto size{ rl::Vector2((flipped ? -1.0F : 1.0F), 1.0) * details.size };

    return { pos, size };
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
auto Sprites<MaxEntities, SpriteEnums...>::draw_all(rl::Texture const& texture_sheet,
                                                    const rl::Vector2 pos,
                                                    const float dt,
                                                    const bool flipped) -> void
{
    for (const auto [id, sprite] : m_sprites | std::views::enumerate)
    {
        draw(texture_sheet, pos, id, dt, flipped);
    }
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
auto Sprites<MaxEntities, SpriteEnums...>::draw(
    rl::Texture const& texture_sheet, const rl::Vector2 pos, const unsigned id, const float dt, const bool flipped)
    -> void
{
    (draw_part<SpriteEnums>(texture_sheet, pos, id, dt, flipped), ...);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
auto Sprites<MaxEntities, SpriteEnums...>::draw_part(
    rl::Texture const& texture_sheet, const rl::Vector2 pos, const unsigned id, const float dt, const bool flipped)
    -> void
{
    part_mut<SpriteEnum>(id).draw(texture_sheet, pos, dt, flipped);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
auto Sprites<MaxEntities, SpriteEnums...>::set(const unsigned id, const SpriteEnum sprite) -> void
{
    part_mut<SpriteEnum>(id).set(sprite);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
auto Sprites<MaxEntities, SpriteEnums...>::set(const unsigned id, const SpriteEnum sprite, const float duration) -> void
{
    part_mut<SpriteEnum>(id).set(sprite, duration);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
auto Sprites<MaxEntities, SpriteEnums...>::movement_set(const unsigned id, const SpriteEnum sprite) -> void
{
    part_mut<SpriteEnum>(id).movement_set(sprite);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
auto Sprites<MaxEntities, SpriteEnums...>::by_id(const unsigned id) -> EntitySprites<MaxEntities, SpriteEnums...>
{
    return { this, id };
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
auto Sprites<MaxEntities, SpriteEnums...>::sprite(const unsigned id) const -> SpriteEnum
{
    return part<SpriteEnum>(id).sprite();
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
auto Sprites<MaxEntities, SpriteEnums...>::current_frame(const unsigned id) const -> unsigned
{
    return part<SpriteEnum>(id).current_frame();
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
auto Sprites<MaxEntities, SpriteEnums...>::unset(const unsigned id) -> void
{
    slog::log(slog::TRC, "Unsetting sprite parts for entity id {}", id);
    (part_mut<SpriteEnums>(id).unset(), ...);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
auto Sprites<MaxEntities, SpriteEnums...>::details(unsigned id) const -> SpriteDetails
{
    const auto sprite_part{ part<SpriteEnum>(id) };
    const auto sprite{ sprite_part.sprite() };

    return sprite_part.s_details.get(sprite);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
auto Sprites<MaxEntities, SpriteEnums...>::part_mut(const unsigned id) -> SpritePart<SpriteEnum>&
{
    return std::get<SpritePart<SpriteEnum>>(m_sprites[id]);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
auto Sprites<MaxEntities, SpriteEnums...>::part(const unsigned id) const -> SpritePart<SpriteEnum> const&
{
    return std::get<SpritePart<SpriteEnum>>(m_sprites[id]);
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
EntitySprites<MaxEntities, SpriteEnums...>::EntitySprites(Sprites<MaxEntities, SpriteEnums...>* const sprites,
                                                          const unsigned id)
    : m_sprites{ sprites }
    , m_id{ id }
{
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
auto EntitySprites<MaxEntities, SpriteEnums...>::set(const SpriteEnum sprite)
    -> EntitySprites<MaxEntities, SpriteEnums...>&
{
    m_sprites->set(m_id, sprite);

    return *this;
}

template <size_t MaxEntities, typename... SpriteEnums>
    requires(sl::Enumerable<SpriteEnums>, ...)
template <typename SpriteEnum>
auto EntitySprites<MaxEntities, SpriteEnums...>::movement_set(const SpriteEnum sprite)
    -> EntitySprites<MaxEntities, SpriteEnums...>&
{
    m_sprites->movement_set(m_id, sprite);

    return *this;
}
} // namespace seb_engine

#endif

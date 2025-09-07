#ifndef SE_COMPONENTS_HPP_
#define SE_COMPONENTS_HPP_

#include "se-bbox.hpp"
#include "seblib.hpp"
#include "sl-log.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

namespace seb_engine
{
namespace sl = seblib;

class IComp
{
public:
    IComp(const IComp&) = default;
    IComp(IComp&&) = default;
    virtual ~IComp() = default;

    virtual auto reset(size_t id) -> void = 0;

    auto operator=(const IComp&) -> IComp& = default;
    auto operator=(IComp&&) -> IComp& = default;

protected:
    IComp() = default;
};

template <size_t MaxEntities, typename Comp>
class Component : public IComp
{
public:
    auto reset(size_t id) -> void override;
    auto vec() -> std::vector<Comp>&;

private:
    std::vector<Comp> m_vec{ MaxEntities, Comp{} };
};

template <size_t MaxEntities>
class EntityComponents;

template <size_t MaxEntities>
class Components
{
public:
    Components();

    auto uninit_destroyed_entity(size_t id) -> void;
    [[nodiscard]] auto by_id(size_t id) -> EntityComponents<MaxEntities>;
    template <typename Comp>
    [[maybe_unused]] auto reg() -> Component<MaxEntities, Comp>*;
    template <typename Comp>
    [[nodiscard]] auto vec() -> std::vector<Comp>&;
    template <typename Comp>
    [[nodiscard]] auto get(size_t id) -> Comp&;
    auto move(float dt) -> void;

    friend class EntityComponents<MaxEntities>;

private:
    std::unordered_map<size_t, std::unique_ptr<IComp>> m_components;

    template <typename Comp>
    auto component() -> Component<MaxEntities, Comp>*;
};

template <size_t MaxEntities>
class EntityComponents
{
public:
    EntityComponents() = delete;

    template <typename Comp>
    [[nodiscard]] auto get() -> Comp&;

    friend class Components<MaxEntities>;

private:
    Components<MaxEntities>* m_components;
    size_t m_id;

    EntityComponents(Components<MaxEntities>& components, size_t id);
};

struct Position;
using Pos = sl::Point<seb_engine::Position>;

struct Velocity;
using Vel = sl::Point<seb_engine::Velocity>;
} // namespace seb_engine

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
namespace slog = seblib::log;

template <size_t MaxEntities, typename Comp>
auto Component<MaxEntities, Comp>::reset(const size_t id) -> void
{
    m_vec[id] = Comp{};
}

template <size_t MaxEntities, typename Comp>
auto Component<MaxEntities, Comp>::vec() -> std::vector<Comp>&
{
    return m_vec;
}

template <size_t MaxEntities>
Components<MaxEntities>::Components()
{
    reg<Pos>();
    reg<Vel>();
    reg<BBox>();
}

template <size_t MaxEntities>
auto Components<MaxEntities>::uninit_destroyed_entity(const size_t id) -> void
{
    for (auto& [_, component] : m_components)
    {
        component->reset(id);
    }
}

template <size_t MaxEntities>
auto Components<MaxEntities>::by_id(const size_t id) -> EntityComponents<MaxEntities>
{
    return { *this, id };
}

template <size_t MaxEntities>
template <typename Comp>
auto Components<MaxEntities>::reg() -> Component<MaxEntities, Comp>*
{
    m_components.emplace(typeid(Comp).hash_code(), std::make_unique<Component<MaxEntities, Comp>>());

    return component<Comp>();
}

template <size_t MaxEntities>
template <typename Comp>
auto Components<MaxEntities>::vec() -> std::vector<Comp>&
{
    return component<Comp>()->vec();
}

template <size_t MaxEntities>
template <typename Comp>
auto Components<MaxEntities>::get(const size_t id) -> Comp&
{
    return component<Comp>()->vec()[id];
}

template <size_t MaxEntities>
auto Components<MaxEntities>::move(const float dt) -> void
{
    auto& pos = vec<Pos>();
    auto& vel = vec<Vel>();
    std::ranges::transform(pos, vel, pos.begin(), [dt](const auto pos, const auto vel) { return pos + (vel * dt); });
}

template <size_t MaxEntities>
template <typename Comp>
auto Components<MaxEntities>::component() -> Component<MaxEntities, Comp>*
{
#ifndef NDEBUG
    if (!m_components.contains(typeid(Comp).hash_code()))
    {
        slog::log(slog::FTL, "Components map does not contain component {}", typeid(Comp).name());
    }
#endif

    return dynamic_cast<Component<MaxEntities, Comp>*>(m_components[typeid(Comp).hash_code()].get());
}

template <size_t MaxEntities>
template <typename Comp>
auto EntityComponents<MaxEntities>::get() -> Comp&
{
    return m_components->template component<Comp>()->vec()[m_id];
}

template <size_t MaxEntities>
EntityComponents<MaxEntities>::EntityComponents(Components<MaxEntities>& components, const size_t id) :
    m_components(&components), m_id(id)
{
}
} // namespace seb_engine

#endif

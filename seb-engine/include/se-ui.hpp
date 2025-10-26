#ifndef SE_UI_HPP_
#define SE_UI_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "sl-math.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace seb_engine::ui
{
inline constexpr unsigned WINDOW_WIDTH{ 800 };
inline constexpr unsigned WINDOW_HEIGHT{ 450 };

namespace rl = raylib;
namespace sm = seblib::math;

struct PercentSize
{
    unsigned width{ 100 };  // NOLINT(*magic-numbers)
    unsigned height{ 100 }; // NOLINT(*magic-numbers)

    explicit PercentSize(unsigned width, unsigned height);
};

struct TextAbsSize
{
    unsigned size;

    explicit TextAbsSize(unsigned size);
};

struct TextPctSize
{
    unsigned size;

    explicit TextPctSize(unsigned size);

    [[nodiscard]] auto abs() const -> unsigned;
};

struct Text
{
public:
    std::string text;
    std::variant<TextAbsSize, TextPctSize> size{ TextAbsSize{ 0 } }; // NOLINT(*magic-numbers)
    rl::Color color;

    Text() = default;

    [[nodiscard]] auto width() const -> int;
    auto draw(rl::Vector2 pos) const -> void;
    [[nodiscard]] auto text_size() const -> unsigned;
};

class Element
{
public:
    std::function<void()> on_click;
    rl::Rectangle rect;
    std::optional<unsigned> parent;
    unsigned layer{ 0 };

    Element(const Element&) = default;
    Element(Element&&) = default;
    virtual ~Element() = default;

    virtual auto render() -> void = 0;

    auto operator=(const Element&) -> Element& = default;
    auto operator=(Element&&) -> Element& = default;
    auto set_pos(PercentSize pos) -> void;
    auto set_size(PercentSize size) -> void;
    [[nodiscard]] auto mouse_overlaps(rl::Vector2 mouse_pos) const -> bool;

protected:
    Element() = default;
};

struct Button : Element
{
    Text text;
    rl::Color color;

    Button() = default;

    auto render() -> void override;
};

template <typename Elem>
struct ElementWithId
{
    size_t id{ 0 };
    Elem* element{ nullptr };
};

class Screen
{
public:
    template <typename Elem>
    [[maybe_unused]] auto new_element() -> ElementWithId<Elem>;
    [[nodiscard]] auto elements() -> std::vector<std::unique_ptr<Element>>&;
    auto click_action(sm::Vec2 mouse_pos) -> bool;
    auto render() -> void;

private:
    std::vector<std::unique_ptr<Element>> m_elements;
};
} // namespace seb_engine::ui

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine::ui
{
template <typename Elem>
auto Screen::new_element() -> ElementWithId<Elem>
{
    return {
        .id = m_elements.size(),
        .element = dynamic_cast<Elem*>(m_elements.emplace_back(std::make_unique<Elem>()).get()),
    };
}
} // namespace seb_engine::ui

#endif

#ifndef SEBLIB_HPP_
#define SEBLIB_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>

inline constexpr unsigned WINDOW_WIDTH = 800;
inline constexpr unsigned WINDOW_HEIGHT = 450;

namespace seblib
{
// exists to allow constexpr vec declarations
struct SimpleVec2
{
    float x;
    float y;

    SimpleVec2() = delete;

    constexpr SimpleVec2(float x, float y);

    constexpr operator raylib::Vector2() const; // NOLINT(hicpp-explicit-conversions)
};

namespace math
{
constexpr float degrees_to_radians(float ang);
constexpr float radians_to_degrees(float ang);
} // namespace math

namespace ui
{
struct PercentSize
{
    unsigned width = 100;  // NOLINT(*magic-numbers)
    unsigned height = 100; // NOLINT(*magic-numbers)
};

struct Text
{
    std::string text;
    raylib::Color color;

private:
    int m_size = 0;
    bool m_is_percent_size = false; // required for redrawing UIs when resizing window - if false,
                                    // text size will remain the same when resizing

public:
    [[nodiscard]] int width() const;
    void draw(raylib::Vector2 pos) const;
    void set_size(int size);
    void set_percent_size(unsigned size);
    [[nodiscard]] int size() const;
};

struct Element
{
    std::function<void()> on_click = []() {};
    raylib::Rectangle rect;
    std::optional<unsigned> parent = std::nullopt;
    unsigned layer = 0;

    Element(const Element&) = default;
    Element(Element&&) = default;
    Element& operator=(const Element&) = default;
    Element& operator=(Element&&) = default;
    virtual ~Element() = default;

    virtual void render() = 0;

    void set_pos(PercentSize pos);
    void set_size(PercentSize size);
    [[nodiscard]] bool mouse_overlaps(raylib::Vector2 mouse_pos) const;

protected:
    Element() = default;
};

struct Button : public Element
{
    Text text;
    raylib::Color color;

    Button() = default;

    void render() override;
};

class Screen
{
    std::vector<std::unique_ptr<Element>> m_elements;

public:
    template <typename Elem>
    [[maybe_unused]] std::tuple<unsigned, Elem*> new_element();
    [[nodiscard]] std::vector<std::unique_ptr<Element>>& elements();
    void click_action(raylib::Vector2 mouse_pos);
    void render();
};
} // namespace ui
} // namespace seblib

namespace seblib
{
constexpr SimpleVec2::SimpleVec2(float x, float y) : x(x), y(y)
{
}

constexpr SimpleVec2::operator raylib::Vector2() const
{
    return { x, y };
}

namespace math
{
constexpr float degrees_to_radians(float ang)
{
    return (float)(ang * M_PI / 180.0); // NOLINT(*magic-numbers)
}

constexpr float radians_to_degrees(float ang)
{
    return (float)(ang * 180.0 / M_PI); // NOLINT(*magic-numbers)
}
} // namespace math

namespace ui
{
template <typename Elem>
std::tuple<unsigned, Elem*> Screen::new_element()
{
    const unsigned id = m_elements.size();
    auto* element = dynamic_cast<Elem*>(m_elements.emplace_back(std::make_unique<Elem>()).get());

    return std::make_tuple(id, element);
}
} // namespace ui
} // namespace seblib

#endif

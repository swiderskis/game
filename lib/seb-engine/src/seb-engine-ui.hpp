#ifndef SEB_ENGINE_UI_HPP_
#define SEB_ENGINE_UI_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <functional>
#include <memory>
#include <optional>
#include <string>

inline constexpr unsigned WINDOW_WIDTH = 800;
inline constexpr unsigned WINDOW_HEIGHT = 450;

namespace seb_engine::ui
{
namespace rl = raylib;

struct PercentSize
{
    unsigned width = 100;  // NOLINT(*magic-numbers)
    unsigned height = 100; // NOLINT(*magic-numbers)
};

struct Text
{
    std::string text;
    rl::Color color;

private:
    int m_size = 0;
    bool m_is_percent_size = false; // required for redrawing UIs when resizing window - if false,
                                    // text size will remain the same when resizing

public:
    [[nodiscard]] int width() const;
    void draw(rl::Vector2 pos) const;
    void set_size(int size);
    void set_percent_size(unsigned size);
    [[nodiscard]] int size() const;
};

struct Element
{
    std::function<void()> on_click = []() {};
    rl::Rectangle rect;
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
    [[nodiscard]] bool mouse_overlaps(rl::Vector2 mouse_pos) const;

protected:
    Element() = default;
};

struct Button : Element
{
    Text text;
    rl::Color color;

    Button() = default;

    void render() override;
};

template <typename Elem>
struct ElementWithId
{
    size_t id = 0;
    Elem* element = nullptr;
};

class Screen
{
    std::vector<std::unique_ptr<Element>> m_elements;

public:
    template <typename Elem>
    [[maybe_unused]] ElementWithId<Elem> new_element();
    [[nodiscard]] std::vector<std::unique_ptr<Element>>& elements();
    void click_action(rl::Vector2 mouse_pos);
    void render();
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
ElementWithId<Elem> Screen::new_element()
{
    return ElementWithId<Elem>{
        .id = m_elements.size(),
        .element = dynamic_cast<Elem*>(m_elements.emplace_back(std::make_unique<Elem>()).get()),
    };
}
} // namespace seb_engine::ui

#endif

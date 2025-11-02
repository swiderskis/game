#include "se-ui.hpp"

#include "seblib.hpp"

#include <algorithm>
#include <ranges>

namespace sl = seblib;

namespace seb_engine::ui
{
namespace ranges = std::ranges;
namespace views = std::views;

PercentSize::PercentSize(unsigned const width, unsigned const height)
    : width{ width }
    , height{ height }
{
}

TextAbsSize::TextAbsSize(unsigned const size)
    : size{ size }
{
}

TextPctSize::TextPctSize(unsigned const size)
    : size{ size }
{
}

auto TextPctSize::abs() const -> unsigned
{
    return static_cast<unsigned>(WINDOW_HEIGHT * (size / 100.0));
}

auto Text::width() const -> int
{
    return rl::MeasureText(text, static_cast<int>(text_size()));
}

auto Text::draw(rl::Vector2 const pos) const -> void
{
    rl::DrawText(text, static_cast<int>(pos.x), static_cast<int>(pos.y), static_cast<int>(text_size()), ::BLACK);
}

auto Text::text_size() const -> unsigned
{
    return sl::match(
        size,
        [](TextAbsSize const size) -> unsigned { return size.size; },
        [](TextPctSize const size) -> unsigned { return size.abs(); }
    );
}

auto Element::set_pos(PercentSize const pos) -> void
{
    rect.x = static_cast<float>(pos.width * WINDOW_WIDTH / 100.0) - (rect.width / 2);
    rect.y = static_cast<float>(pos.height * WINDOW_HEIGHT / 100.0) - (rect.height / 2);
}

auto Element::set_size(PercentSize const size) -> void
{
    auto const old_width{ rect.width };
    auto const old_height{ rect.height };
    rect.width = static_cast<float>(size.width * WINDOW_WIDTH / 100.0);
    rect.height = static_cast<float>(size.height * WINDOW_HEIGHT / 100.0);
    rect.x += (old_width - rect.width) / 2;
    rect.y += (old_height - rect.height) / 2;
}

auto Element::mouse_overlaps(rl::Vector2 const mouse_pos) const -> bool
{
    return rect.CheckCollision(mouse_pos);
}

auto Button::render() -> void
{
    rect.Draw(color);
    float const x{ rect.x + ((rect.width - static_cast<float>(text.width())) / 2) };
    float const y{ rect.y + ((rect.height - static_cast<float>(text.text_size())) / 2) };
    text.draw(rl::Vector2{ x, y });
}

auto Screen::elements() -> std::vector<std::unique_ptr<Element>> &
{
    return m_elements;
}

auto Screen::click_action(sm::Vec2 const mouse_pos) -> bool
{
    auto clicked_elements{ m_elements
                           | views::transform([](auto const & element) { return element.get(); })
                           | views::filter([mouse_pos](auto const * element)
                                           { return element->mouse_overlaps(mouse_pos); }) };
    auto const clicked{ ranges::max_element(clicked_elements, {}, &Element::layer) };
    if (clicked == ranges::end(clicked_elements))
    {
        return false;
    }

    (*clicked)->on_click();

    return true;
}

auto Screen::render() -> void
{
    for (auto & element : m_elements)
    {
        element->render();
    }
}
} // namespace seb_engine::ui

#include "se-ui.hpp"

#include "seblib.hpp"

#include <algorithm>
#include <ranges>

namespace sl = seblib;

namespace seb_engine::ui
{
namespace ranges = std::ranges;
namespace views = std::views;

PercentSize::PercentSize(const unsigned width, const unsigned height)
    : width{ width }
    , height{ height }
{
}

TextAbsSize::TextAbsSize(const unsigned size)
    : size{ size }
{
}

TextPctSize::TextPctSize(const unsigned size)
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

auto Text::draw(const rl::Vector2 pos) const -> void
{
    rl::DrawText(text, static_cast<int>(pos.x), static_cast<int>(pos.y), static_cast<int>(text_size()), ::BLACK);
}

auto Text::text_size() const -> unsigned
{
    return sl::match(
        size,
        [](const TextAbsSize size) -> unsigned { return size.size; },
        [](const TextPctSize size) -> unsigned { return size.abs(); }
    );
}

auto Element::set_pos(const PercentSize pos) -> void
{
    rect.x = static_cast<float>(pos.width * WINDOW_WIDTH / 100.0) - (rect.width / 2);
    rect.y = static_cast<float>(pos.height * WINDOW_HEIGHT / 100.0) - (rect.height / 2);
}

auto Element::set_size(const PercentSize size) -> void
{
    const auto old_width{ rect.width };
    const auto old_height{ rect.height };
    rect.width = static_cast<float>(size.width * WINDOW_WIDTH / 100.0);
    rect.height = static_cast<float>(size.height * WINDOW_HEIGHT / 100.0);
    rect.x += (old_width - rect.width) / 2;
    rect.y += (old_height - rect.height) / 2;
}

auto Element::mouse_overlaps(const rl::Vector2 mouse_pos) const -> bool
{
    return rect.CheckCollision(mouse_pos);
}

auto Button::render() -> void
{
    rect.Draw(color);
    const float x{ rect.x + ((rect.width - static_cast<float>(text.width())) / 2) };
    const float y{ rect.y + ((rect.height - static_cast<float>(text.text_size())) / 2) };
    text.draw(rl::Vector2{ x, y });
}

auto Screen::elements() -> std::vector<std::unique_ptr<Element>>&
{
    return m_elements;
}

auto Screen::click_action(const sm::Vec2 mouse_pos) -> bool
{
    auto clicked_elements{ m_elements
                           | views::transform([](const auto& element) { return element.get(); })
                           | views::filter([mouse_pos](const auto* element)
                                           { return element->mouse_overlaps(mouse_pos); }) };
    const auto clicked{ ranges::max_element(clicked_elements, {}, &Element::layer) };
    if (clicked == ranges::end(clicked_elements))
    {
        return false;
    }

    (*clicked)->on_click();

    return true;
}

auto Screen::render() -> void
{
    for (auto& element : m_elements)
    {
        element->render();
    }
}
} // namespace seb_engine::ui

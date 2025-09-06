#include "se-ui.hpp"

namespace seb_engine::ui
{
auto Text::width() const -> int
{
    return rl::MeasureText(text, m_size);
}

auto Text::draw(const rl::Vector2 pos) const -> void
{
    rl::DrawText(text, static_cast<int>(pos.x), static_cast<int>(pos.y), m_size, ::BLACK);
}

auto Text::set_size(const int size) -> void
{
    this->m_size = size;
    m_is_percent_size = false;
}

auto Text::set_percent_size(const unsigned size) -> void
{
    this->m_size = static_cast<int>(WINDOW_HEIGHT * (size / 100.0));
    m_is_percent_size = true;
}

auto Text::size() const -> int
{
    return m_size;
}

auto Element::set_pos(const PercentSize pos) -> void
{
    rect.x = static_cast<float>(pos.width * WINDOW_WIDTH / 100.0) - rect.width / 2;
    rect.y = static_cast<float>(pos.height * WINDOW_HEIGHT / 100.0) - rect.height / 2;
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
    const float y{ rect.y + ((rect.height - static_cast<float>(text.size())) / 2) };
    text.draw(rl::Vector2{ x, y });
}

auto Screen::elements() -> std::vector<std::unique_ptr<Element>>&
{
    return m_elements;
}

auto Screen::click_action(const rl::Vector2 mouse_pos) -> void
{
    Element* clicked_element{ nullptr };
    for (auto& element : m_elements)
    {
        if (!element->mouse_overlaps(mouse_pos))
        {
            continue;
        }

        if (clicked_element == nullptr || element->layer >= clicked_element->layer)
        {
            clicked_element = element.get();
        }
    }

    if (clicked_element != nullptr)
    {
        clicked_element->on_click();
    }
}

auto Screen::render() -> void
{
    for (auto& element : m_elements)
    {
        element->render();
    }
}
} // namespace seb_engine::ui

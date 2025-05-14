#include "seblib.hpp"

namespace rl = raylib;

namespace seblib::ui
{
int Text::width() const
{
    return rl::MeasureText(text, m_size);
}

void Text::draw(const rl::Vector2 pos) const
{
    rl::DrawText(text, (int)pos.x, (int)pos.y, m_size, ::BLACK);
}

void Text::set_size(const int size)
{
    this->m_size = size;
    m_is_percent_size = false;
}

void Text::set_percent_size(const unsigned size)
{
    this->m_size = (int)(WINDOW_HEIGHT * (size / 100.0));
    m_is_percent_size = true;
}

int Text::size() const
{
    return m_size;
}

void Element::set_pos(const PercentSize pos)
{
    rect.x = (float)(pos.width / 100.0) * WINDOW_WIDTH - rect.width / 2;
    rect.y = (float)(pos.height / 100.0) * WINDOW_HEIGHT - rect.height / 2;
}

void Element::set_size(const PercentSize size)
{
    const auto old_width = rect.width;
    const auto old_height = rect.height;
    rect.width = (float)(size.width / 100.0) * WINDOW_WIDTH;
    rect.height = (float)(size.height / 100.0) * WINDOW_HEIGHT;
    rect.x += (old_width - rect.width) / 2;
    rect.y += (old_height - rect.height) / 2;
}

bool Element::mouse_overlaps(const rl::Vector2 mouse_pos) const
{
    return rect.CheckCollision(mouse_pos);
}

void Button::render()
{
    rect.Draw(color);
    const float x = rect.x + ((rect.width - (float)text.width()) / 2);
    const float y = rect.y + ((rect.height - (float)text.size()) / 2);
    text.draw(rl::Vector2(x, y));
}

std::vector<std::unique_ptr<Element>>& Screen::elements()
{
    return m_elements;
}

void Screen::click_action(const rl::Vector2 mouse_pos)
{
    Element* clicked_element = nullptr;
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

void Screen::render()
{
    for (auto& element : m_elements)
    {
        element->render();
    }
}
} // namespace seblib::ui

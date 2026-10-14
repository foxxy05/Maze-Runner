#include "UI.h"
#include "Config.h"
#include <cmath>

namespace UI {

void drawText(sf::RenderWindow& window, const sf::Font& font, const std::string& text,
              sf::Vector2f pos, unsigned int size, sf::Color color, bool centered) {
    sf::Text txt;
    txt.setFont(font);
    txt.setString(text);
    txt.setCharacterSize(size);
    txt.setFillColor(color);

    if (centered) {
        sf::FloatRect bounds = txt.getLocalBounds();
        txt.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    }
    txt.setPosition(pos);
    window.draw(txt);
}

void drawPanel(sf::RenderWindow& window, sf::FloatRect bounds, sf::Color bgColor,
               sf::Color outlineColor, float outlineThickness) {
    sf::RectangleShape rect(sf::Vector2f(bounds.width, bounds.height));
    rect.setPosition(bounds.left, bounds.top);
    rect.setFillColor(bgColor);
    if (outlineThickness > 0.0f) {
        rect.setOutlineColor(outlineColor);
        rect.setOutlineThickness(outlineThickness);
    }
    window.draw(rect);
}

void drawAnimatedBackground(sf::RenderWindow& window, float elapsedTime) {
    // Fill deep dark background
    window.clear(Theme::Background);

    float spacing = cfg::MENU_BG_GRID_SPACING;
    float offset = std::fmod(elapsedTime * cfg::MENU_BG_SPEED, spacing);

    sf::VertexArray lines(sf::Lines);

    // Vertical drifting lines
    for (float x = -spacing + offset; x <= static_cast<float>(cfg::WINDOW_WIDTH) + spacing; x += spacing) {
        lines.append(sf::Vertex(sf::Vector2f(x, 0.0f), Theme::GridLine));
        lines.append(sf::Vertex(sf::Vector2f(x, static_cast<float>(cfg::WINDOW_HEIGHT)), Theme::GridLine));
    }

    // Horizontal drifting lines
    for (float y = -spacing + offset; y <= static_cast<float>(cfg::WINDOW_HEIGHT) + spacing; y += spacing) {
        lines.append(sf::Vertex(sf::Vector2f(0.0f, y), Theme::GridLine));
        lines.append(sf::Vertex(sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH), y), Theme::GridLine));
    }

    window.draw(lines);
}

void MenuList::addItem(const std::string& label, bool enabled, int id) {
    items_.push_back({label, enabled, id});
    if (selectedIndex_ >= static_cast<int>(items_.size())) {
        selectedIndex_ = 0;
    }
}

void MenuList::navigate(int dir) {
    if (items_.empty()) return;
    int count = static_cast<int>(items_.size());
    int attempts = 0;

    int newIndex = selectedIndex_;
    do {
        newIndex = (newIndex + dir + count) % count;
        attempts++;
    } while (!items_[newIndex].enabled && attempts < count);

    if (items_[newIndex].enabled) {
        selectedIndex_ = newIndex;
    }
}

int MenuList::handleEvent(const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) return -1;

    if (event.key.code == sf::Keyboard::W || event.key.code == sf::Keyboard::Up) {
        navigate(-1);
        return -1;
    }
    if (event.key.code == sf::Keyboard::S || event.key.code == sf::Keyboard::Down) {
        navigate(1);
        return -1;
    }
    if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
        if (!items_.empty() && selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(items_.size())) {
            if (items_[selectedIndex_].enabled) {
                return items_[selectedIndex_].id;
            }
        }
        return -1;
    }
    if (event.key.code == sf::Keyboard::Escape) {
        return -2;
    }

    return -1;
}

void MenuList::setSelectedIndex(int idx) {
    if (idx >= 0 && idx < static_cast<int>(items_.size()) && items_[idx].enabled) {
        selectedIndex_ = idx;
    }
}

void MenuList::draw(sf::RenderWindow& window, const sf::Font& fontBold, const sf::Font& fontRegular,
                    sf::Vector2f centerStart, float itemSpacing, float itemWidth) {
    float itemH = 44.0f;

    for (size_t i = 0; i < items_.size(); ++i) {
        const auto& item = items_[i];
        bool isSelected = (static_cast<int>(i) == selectedIndex_);

        float yPos = centerStart.y + static_cast<float>(i) * itemSpacing;
        sf::FloatRect bounds(centerStart.x - itemWidth / 2.0f, yPos - itemH / 2.0f, itemWidth, itemH);

        sf::Color bgCol = Theme::Panel;
        sf::Color outlineCol = isSelected ? Theme::Orange : Theme::PanelBorder;
        float outlineThick = isSelected ? 2.0f : 1.0f;

        drawPanel(window, bounds, bgCol, outlineCol, outlineThick);

        if (isSelected) {
            // Selected left highlight bar
            sf::RectangleShape bar(sf::Vector2f(4.0f, itemH));
            bar.setPosition(bounds.left, bounds.top);
            bar.setFillColor(Theme::Orange);
            window.draw(bar);
        }

        sf::Color textCol;
        if (!item.enabled) {
            textCol = Theme::TextSecondary;
        } else if (isSelected) {
            textCol = Theme::Orange;
        } else {
            textCol = Theme::TextPrimary;
        }

        // Draw item label
        sf::Text labelTxt;
        labelTxt.setFont(isSelected ? fontBold : fontRegular);
        labelTxt.setString(item.label);
        labelTxt.setCharacterSize(20);
        labelTxt.setFillColor(textCol);
        labelTxt.setPosition(bounds.left + 24.0f, bounds.top + 10.0f);
        window.draw(labelTxt);

    }
}

} // namespace UI

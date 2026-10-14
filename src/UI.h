#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Theme.h"

namespace UI {

void drawText(sf::RenderWindow& window, const sf::Font& font, const std::string& text,
              sf::Vector2f pos, unsigned int size, sf::Color color, bool centered = false);

void drawPanel(sf::RenderWindow& window, sf::FloatRect bounds, sf::Color bgColor,
               sf::Color outlineColor = sf::Color::Transparent, float outlineThickness = 0.0f);

void drawAnimatedBackground(sf::RenderWindow& window, float elapsedTime);

struct MenuItem {
    std::string label;
    bool enabled = true;
    int id = 0;
};

class MenuList {
public:
    MenuList() = default;

    void addItem(const std::string& label, bool enabled = true, int id = 0);
    void navigate(int dir);
    int handleEvent(const sf::Event& event); // Returns selected item id, or -1 if no selection, -2 if Esc pressed

    void setSelectedIndex(int idx);

    void draw(sf::RenderWindow& window, const sf::Font& fontBold, const sf::Font& fontRegular,
              sf::Vector2f centerStart, float itemSpacing = 55.0f, float itemWidth = 420.0f);

private:
    std::vector<MenuItem> items_;
    int selectedIndex_ = 0;
};

} // namespace UI

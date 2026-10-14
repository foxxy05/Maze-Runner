#pragma once
#include <SFML/Graphics/Color.hpp>

namespace Theme {

// Palette defined in project specification
const sf::Color Background(0x07, 0x0A, 0x12);    // #070A12
const sf::Color Panel(0x0E, 0x14, 0x22);         // #0E1422
const sf::Color Cyan(0x00, 0xE5, 0xFF);          // #00E5FF (Runner/Accent)
const sf::Color Crimson(0xFF, 0x2D, 0x55);       // #FF2D55 (Hunter)
const sf::Color Green(0x00, 0xFF, 0x87);         // #00FF87 (Exit)
const sf::Color Amber(0xFF, 0xB8, 0x00);         // #FFB800 (Power-up)
const sf::Color Text(0xE6, 0xEC, 0xFF);          // #E6ECFF

// Extended UI styling helpers
const sf::Color TextDim(140, 155, 190);
const sf::Color TextMuted(95, 105, 135);
const sf::Color PanelOutline(30, 48, 80);
const sf::Color PanelOutlineBright(0, 229, 255, 120);
const sf::Color OverlayDim(7, 10, 18, 220);
const sf::Color MenuGrid(15, 25, 45, 75);
const sf::Color WallBase(18, 23, 38);
const sf::Color PathBase(10, 13, 24);

} // namespace Theme

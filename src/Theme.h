#pragma once

#include <SFML/Graphics/Color.hpp>
#include <algorithm>

namespace Theme {

inline const sf::Color Background(19, 19, 21);
inline const sf::Color Panel(28, 28, 31);
inline const sf::Color PanelBorder(44, 44, 48);
inline const sf::Color Floor(25, 25, 28);
inline const sf::Color GridLine(32, 32, 36);
inline const sf::Color Wall(42, 42, 46);
inline const sf::Color WallEdge(60, 60, 66);
inline const sf::Color TextPrimary(236, 232, 225);
inline const sf::Color TextSecondary(142, 142, 147);
inline const sf::Color Orange(255, 122, 26);
inline const sf::Color DarkOrange(196, 86, 11);
inline const sf::Color Bone(230, 226, 218);
inline const sf::Color Red(229, 56, 59);
inline const sf::Color ExitGold(255, 201, 77);
inline const sf::Color FogDark(14, 14, 16);
inline const sf::Color VignetteBlack(0, 0, 0);
inline const sf::Color GradientMask(255, 255, 255);
inline const sf::Color Transparent(0, 0, 0, 0);

inline sf::Color withAlpha(sf::Color color, sf::Uint8 alpha) {
    color.a = alpha;
    return color;
}

inline sf::Color mix(sf::Color from, sf::Color to, float amount) {
    amount = std::max(0.0f, std::min(1.0f, amount));
    return sf::Color(
        static_cast<sf::Uint8>(from.r + (to.r - from.r) * amount),
        static_cast<sf::Uint8>(from.g + (to.g - from.g) * amount),
        static_cast<sf::Uint8>(from.b + (to.b - from.b) * amount),
        static_cast<sf::Uint8>(from.a + (to.a - from.a) * amount));
}

} // namespace Theme

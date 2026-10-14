#pragma once
#include <SFML/Window.hpp>
#include <vector>
#include <string>

struct KeySet {
    sf::Keyboard::Key up;
    sf::Keyboard::Key down;
    sf::Keyboard::Key left;
    sf::Keyboard::Key right;
};

struct Direction {
    int dr = 0;
    int dc = 0;
    bool isZero() const { return dr == 0 && dc == 0; }
};

class DirectionInput {
public:
    explicit DirectionInput(KeySet keys);

    Direction update(const sf::Window& window);
    void reset(const sf::Window& window);
    Direction currentDirection() const { return currentDir_; }
    unsigned long long lastPressTick() const { return lastPressTick_; }

    bool isKeyUpDown() const { return sf::Keyboard::isKeyPressed(keys_.up); }
    bool isKeyDownDown() const { return sf::Keyboard::isKeyPressed(keys_.down); }
    bool isKeyLeftDown() const { return sf::Keyboard::isKeyPressed(keys_.left); }
    bool isKeyRightDown() const { return sf::Keyboard::isKeyPressed(keys_.right); }
    int heldKeyCount() const { return static_cast<int>(pressOrder_.size()); }

    std::string directionString() const {
        if (currentDir_.dr == -1 && currentDir_.dc == 0) return "UP";
        if (currentDir_.dr == 1 && currentDir_.dc == 0) return "DOWN";
        if (currentDir_.dr == 0 && currentDir_.dc == -1) return "LEFT";
        if (currentDir_.dr == 0 && currentDir_.dc == 1) return "RIGHT";
        return "NONE";
    }

private:
    KeySet keys_;
    bool prevPressed_[4] = {false, false, false, false};
    std::vector<int> pressOrder_;
    Direction currentDir_{0, 0};
    unsigned long long lastPressTick_ = 0;
};

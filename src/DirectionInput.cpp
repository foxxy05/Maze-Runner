#include "DirectionInput.h"
#include <algorithm>

DirectionInput::DirectionInput(KeySet keys)
    : keys_(keys) {
}

Direction DirectionInput::update(const sf::Window& window) {
    if (!window.hasFocus()) {
        currentDir_ = {0, 0};
        return currentDir_;
    }

    const sf::Keyboard::Key keyList[4] = {keys_.up, keys_.down, keys_.left, keys_.right};

    for (int i = 0; i < 4; i++) {
        bool isDown = sf::Keyboard::isKeyPressed(keyList[i]);
        if (isDown && !prevPressed_[i]) {
            pressOrder_.erase(std::remove(pressOrder_.begin(), pressOrder_.end(), i), pressOrder_.end());
            pressOrder_.push_back(i);
            static unsigned long long s_globalTick = 0;
            lastPressTick_ = ++s_globalTick;
        } else if (!isDown && prevPressed_[i]) {
            pressOrder_.erase(std::remove(pressOrder_.begin(), pressOrder_.end(), i), pressOrder_.end());
        }
        prevPressed_[i] = isDown;
    }

    pressOrder_.erase(
        std::remove_if(pressOrder_.begin(), pressOrder_.end(), [&](int idx) {
            return !sf::Keyboard::isKeyPressed(keyList[idx]);
        }),
        pressOrder_.end()
    );

    if (pressOrder_.empty()) {
        currentDir_ = {0, 0};
        return currentDir_;
    }

    int winningIdx = pressOrder_.back();
    switch (winningIdx) {
        case 0: currentDir_ = {-1, 0}; break; // Up
        case 1: currentDir_ = { 1, 0}; break; // Down
        case 2: currentDir_ = { 0,-1}; break; // Left
        case 3: currentDir_ = { 0, 1}; break; // Right
        default: currentDir_ = {0, 0}; break;
    }

    return currentDir_;
}

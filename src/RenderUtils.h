#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "Config.h"
#include "Theme.h"

// Helper to draw Sci-Fi Player Character with 4-frame stride running animation
inline void drawAnimatedPlayer(sf::RenderWindow& window, sf::Vector2f center, int dr, int dc, int frame, bool isMoving) {
    float legL = 0.0f, legR = 0.0f, armL = 0.0f, armR = 0.0f, headBob = 0.0f;
    if (isMoving) {
        if (frame == 1) { legL = 4.0f; legR = -4.0f; armL = -3.0f; armR = 3.0f; headBob = 1.0f; }
        if (frame == 2) { legL = 1.0f; legR = -1.0f; armL = 0.0f;  armR = 0.0f; headBob = 0.0f; }
        if (frame == 3) { legL = -4.0f; legR = 4.0f; armL = 3.0f;  armR = -3.0f; headBob = 1.0f; }
    }

    sf::RectangleShape leftLeg(sf::Vector2f(3.0f, 6.0f));
    leftLeg.setPosition(center.x - 3.0f, center.y + 2.0f + legL);
    leftLeg.setFillColor(Theme::DarkOrange);
    window.draw(leftLeg);

    sf::RectangleShape rightLeg(sf::Vector2f(3.0f, 6.0f));
    rightLeg.setPosition(center.x + 1.0f, center.y + 2.0f + legR);
    rightLeg.setFillColor(Theme::DarkOrange);
    window.draw(rightLeg);

    sf::RectangleShape leftBoot(sf::Vector2f(4.0f, 2.5f));
    leftBoot.setPosition(center.x - 3.5f, center.y + 7.5f + legL);
    leftBoot.setFillColor(Theme::Orange);
    window.draw(leftBoot);

    sf::RectangleShape rightBoot(sf::Vector2f(4.0f, 2.5f));
    rightBoot.setPosition(center.x + 0.5f, center.y + 7.5f + legR);
    rightBoot.setFillColor(Theme::Orange);
    window.draw(rightBoot);

    sf::RectangleShape torso(sf::Vector2f(9.0f, 8.0f));
    torso.setOrigin(4.5f, 4.0f);
    torso.setPosition(center.x, center.y - 1.0f + headBob);
    torso.setFillColor(Theme::Orange);
    window.draw(torso);

    sf::RectangleShape leftArm(sf::Vector2f(2.5f, 6.0f));
    leftArm.setPosition(center.x - 6.5f, center.y - 4.0f + armL + headBob);
    leftArm.setFillColor(Theme::DarkOrange);
    window.draw(leftArm);

    sf::RectangleShape rightArm(sf::Vector2f(2.5f, 6.0f));
    rightArm.setPosition(center.x + 4.0f, center.y - 4.0f + armR + headBob);
    rightArm.setFillColor(Theme::DarkOrange);
    window.draw(rightArm);

    sf::RectangleShape helmet(sf::Vector2f(7.0f, 6.0f));
    helmet.setOrigin(3.5f, 3.0f);
    helmet.setPosition(center.x, center.y - 7.0f + headBob);
    helmet.setFillColor(Theme::Orange);
    window.draw(helmet);

    sf::RectangleShape visor(sf::Vector2f(5.0f, 2.5f));
    visor.setOrigin(2.5f, 1.25f);
    visor.setPosition(center.x + dc * 2.0f, center.y - 7.0f + dr * 1.5f + headBob);
    visor.setFillColor(Theme::Bone);
    window.draw(visor);

    sf::CircleShape core(1.8f);
    core.setOrigin(1.8f, 1.8f);
    core.setPosition(center.x, center.y - 1.0f + headBob);
    core.setFillColor(Theme::Bone);
    window.draw(core);

    if (isMoving) {
        sf::CircleShape trail(2.0f);
        trail.setOrigin(1.0f, 1.0f);
        trail.setPosition(center.x - dc * 7.0f, center.y - dr * 7.0f);
        trail.setFillColor(Theme::withAlpha(Theme::DarkOrange, 150));
        window.draw(trail);
    }
}

// Helper to draw Cyber-Spectre Hunter Character with 4-frame stride animation
inline void drawAnimatedHunter(sf::RenderWindow& window, sf::Vector2f center, int frame, bool isMoving, float pulse, bool isDormant) {
    sf::CircleShape threat(cfg::HUNTER_THREAT_RADIUS);
    threat.setOrigin(cfg::HUNTER_THREAT_RADIUS, cfg::HUNTER_THREAT_RADIUS);
    threat.setPosition(center);
    threat.setFillColor(Theme::withAlpha(Theme::Red,
        static_cast<sf::Uint8>(isDormant ? 24 : 45 + pulse * 20)));
    window.draw(threat);

    float clawL = 0.0f, clawR = 0.0f, robeSway = 0.0f;
    if (isMoving) {
        if (frame == 1) { clawL = -4.0f; clawR = 3.0f; robeSway = -2.0f; }
        if (frame == 2) { clawL = 0.0f;  clawR = 0.0f; robeSway = 0.0f; }
        if (frame == 3) { clawL = 3.0f;  clawR = -4.0f; robeSway = 2.0f; }
    }

    sf::ConvexShape cloak;
    cloak.setPointCount(5);
    cloak.setPoint(0, sf::Vector2f(center.x - 6.0f, center.y - 3.0f));
    cloak.setPoint(1, sf::Vector2f(center.x + 6.0f, center.y - 3.0f));
    cloak.setPoint(2, sf::Vector2f(center.x + 7.0f + robeSway, center.y + 8.0f));
    cloak.setPoint(3, sf::Vector2f(center.x, center.y + 10.0f));
    cloak.setPoint(4, sf::Vector2f(center.x - 7.0f + robeSway, center.y + 8.0f));
    cloak.setFillColor(Theme::Bone);
    window.draw(cloak);

    sf::RectangleShape leftClaw(sf::Vector2f(3.0f, 7.0f));
    leftClaw.setPosition(center.x - 8.5f, center.y - 2.0f + clawL);
    leftClaw.setFillColor(Theme::Bone);
    window.draw(leftClaw);

    sf::RectangleShape rightClaw(sf::Vector2f(3.0f, 7.0f));
    rightClaw.setPosition(center.x + 5.5f, center.y - 2.0f + clawR);
    rightClaw.setFillColor(Theme::Bone);
    window.draw(rightClaw);

    sf::ConvexShape skull;
    skull.setPointCount(6);
    skull.setPoint(0, sf::Vector2f(0, -9));
    skull.setPoint(1, sf::Vector2f(6, -5));
    skull.setPoint(2, sf::Vector2f(5, 2));
    skull.setPoint(3, sf::Vector2f(0, 5));
    skull.setPoint(4, sf::Vector2f(-5, 2));
    skull.setPoint(5, sf::Vector2f(-6, -5));
    skull.setPosition(center);
    skull.setFillColor(Theme::Bone);
    window.draw(skull);

    sf::CircleShape leftEye(2.0f);
    leftEye.setOrigin(2.0f, 2.0f);
    leftEye.setPosition(center.x - 2.5f, center.y - 2.0f);
    leftEye.setFillColor(Theme::Red);
    window.draw(leftEye);

    sf::CircleShape rightEye(2.0f);
    rightEye.setOrigin(2.0f, 2.0f);
    rightEye.setPosition(center.x + 2.5f, center.y - 2.0f);
    rightEye.setFillColor(Theme::Red);
    window.draw(rightEye);
}


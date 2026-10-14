#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "Config.h"

// Helper to draw Sci-Fi Player Character with 4-frame stride running animation
inline void drawAnimatedPlayer(sf::RenderWindow& window, sf::Vector2f center, int dr, int dc, int frame, bool isMoving, float pulse) {
    sf::CircleShape glow(cfg::PLAYER_GLOW_RADIUS);
    glow.setOrigin(cfg::PLAYER_GLOW_RADIUS, cfg::PLAYER_GLOW_RADIUS);
    glow.setPosition(center);
    glow.setFillColor(sf::Color(0, 240, 255, static_cast<sf::Uint8>(40 + pulse * 30)));
    window.draw(glow);

    float legL = 0.0f, legR = 0.0f, armL = 0.0f, armR = 0.0f, headBob = 0.0f;
    if (isMoving) {
        if (frame == 1) { legL = 4.0f; legR = -4.0f; armL = -3.0f; armR = 3.0f; headBob = 1.0f; }
        if (frame == 2) { legL = 1.0f; legR = -1.0f; armL = 0.0f;  armR = 0.0f; headBob = 0.0f; }
        if (frame == 3) { legL = -4.0f; legR = 4.0f; armL = 3.0f;  armR = -3.0f; headBob = 1.0f; }
    }

    sf::RectangleShape leftLeg(sf::Vector2f(3.0f, 6.0f));
    leftLeg.setPosition(center.x - 3.0f, center.y + 2.0f + legL);
    leftLeg.setFillColor(sf::Color(0, 180, 220));
    window.draw(leftLeg);

    sf::RectangleShape rightLeg(sf::Vector2f(3.0f, 6.0f));
    rightLeg.setPosition(center.x + 1.0f, center.y + 2.0f + legR);
    rightLeg.setFillColor(sf::Color(0, 180, 220));
    window.draw(rightLeg);

    sf::RectangleShape leftBoot(sf::Vector2f(4.0f, 2.5f));
    leftBoot.setPosition(center.x - 3.5f, center.y + 7.5f + legL);
    leftBoot.setFillColor(sf::Color(0, 240, 255));
    window.draw(leftBoot);

    sf::RectangleShape rightBoot(sf::Vector2f(4.0f, 2.5f));
    rightBoot.setPosition(center.x + 0.5f, center.y + 7.5f + legR);
    rightBoot.setFillColor(sf::Color(0, 240, 255));
    window.draw(rightBoot);

    sf::RectangleShape torso(sf::Vector2f(9.0f, 8.0f));
    torso.setOrigin(4.5f, 4.0f);
    torso.setPosition(center.x, center.y - 1.0f + headBob);
    torso.setFillColor(sf::Color(0, 210, 240));
    window.draw(torso);

    sf::RectangleShape leftArm(sf::Vector2f(2.5f, 6.0f));
    leftArm.setPosition(center.x - 6.5f, center.y - 4.0f + armL + headBob);
    leftArm.setFillColor(sf::Color(0, 160, 200));
    window.draw(leftArm);

    sf::RectangleShape rightArm(sf::Vector2f(2.5f, 6.0f));
    rightArm.setPosition(center.x + 4.0f, center.y - 4.0f + armR + headBob);
    rightArm.setFillColor(sf::Color(0, 160, 200));
    window.draw(rightArm);

    sf::RectangleShape helmet(sf::Vector2f(7.0f, 6.0f));
    helmet.setOrigin(3.5f, 3.0f);
    helmet.setPosition(center.x, center.y - 7.0f + headBob);
    helmet.setFillColor(sf::Color(0, 240, 255));
    window.draw(helmet);

    sf::RectangleShape visor(sf::Vector2f(5.0f, 2.5f));
    visor.setOrigin(2.5f, 1.25f);
    visor.setPosition(center.x + dc * 2.0f, center.y - 7.0f + dr * 1.5f + headBob);
    visor.setFillColor(sf::Color(255, 255, 255));
    window.draw(visor);

    sf::CircleShape core(1.8f);
    core.setOrigin(1.8f, 1.8f);
    core.setPosition(center.x, center.y - 1.0f + headBob);
    core.setFillColor(sf::Color(255, 255, 255));
    window.draw(core);

    if (isMoving) {
        sf::CircleShape trail(2.0f);
        trail.setOrigin(1.0f, 1.0f);
        trail.setPosition(center.x - dc * 7.0f, center.y - dr * 7.0f);
        trail.setFillColor(sf::Color(0, 255, 220, 180));
        window.draw(trail);
    }
}

// Helper to draw Cyber-Spectre Hunter Character with 4-frame stride animation
inline void drawAnimatedHunter(sf::RenderWindow& window, sf::Vector2f center, int frame, bool isMoving, float pulse, bool isDormant) {
    sf::CircleShape threat(cfg::HUNTER_THREAT_RADIUS);
    threat.setOrigin(cfg::HUNTER_THREAT_RADIUS, cfg::HUNTER_THREAT_RADIUS);
    threat.setPosition(center);
    sf::Color threatColor = isDormant ? sf::Color(120, 25, 35, 90)
                                      : sf::Color(255, 20, 50, static_cast<sf::Uint8>(75 + pulse * 75));
    threat.setFillColor(threatColor);
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
    cloak.setFillColor(isDormant ? sf::Color(120, 20, 30) : sf::Color(220, 15, 45));
    window.draw(cloak);

    sf::RectangleShape leftClaw(sf::Vector2f(3.0f, 7.0f));
    leftClaw.setPosition(center.x - 8.5f, center.y - 2.0f + clawL);
    leftClaw.setFillColor(sf::Color(255, 80, 100));
    window.draw(leftClaw);

    sf::RectangleShape rightClaw(sf::Vector2f(3.0f, 7.0f));
    rightClaw.setPosition(center.x + 5.5f, center.y - 2.0f + clawR);
    rightClaw.setFillColor(sf::Color(255, 80, 100));
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
    skull.setFillColor(isDormant ? sf::Color(160, 40, 50) : sf::Color(255, 35, 65));
    window.draw(skull);

    sf::CircleShape leftEye(2.0f);
    leftEye.setOrigin(2.0f, 2.0f);
    leftEye.setPosition(center.x - 2.5f, center.y - 2.0f);
    leftEye.setFillColor(isDormant ? sf::Color(255, 180, 0) : sf::Color(255, 255, 180));
    window.draw(leftEye);

    sf::CircleShape rightEye(2.0f);
    rightEye.setOrigin(2.0f, 2.0f);
    rightEye.setPosition(center.x + 2.5f, center.y - 2.0f);
    rightEye.setFillColor(isDormant ? sf::Color(255, 180, 0) : sf::Color(255, 255, 180));
    window.draw(rightEye);
}

// Vector 7-Segment Digital Number Renderer
inline void drawDigitalNumber(sf::RenderWindow& window, sf::Vector2f pos, int number, sf::Color color) {
    float w = cfg::DIGIT_WIDTH, h = cfg::DIGIT_HEIGHT, t = cfg::DIGIT_THICKNESS;

    auto drawSeg = [&](float x, float y, float sw, float sh) {
        sf::RectangleShape seg(sf::Vector2f(sw, sh));
        seg.setPosition(pos.x + x, pos.y + y);
        seg.setFillColor(color);
        window.draw(seg);
    };

    bool a = (number == 2 || number == 3 || number == 5);
    bool b = (number == 1 || number == 2 || number == 3 || number == 4);
    bool c = (number == 1 || number == 3 || number == 4 || number == 5);
    bool d = (number == 2 || number == 3 || number == 5);
    bool e = (number == 2);
    bool f = (number == 4 || number == 5);
    bool g = (number == 2 || number == 3 || number == 4 || number == 5);

    if (a) drawSeg(0, 0, w, t);
    if (b) drawSeg(w - t, 0, t, h / 2);
    if (c) drawSeg(w - t, h / 2, t, h / 2);
    if (d) drawSeg(0, h - t, w, t);
    if (e) drawSeg(0, h / 2, t, h / 2);
    if (f) drawSeg(0, 0, t, h / 2);
    if (g) drawSeg(0, h / 2 - t / 2, w, t);
}

// Big Central 5-to-1 Countdown Overlay
inline void drawBigCountdownOverlay(sf::RenderWindow& window, int mazeCols, int mazeRows, int cellW, int cellH, float previewTimeLeft, bool canMove) {
    float winW = static_cast<float>(mazeCols * cellW);
    float winH = static_cast<float>(mazeRows * cellH);
    sf::Vector2f center(winW / 2.0f, winH / 2.0f);

    sf::RectangleShape backdrop(sf::Vector2f(280.0f, 130.0f));
    backdrop.setOrigin(140.0f, 65.0f);
    backdrop.setPosition(center);
    backdrop.setFillColor(sf::Color(8, 10, 16, 230));
    backdrop.setOutlineThickness(2.5f);
    backdrop.setOutlineColor(canMove ? sf::Color(0, 255, 135) : sf::Color(0, 230, 255));
    window.draw(backdrop);

    int digitNum = static_cast<int>(std::ceil(previewTimeLeft));
    if (digitNum > cfg::MAX_COUNTDOWN_DIGIT) digitNum = cfg::MAX_COUNTDOWN_DIGIT;
    if (digitNum < cfg::MIN_COUNTDOWN_DIGIT) digitNum = cfg::MIN_COUNTDOWN_DIGIT;

    sf::CircleShape ring(36.0f);
    ring.setOrigin(36.0f, 36.0f);
    ring.setPosition(center.x, center.y - 12.0f);
    ring.setFillColor(sf::Color(14, 18, 28, 220));
    ring.setOutlineThickness(3.0f);
    ring.setOutlineColor(canMove ? sf::Color(0, 255, 135) : sf::Color(0, 230, 255));
    window.draw(ring);

    sf::Color numColor = canMove ? sf::Color(0, 255, 135) : sf::Color(0, 240, 255);
    drawDigitalNumber(window, sf::Vector2f(center.x - 12.0f, center.y - 31.0f), digitNum, numColor);

    sf::RectangleShape statusBg(sf::Vector2f(260.0f, 24.0f));
    statusBg.setOrigin(130.0f, 12.0f);
    statusBg.setPosition(center.x, center.y + 38.0f);
    statusBg.setFillColor(canMove ? sf::Color(0, 180, 90, 200) : sf::Color(0, 110, 170, 200));
    window.draw(statusBg);
}

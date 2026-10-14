#include <SFML/Graphics.hpp>
#include "Maze.h"
#include "Player.h"
#include "Hunter.h"
#include "RenderUtils.h"
#include <sstream>
#include <iomanip>

// ============================================================================
// GAME MECHANICS CONFIGURATION (CONFIGURABLE MACROS)
// ============================================================================

// Grid & Tile Dimensions
#define MAZE_ROWS 39                  // Grid Row Count (Odd dimension for DFS)
#define MAZE_COLS 39                  // Grid Column Count
#define CELL_W 20                     // Tile Width (px)
#define CELL_H 16                     // Tile Height (px)

// Timers & Delay Controls (Balanced for fun gameplay)
#define MAP_PREVIEW_DURATION 5.0f     // Full map visibility duration (seconds)
#define MOVEMENT_FREEZE_TIME 4.0f     // Player freeze duration before movement unlocks
#define HUNTER_HEADSTART_DELAY 3.5f   // Seconds before Hunter awakens and chases
#define HUNTER_MOVE_INTERVAL 0.120f    // Hunter pathfinding step interval (200ms)

// Radar & Fog Controls
#define HUNTER_BLINK_CYCLE 1.5f       // Radar blink cycle interval
#define HUNTER_BLINK_ON_TIME 0.25f    // Radar blip duration inside fog

// Display & Gameplay Constants
#define TARGET_FPS 60
#define PULSE_SPEED 5.0f
#define FOG_VISION_RADIUS 3
#define FOG_LIGHT_CONE_EXTRA 4
#define DELTA_TIME_CAP 0.1f

enum class GameState { Previewing, Playing, Won, Lost };

int main() {
    // 1. Initialize Maze, Player, and Hunter entities
    Maze maze(MAZE_ROWS, MAZE_COLS);
    maze.generate();

    Player player(maze.startRow(), maze.startCol(), CELL_W, CELL_H);
    player.resetDiscovery(MAZE_ROWS, MAZE_COLS);
    player.discoverAround(maze, FOG_VISION_RADIUS, FOG_LIGHT_CONE_EXTRA);

    Hunter hunter(maze.startRow(), maze.startCol(), CELL_W, CELL_H);

    // 2. Create SFML Window
    sf::RenderWindow window(
        sf::VideoMode(MAZE_COLS * CELL_W, MAZE_ROWS * CELL_H),
        "Maze Chase - PREVIEWING MAP...",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setFramerateLimit(TARGET_FPS);

    sf::Clock gameTimer;
    sf::Clock hunterStepClock;
    sf::Clock pulseClock;
    sf::Clock deltaClock;

    GameState state = GameState::Previewing;

    // Helper lambda to reset game state for new runs (R key)
    auto resetGame = [&]() {
        maze.generate();
        player = Player(maze.startRow(), maze.startCol(), CELL_W, CELL_H);
        player.resetDiscovery(MAZE_ROWS, MAZE_COLS);
        player.discoverAround(maze, FOG_VISION_RADIUS, FOG_LIGHT_CONE_EXTRA);
        hunter = Hunter(maze.startRow(), maze.startCol(), CELL_W, CELL_H);
        state = GameState::Previewing;
        gameTimer.restart();
        hunterStepClock.restart();
        deltaClock.restart();
    };

    // ========================================================================
    // MAIN GAME LOOP
    // ========================================================================
    while (window.isOpen()) {
        float dt = deltaClock.restart().asSeconds();
        if (dt > DELTA_TIME_CAP) dt = DELTA_TIME_CAP;

        // --- EVENT HANDLING ---
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
                resetGame();
                continue;
            }
        }

        float elapsedTotal = gameTimer.getElapsedTime().asSeconds();
        bool playerCanMove = (elapsedTotal >= MOVEMENT_FREEZE_TIME);

        // Read Directional Input (WASD or Arrow Keys)
        int inputDr = 0, inputDc = 0;
        if (playerCanMove && (state == GameState::Previewing || state == GameState::Playing)) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)    || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) inputDr = -1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)  || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) inputDr = 1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)  || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) inputDc = -1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) inputDc = 1;
        }

        // --- GAME STATE & PHYSICS UPDATES ---
        if (state == GameState::Previewing || state == GameState::Playing) {
            if (playerCanMove) {
                player.update(dt, inputDr, inputDc, maze);
                player.discoverAround(maze, FOG_VISION_RADIUS, FOG_LIGHT_CONE_EXTRA);
            }

            // Win Condition Check (Player reaches exit tile)
            if (player.gridRow() == maze.exitRow() && player.gridCol() == maze.exitCol()) {
                state = GameState::Won;
                window.setTitle("Maze Chase - YOU ESCAPED! (Press R for new random maze)");
            }
        }

        // Handle Map Preview Timer
        if (state == GameState::Previewing) {
            float previewLeft = MAP_PREVIEW_DURATION - elapsedTotal;
            if (previewLeft > 0) {
                std::ostringstream ss;
                if (!playerCanMove) {
                    ss << "MAP PREVIEW (5s): MEMORIZE MAZE (Move unlocks in " << std::fixed << std::setprecision(1) << (MOVEMENT_FREEZE_TIME - elapsedTotal) << "s)";
                } else {
                    ss << "MOVE UNLOCKED! RUN! (Fog falls in " << std::fixed << std::setprecision(1) << previewLeft << "s)";
                }
                window.setTitle(ss.str());
            } else {
                state = GameState::Playing;
            }
        }

        // Handle Hunter Pursuit Logic
        if (state == GameState::Playing) {
            float hunterAwakeLeft = HUNTER_HEADSTART_DELAY - elapsedTotal;
            bool hunterIsAwake = (hunterAwakeLeft <= 0);

            if (!hunterIsAwake) {
                std::ostringstream ss;
                ss << "RUN! HUNTER AWAKENS IN " << std::fixed << std::setprecision(1) << hunterAwakeLeft << "s...";
                window.setTitle(ss.str());
            } else {
                window.setTitle("Maze Chase - RUN FOR THE EXIT! (Press R to retry)");

                // Recompute BFS Pathfinding Step every 350ms
                if (hunterStepClock.getElapsedTime().asSeconds() >= HUNTER_MOVE_INTERVAL) {
                    hunter.step(maze, player.gridRow(), player.gridCol());
                    hunterStepClock.restart();
                }

                hunter.update(dt);

                // Lose Condition Check (Hunter touches player)
                float diffX = player.posX() - hunter.posX();
                float diffY = player.posY() - hunter.posY();
                if ((diffX * diffX + diffY * diffY) < (CELL_H * 0.75f) * (CELL_H * 0.75f)) {
                    state = GameState::Lost;
                    window.setTitle("Maze Chase - CAUGHT BY THE HUNTER! (Press R to retry)");
                }
            }
        }

        // --- RENDER PIPELINE ---
        window.clear(sf::Color(7, 10, 18));
        float pulse = (std::sin(pulseClock.getElapsedTime().asSeconds() * PULSE_SPEED) + 1.0f) * 0.5f;
        bool isPreviewingMap = (state == GameState::Previewing);

        // 1. Draw Maze Tiles & Exit Gate
        for (int r = 0; r < MAZE_ROWS; r++) {
            for (int c = 0; c < MAZE_COLS; c++) {
                if (!isPreviewingMap && !player.isDiscovered(r, c)) continue;

                sf::Vector2f pos(c * CELL_W, r * CELL_H);
                if (maze.isWall(r, c)) {
                    sf::RectangleShape wallBase(sf::Vector2f(CELL_W, CELL_H));
                    wallBase.setPosition(pos);
                    wallBase.setFillColor(sf::Color(18, 23, 38));
                    window.draw(wallBase);

                    sf::RectangleShape wallCap(sf::Vector2f(CELL_W - 2, 2.5f));
                    wallCap.setPosition(pos.x + 1, pos.y + 1);
                    wallCap.setFillColor(sf::Color(0, 229, 255));
                    window.draw(wallCap);
                } else {
                    sf::RectangleShape pathTile(sf::Vector2f(CELL_W, CELL_H));
                    pathTile.setPosition(pos);
                    pathTile.setFillColor(sf::Color(10, 13, 24));
                    window.draw(pathTile);
                }

                if (r == maze.exitRow() && c == maze.exitCol()) {
                    sf::RectangleShape exitGate(sf::Vector2f(CELL_W - 2, CELL_H - 2));
                    exitGate.setPosition(pos.x + 1, pos.y + 1);
                    exitGate.setFillColor(sf::Color(0, static_cast<sf::Uint8>(180 + pulse * 75), 135));
                    window.draw(exitGate);
                }
            }
        }

        // 2. Draw Player Entity
        sf::Vector2f playerCenter(player.posX() + CELL_W / 2.0f, player.posY() + CELL_H / 2.0f);
        drawAnimatedPlayer(window, playerCenter, player.facingDr(), player.facingDc(), player.animFrame(), player.isMoving(), pulse);

        // 3. Draw Hunter Entity (Radar Blinking)
        float blinkPhase = std::fmod(pulseClock.getElapsedTime().asSeconds(), HUNTER_BLINK_CYCLE);
        bool isBlinkActive = (blinkPhase < HUNTER_BLINK_ON_TIME);
        bool hunterInFog = player.isDiscovered(hunter.gridRow(), hunter.gridCol());
        bool hunterVisible = isPreviewingMap || (hunterInFog && isBlinkActive);

        if (hunterVisible) {
            sf::Vector2f hunterCenter(hunter.posX() + CELL_W / 2.0f, hunter.posY() + CELL_H / 2.0f);
            bool isDormant = (elapsedTotal < HUNTER_HEADSTART_DELAY);
            drawAnimatedHunter(window, hunterCenter, hunter.animFrame(), hunter.isMoving(), pulse, isDormant);
        }

        // 4. Draw Central 5-to-1 Countdown Overlay
        if (state == GameState::Previewing && (MAP_PREVIEW_DURATION - elapsedTotal) > 0) {
            drawBigCountdownOverlay(window, MAZE_COLS, MAZE_ROWS, CELL_W, CELL_H, MAP_PREVIEW_DURATION - elapsedTotal, playerCanMove);
        }

        window.display();
    }

    return 0;
}

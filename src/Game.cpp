#include "Game.h"
#include "RenderUtils.h"
#include "Theme.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <random>
#include <functional>

static float clampVal(float val, float low, float high) {
    if (val < low) return low;
    if (val > high) return high;
    return val;
}

Game::Game(unsigned int seed)
    : window_(sf::VideoMode(cfg::WINDOW_WIDTH, cfg::WINDOW_HEIGHT), "Maze Chase", sf::Style::Titlebar | sf::Style::Close),
      cameraView_(),
      previewView_(),
      p1View_(),
      p2View_(),
      p1PreviewView_(),
      p2PreviewView_(),
      maze_(cfg::MAZE_ROWS, cfg::MAZE_COLS),
      player_(1, 1, cfg::TILE_SIZE, cfg::TILE_SIZE),
      hunter_(1, 1, cfg::TILE_SIZE, cfg::TILE_SIZE),
      tpRunner_(1, 1, cfg::TILE_SIZE, cfg::TILE_SIZE),
      tpHunter_(1, 1, cfg::TILE_SIZE, cfg::TILE_SIZE, cfg::TWO_P_HUNTER_CELLS_PER_SEC),
      wasdInput_({sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D}),
      arrowInput_({sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right}),
      initialSeed_(seed) {

    window_.setFramerateLimit(cfg::TARGET_FPS);

    // Setup 1P camera view (~25x14 tiles = 800x450 at 32px per tile)
    cameraView_.setSize(cfg::CAMERA_VIEW_W, cfg::CAMERA_VIEW_H);

    // Setup 1P preview view to show full 1248x1248 maze scaled to fit 16:9 window
    float mazeW = static_cast<float>(cfg::MAZE_COLS * cfg::TILE_SIZE);
    float mazeH = static_cast<float>(cfg::MAZE_ROWS * cfg::TILE_SIZE);
    previewView_.setCenter(mazeW / 2.0f, mazeH / 2.0f);
    float previewScaleH = mazeH * 1.18f;
    previewView_.setSize(previewScaleH * (16.0f / 9.0f), previewScaleH);

    // Setup 2P split viewports (640x720 each)
    p1View_.setViewport(sf::FloatRect(0.0f, 0.0f, 0.5f, 1.0f));
    p1View_.setSize(400.0f, 450.0f); // 8:9 aspect ratio

    p2View_.setViewport(sf::FloatRect(0.5f, 0.0f, 0.5f, 1.0f));
    p2View_.setSize(400.0f, 450.0f);

    p1PreviewView_.setSize(previewScaleH * (8.0f / 9.0f), previewScaleH);
    p1PreviewView_.setCenter(mazeW / 2.0f, mazeH / 2.0f);
    p1PreviewView_.setViewport(sf::FloatRect(0.0f, 0.0f, 0.5f, 1.0f));

    p2PreviewView_.setSize(previewScaleH * (8.0f / 9.0f), previewScaleH);
    p2PreviewView_.setCenter(mazeW / 2.0f, mazeH / 2.0f);
    p2PreviewView_.setViewport(sf::FloatRect(0.5f, 0.0f, 0.5f, 1.0f));

    // Initialize Menu Lists
    mainMenu_.addItem("Single Player", true, 1);
    mainMenu_.addItem("Local 2-Player", true, 2);
    mainMenu_.addItem("Leaderboard", true, 3);
    mainMenu_.addItem("How to Play", true, 4);
    mainMenu_.addItem("Controls Test", true, 6);
    mainMenu_.addItem("Quit", true, 5);

    pauseMenu_.addItem("Resume", true, 1);
    pauseMenu_.addItem("Restart", true, 2);
    pauseMenu_.addItem("Main Menu", true, 3);

    gameOverMenu_.addItem("Retry", true, 1);
    gameOverMenu_.addItem("Main Menu", true, 2);

    twoPlayerPauseMenu_.addItem("Resume", true, 1);
    twoPlayerPauseMenu_.addItem("Restart Match", true, 2);
    twoPlayerPauseMenu_.addItem("Main Menu", true, 3);

    twoPlayerMatchOverMenu_.addItem("Rematch", true, 1);
    twoPlayerMatchOverMenu_.addItem("Main Menu", true, 2);

    loadFonts();
    buildRadialGradient();
    resetRound(initialSeed_);
}

void Game::buildRadialGradient() {
    const unsigned int size = cfg::RADIAL_GRADIENT_SIZE;
    sf::Image image; image.create(size, size, sf::Color::Transparent);
    for (unsigned int y = 0; y < size; ++y) for (unsigned int x = 0; x < size; ++x) {
        float dx = (static_cast<float>(x) / (size - 1)) * 2.0f - 1.0f;
        float dy = (static_cast<float>(y) / (size - 1)) * 2.0f - 1.0f;
        float edge = std::min(1.0f, std::sqrt(dx * dx + dy * dy));
        sf::Uint8 alpha = static_cast<sf::Uint8>(edge * edge * 190.0f);
        image.setPixel(x, y, sf::Color(255, 255, 255, alpha));
    }
    radialGradientTexture_.loadFromImage(image);
    radialGradientSprite_.setTexture(radialGradientTexture_);
}

void Game::rebuildMazeTexture() {
    const unsigned int width = cfg::MAZE_COLS * cfg::TILE_SIZE;
    const unsigned int height = cfg::MAZE_ROWS * cfg::TILE_SIZE;
    if (!mazeTexture_.create(width, height)) return;
    mazeTexture_.clear(Theme::Background);
    for (int r = 0; r < cfg::MAZE_ROWS; ++r) for (int c = 0; c < cfg::MAZE_COLS; ++c) {
        sf::Vector2f pos(c * cfg::TILE_SIZE, r * cfg::TILE_SIZE);
        sf::RectangleShape tile({static_cast<float>(cfg::TILE_SIZE), static_cast<float>(cfg::TILE_SIZE)});
        tile.setPosition(pos);
        if (maze_.isWall(r,c)) {
            tile.setFillColor(Theme::WallBase); mazeTexture_.draw(tile);
            const int dr[4] = {-1, 1, 0, 0}, dc[4] = {0, 0, -1, 1};
            for (int side = 0; side < 4; ++side) if (!maze_.isWall(r + dr[side], c + dc[side])) {
                sf::RectangleShape edge({side < 2 ? static_cast<float>(cfg::TILE_SIZE) : 2.0f, side < 2 ? 2.0f : static_cast<float>(cfg::TILE_SIZE)});
                edge.setPosition(pos.x + (side == 3 ? cfg::TILE_SIZE - 2.0f : 0.0f), pos.y + (side == 1 ? cfg::TILE_SIZE - 2.0f : 0.0f));
                edge.setFillColor(sf::Color(0, 220, 255, 125)); mazeTexture_.draw(edge);
            }
        } else {
            tile.setFillColor(Theme::PathBase); mazeTexture_.draw(tile);
            sf::Vertex grid[] = {sf::Vertex(pos, sf::Color(35, 60, 85, 35)), sf::Vertex({pos.x + cfg::TILE_SIZE, pos.y}, sf::Color(35,60,85,35))};
            mazeTexture_.draw(grid, 2, sf::Lines);
        }
    }
    mazeTexture_.display(); mazeSprite_.setTexture(mazeTexture_.getTexture()); mazeTextureReady_ = true;
}

bool Game::tryLoadFont(sf::Font& font, const std::string& assetPath, const std::vector<std::string>& fallbacks) {
    if (font.loadFromFile(assetPath)) {
        return true;
    }
    for (const auto& fb : fallbacks) {
        if (font.loadFromFile(fb)) {
            std::cerr << "[WARNING] Font '" << assetPath << "' missing or failed to load. Falling back to system font: " << fb << std::endl;
            return true;
        }
    }
    std::cerr << "[ERROR] Could not load font '" << assetPath << "' or any system fallback!" << std::endl;
    return false;
}

void Game::loadFonts() {
    const std::vector<std::string> sysRegular = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };
    const std::vector<std::string> sysBold = {
        "C:/Windows/Fonts/segoeuib.ttf",
        "C:/Windows/Fonts/arialbd.ttf"
    };

    tryLoadFont(fontOrbitron_, "assets/fonts/Orbitron-Variable.ttf", sysBold);
    tryLoadFont(fontRajdhaniMedium_, "assets/fonts/Rajdhani-Medium.ttf", sysRegular);
    tryLoadFont(fontRajdhaniBold_, "assets/fonts/Rajdhani-Bold.ttf", sysBold);
}

void Game::resetRound(unsigned int seed) {
    wasdInput_.reset(window_);
    arrowInput_.reset(window_);
    showBfsDemo_ = false;
    bfsDemoElapsed_ = 0.0f;
    shownBfsRevision_ = 0;
    maze_.beginGenerate(seed);
    generationForTwoPlayer_ = false;
    generationSpeed_ = 1.0f;
}

void Game::finishGeneratedRound() {
    std::cout << "Maze Seed: " << maze_.seed() << std::endl;

    rebuildMazeTexture();
    if (generationForTwoPlayer_) {
        std::pair<int, int> spawn = Pathfinder::findSpawnCell(
            maze_, {maze_.startRow(), maze_.startCol()}, cfg::HUNTER_SPAWN_DIST);
        tpRunner_ = Player(maze_.startRow(), maze_.startCol(), cfg::TILE_SIZE, cfg::TILE_SIZE);
        tpRunner_.setCellsPerSec(cfg::PLAYER_CELLS_PER_SEC);
        tpRunner_.resetDiscovery(cfg::MAZE_ROWS, cfg::MAZE_COLS);
        tpRunner_.discoverAround(maze_, cfg::TWO_P_RUNNER_LOS_RADIUS, 0);
        tpHunter_ = Hunter(spawn.first, spawn.second, cfg::TILE_SIZE, cfg::TILE_SIZE, cfg::TWO_P_HUNTER_CELLS_PER_SEC);
        tpHunter_.resetDiscovery(cfg::MAZE_ROWS, cfg::MAZE_COLS);
        tpHunter_.discoverAround(maze_, cfg::TWO_P_HUNTER_LOS_RADIUS);
        spawnPowerUps(true);
        p1CameraPos_ = (twoPlayerRound_ == 1)
            ? sf::Vector2f(tpRunner_.posX() + cfg::TILE_SIZE / 2.0f, tpRunner_.posY() + cfg::TILE_SIZE / 2.0f)
            : sf::Vector2f(tpHunter_.posX() + cfg::TILE_SIZE / 2.0f, tpHunter_.posY() + cfg::TILE_SIZE / 2.0f);
        p2CameraPos_ = (twoPlayerRound_ == 1)
            ? sf::Vector2f(tpHunter_.posX() + cfg::TILE_SIZE / 2.0f, tpHunter_.posY() + cfg::TILE_SIZE / 2.0f)
            : sf::Vector2f(tpRunner_.posX() + cfg::TILE_SIZE / 2.0f, tpRunner_.posY() + cfg::TILE_SIZE / 2.0f);
        p1View_.setCenter(p1CameraPos_);
        p2View_.setCenter(p2CameraPos_);
        roundElapsed_ = 0.0f;
        pulseTime_ = 0.0f;
        state_ = GameState::TwoPlayerPreview;
        return;
    }

    std::pair<int, int> hunterSpawn = Pathfinder::findSpawnCell(
        maze_, {maze_.startRow(), maze_.startCol()}, cfg::HUNTER_SPAWN_DIST
    );

    player_ = Player(maze_.startRow(), maze_.startCol(), cfg::TILE_SIZE, cfg::TILE_SIZE);
    player_.resetDiscovery(cfg::MAZE_ROWS, cfg::MAZE_COLS);
    int levelFog = std::max(2, cfg::FOG_VISION_RADIUS - (level_ >= 4) - (level_ >= 7));
    player_.discoverAround(maze_, levelFog, cfg::FOG_LIGHT_CONE_EXTRA);

    hunter_ = Hunter(hunterSpawn.first, hunterSpawn.second, cfg::TILE_SIZE, cfg::TILE_SIZE, cfg::HUNTER_CELLS_PER_SEC);
    hunter_.setCellsPerSec(std::min(cfg::PLAYER_CELLS_PER_SEC * 1.05f, cfg::HUNTER_CELLS_PER_SEC * (1.0f + 0.05f * (level_ - 1))));
    spawnPowerUps(false);

    cameraPos_.x = player_.posX() + cfg::TILE_SIZE / 2.0f;
    cameraPos_.y = player_.posY() + cfg::TILE_SIZE / 2.0f;
    cameraView_.setCenter(cameraPos_);

    roundElapsed_ = 0.0f;
    pulseTime_ = 0.0f;
    won_ = false;
}

void Game::startTwoPlayerMatch(unsigned int seed) {
    if (seed == 0) {
        auto nano = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        twoPlayerMatchSeed_ = static_cast<unsigned int>(nano ^ std::random_device{}());
        if (twoPlayerMatchSeed_ == 0) twoPlayerMatchSeed_ = 1;
    } else {
        twoPlayerMatchSeed_ = seed;
    }

    round1Data_ = TwoPlayerRoundData();
    round2Data_ = TwoPlayerRoundData();
    twoPlayerRound_ = 1;
    startTwoPlayerRound(1);
}

void Game::startTwoPlayerRound(int roundNum) {
    wasdInput_.reset(window_);
    arrowInput_.reset(window_);
    showBfsDemo_ = false;
    twoPlayerRound_ = roundNum;
    maze_.beginGenerate(twoPlayerMatchSeed_); // Same seed makes the same maze for both rounds.
    generationForTwoPlayer_ = true;
    generationSpeed_ = 1.0f;
    state_ = GameState::Generating;
}

void Game::run() {
    sf::Clock deltaClock;

    while (window_.isOpen()) {
        float dt = deltaClock.restart().asSeconds();
        if (dt > cfg::DELTA_TIME_CAP) dt = cfg::DELTA_TIME_CAP;

        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
            return;
        }

        // Global F1 toggle for Controls & Ghosting Test in any state
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F1) {
            showControlsTest_ = !showControlsTest_;
            continue;
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F2) { lowFx_ = !lowFx_; toastTime_ = 2.0f; continue; }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F4) { showFps_ = !showFps_; continue; }

        if (showControlsTest_) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::F1) {
                    showControlsTest_ = false;
                }
            }
            continue;
        }

        // Menu State Events
        if (state_ == GameState::Menu) {
            if (subScreen_ != MenuSubScreen::Main) {
                if (event.type == sf::Event::KeyPressed) {
                    if (subScreen_ == MenuSubScreen::Leaderboard && event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num3) {
                        leaderboardTraversal_ = static_cast<int>(event.key.code - sf::Keyboard::Num0);
                        continue;
                    }
                    if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::Return) {
                        subScreen_ = MenuSubScreen::Main;
                    }
                }
            } else {
                int action = mainMenu_.handleEvent(event);
                if (action == 1) { // Single Player
                    level_ = 1; runScore_ = 0;
                    resetRound(0);
                    state_ = GameState::Generating;
                } else if (action == 2) { // Local 2-Player
                    startTwoPlayerMatch(0);
                } else if (action == 3) { // Leaderboard
                    subScreen_ = MenuSubScreen::Leaderboard;
                } else if (action == 4) { // How to Play
                    subScreen_ = MenuSubScreen::HowToPlay;
                } else if (action == 6) { // Controls Test
                    showControlsTest_ = true;
                } else if (action == 5) { // Quit
                    window_.close();
                    return;
                }
            }
        }
        // Single Player In-game
        else if (state_ == GameState::Preview || state_ == GameState::Playing) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    stateBeforePause_ = state_;
                    state_ = GameState::Paused;
                    wasdInput_.reset(window_);
                    arrowInput_.reset(window_);
                    pauseMenu_.setSelectedIndex(0);
                } else if (event.key.code == sf::Keyboard::R) {
                    resetRound(0);
                    state_ = GameState::Generating;
                } else if (event.key.code == sf::Keyboard::F3) {
                    showBfsDemo_ = !showBfsDemo_;
                    bfsDemoElapsed_ = 0.0f;
                } else if (event.key.code == sf::Keyboard::Space) {
                    activatePowerUp(1);
                }
            }
        }
        else if (state_ == GameState::Generating) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    while (maze_.stepGenerate()) {}
                } else if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) {
                    generationSpeed_ = std::min(8.0f, generationSpeed_ * 2.0f);
                } else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Hyphen) {
                    generationSpeed_ = std::max(0.25f, generationSpeed_ * 0.5f);
                }
            }
        }
        // Single Player Pause
        else if (state_ == GameState::Paused) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F3) {
                showBfsDemo_ = !showBfsDemo_;
                bfsDemoElapsed_ = 0.0f;
                continue;
            }
            int action = pauseMenu_.handleEvent(event);
            if (action == 1 || action == -2) { // Resume or Esc
                state_ = stateBeforePause_;
            } else if (action == 2) { // Restart
                resetRound(0);
                state_ = GameState::Generating;
            } else if (action == 3) { // Main Menu
                state_ = GameState::Menu;
                subScreen_ = MenuSubScreen::Main;
            }
        }
        // Single Player Game Over
        else if (state_ == GameState::GameOver) {
            int action = gameOverMenu_.handleEvent(event);
            if (action == 1) { // Retry
                level_ = 1; runScore_ = 0;
                resetRound(0);
                state_ = GameState::Generating;
            } else if (action == 2 || action == -2) { // Main Menu or Esc
                state_ = GameState::Menu;
                subScreen_ = MenuSubScreen::Main;
            }
        }
        else if (state_ == GameState::LevelCleared) {
            if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space)) {
                ++level_; resetRound(0); state_ = GameState::Generating;
            }
        }
        else if (state_ == GameState::NameEntry) {
            if (event.type == sf::Event::TextEntered) {
                char ch = static_cast<char>(event.text.unicode);
                if (event.text.unicode >= 32 && event.text.unicode < 127 && enteredName_.size() < 10) enteredName_ += ch;
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::BackSpace && !enteredName_.empty()) enteredName_.pop_back();
                else if (event.key.code == sf::Keyboard::Return) {
                    leaderboard_.add(enteredName_, pendingScore_, pendingNameIsTwoPlayer_ ? "2P" : "SP", pendingDetail_);
                    state_ = pendingNameIsTwoPlayer_ ? GameState::TwoPlayerMatchOver : GameState::GameOver;
                }
            }
        }
        // 2-Player In-Game Events
        else if (state_ == GameState::TwoPlayerPreview || state_ == GameState::TwoPlayerPlaying) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    stateBeforePause_ = state_;
                    state_ = GameState::TwoPlayerPaused;
                    wasdInput_.reset(window_);
                    arrowInput_.reset(window_);
                    twoPlayerPauseMenu_.setSelectedIndex(0);
                } else if (event.key.code == sf::Keyboard::Space) {
                    activatePowerUp(1);
                } else if (event.key.code == sf::Keyboard::Return) {
                    activatePowerUp(2);
                }
            }
        }
        // 2-Player Paused
        else if (state_ == GameState::TwoPlayerPaused) {
            int action = twoPlayerPauseMenu_.handleEvent(event);
            if (action == 1 || action == -2) { // Resume or Esc
                state_ = stateBeforePause_;
            } else if (action == 2) { // Restart Match
                startTwoPlayerMatch(twoPlayerMatchSeed_);
            } else if (action == 3) { // Main Menu
                state_ = GameState::Menu;
                subScreen_ = MenuSubScreen::Main;
            }
        }
        // 2-Player Intermission (Between Rounds)
        else if (state_ == GameState::TwoPlayerIntermission) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                    startTwoPlayerRound(2);
                } else if (event.key.code == sf::Keyboard::Escape) {
                    state_ = GameState::Menu;
                    subScreen_ = MenuSubScreen::Main;
                }
            }
        }
        // 2-Player Match Over
        else if (state_ == GameState::TwoPlayerMatchOver) {
            int action = twoPlayerMatchOverMenu_.handleEvent(event);
            if (action == 1) { // Rematch
                startTwoPlayerMatch(0);
            } else if (action == 2 || action == -2) { // Main Menu or Esc
                state_ = GameState::Menu;
                subScreen_ = MenuSubScreen::Main;
            }
        }
    }
}

void Game::update(float dt) {
    float instantFps = dt > 0.0f ? 1.0f / dt : 60.0f;
    fpsAverage_ += (instantFps - fpsAverage_) * std::min(1.0f, dt * 2.0f);
    const bool freezeUiTimers = state_ == GameState::Generating || state_ == GameState::Paused ||
                                state_ == GameState::TwoPlayerPaused;
    if (freezeUiTimers) {
        lowFpsTime_ = 0.0f;
    } else {
        if (fpsAverage_ < cfg::LOW_FX_FPS_THRESHOLD) lowFpsTime_ += dt; else lowFpsTime_ = 0.0f;
        if (!lowFx_ && lowFpsTime_ >= cfg::LOW_FX_TRIGGER_SECONDS) { lowFx_ = true; toastTime_ = 3.0f; }
        if (toastTime_ > 0.0f) toastTime_ -= dt;
    }
    if (showControlsTest_) {
        wasdInput_.update(window_);
        arrowInput_.update(window_);
        return;
    }

    if (state_ == GameState::Menu) {
        menuTime_ += dt;
        return;
    }

    if (state_ == GameState::Generating) {
        int steps = (maze_.generationPhase() == GenerationPhase::BRAID)
            ? cfg::GENERATION_BRAID_STEPS_PER_FRAME : cfg::GENERATION_DFS_STEPS_PER_FRAME;
        steps = std::max(1, static_cast<int>(steps * generationSpeed_));
        for (int i = 0; i < steps && maze_.stepGenerate(); ++i) {}
        if (maze_.generationPhase() == GenerationPhase::DONE) {
            finishGeneratedRound();
            if (!generationForTwoPlayer_) state_ = GameState::Preview;
        }
        return;
    }

    if (state_ == GameState::Paused || state_ == GameState::GameOver ||
        state_ == GameState::TwoPlayerPaused || state_ == GameState::TwoPlayerIntermission ||
        state_ == GameState::TwoPlayerMatchOver) {
        return;
    }

    if (state_ == GameState::TwoPlayerPreview || state_ == GameState::TwoPlayerPlaying) {
        updateTwoPlayer(dt);
        return;
    }

    // Single Player Updates
    roundElapsed_ += dt;
    pulseTime_ += dt;
    if (showBfsDemo_) bfsDemoElapsed_ += dt;

    bool playerCanMove = (roundElapsed_ >= cfg::MOVEMENT_FREEZE_TIME);

    Direction dirWASD = wasdInput_.update(window_);
    Direction dirArrows = arrowInput_.update(window_);
    Direction chosenDir = {0, 0};

    if (!dirWASD.isZero() && !dirArrows.isZero()) {
        chosenDir = (wasdInput_.lastPressTick() > arrowInput_.lastPressTick()) ? dirWASD : dirArrows;
    } else if (!dirWASD.isZero()) {
        chosenDir = dirWASD;
    } else {
        chosenDir = dirArrows;
    }

    int inputDr = 0, inputDc = 0;
    if (playerCanMove) {
        inputDr = chosenDir.dr;
        inputDc = chosenDir.dc;
    }
    updatePowerUps(dt, false);

    int prevPlayerR = player_.gridRow();
    int prevPlayerC = player_.gridCol();
    int prevHunterR = hunter_.gridRow();
    int prevHunterC = hunter_.gridCol();

    if (playerCanMove) {
        player_.update(dt, inputDr, inputDc, maze_);
        int levelFog = std::max(2, cfg::FOG_VISION_RADIUS - (level_ >= 4) - (level_ >= 7));
        player_.discoverAround(maze_, levelFog, cfg::FOG_LIGHT_CONE_EXTRA);

    }

    // Win condition check
    if (player_.gridRow() == maze_.exitRow() && player_.gridCol() == maze_.exitCol()) {
        won_ = true;
        finalSteps_ = player_.steps();
        finalTime_ = roundElapsed_;
        runScore_ += 1000 * level_ + std::max(0, static_cast<int>(600.0f - 5.0f * finalTime_));
        state_ = GameState::LevelCleared;
        gameOverMenu_.setSelectedIndex(0);
        return;
    }

    // Preview timer
    if (state_ == GameState::Preview) {
        if (roundElapsed_ >= cfg::MAP_PREVIEW_DURATION) {
            state_ = GameState::Playing;
        }
    }

    // Hunter pursuit logic
    float hunterWake = std::max(5.0f, cfg::HUNTER_WAKE_TIME - static_cast<float>(level_ - 1));
    float hunterAwakeLeft = hunterWake - roundElapsed_;
    bool hunterIsAwake = (hunterAwakeLeft <= 0.0f);

    if (state_ == GameState::Playing) {
        hunter_.update(dt, maze_, player_.gridRow(), player_.gridCol(), hunterIsAwake);
        if (showBfsDemo_ && hunter_.bfsRevision() != shownBfsRevision_) {
            shownBfsRevision_ = hunter_.bfsRevision();
            bfsDemoElapsed_ = 0.0f;
        }

        if (hunterIsAwake) {
            bool sameCell = (player_.gridRow() == hunter_.gridRow() && player_.gridCol() == hunter_.gridCol());
            bool swappedCells = (player_.gridRow() == prevHunterR && player_.gridCol() == prevHunterC &&
                                 hunter_.gridRow() == prevPlayerR && hunter_.gridCol() == prevPlayerC);
            float diffX = player_.posX() - hunter_.posX();
            float diffY = player_.posY() - hunter_.posY();
            float pixelDist = std::sqrt(diffX * diffX + diffY * diffY);
            bool closePixels = (pixelDist < cfg::CATCH_DISTANCE_FACTOR * cfg::TILE_SIZE);

            if (sameCell || swappedCells || closePixels) {
                won_ = false;
                finalSteps_ = player_.steps();
                finalTime_ = roundElapsed_;
                pendingNameIsTwoPlayer_ = false;
                pendingScore_ = runScore_;
                pendingDetail_ = "reached level " + std::to_string(level_);
                enteredName_.clear();
                state_ = GameState::NameEntry;
                return;
            }
        }
    }

    // Camera follow (lerp + clamp to maze bounds)
    float targetX = player_.posX() + cfg::TILE_SIZE / 2.0f;
    float targetY = player_.posY() + cfg::TILE_SIZE / 2.0f;
    float alpha = 1.0f - std::exp(-cfg::CAMERA_LERP_SPEED * dt);
    cameraPos_.x += (targetX - cameraPos_.x) * alpha;
    cameraPos_.y += (targetY - cameraPos_.y) * alpha;

    float mazePixelW = static_cast<float>(cfg::MAZE_COLS * cfg::TILE_SIZE);
    float mazePixelH = static_cast<float>(cfg::MAZE_ROWS * cfg::TILE_SIZE);
    float halfW = cfg::CAMERA_VIEW_W / 2.0f;
    float halfH = cfg::CAMERA_VIEW_H / 2.0f;

    float clampedX = (mazePixelW >= cfg::CAMERA_VIEW_W)
        ? clampVal(cameraPos_.x, halfW, mazePixelW - halfW)
        : (mazePixelW / 2.0f);
    float clampedY = (mazePixelH >= cfg::CAMERA_VIEW_H)
        ? clampVal(cameraPos_.y, halfH, mazePixelH - halfH)
        : (mazePixelH / 2.0f);

    cameraView_.setCenter(clampedX, clampedY);

}

void Game::updateTwoPlayer(float dt) {
    roundElapsed_ += dt;
    pulseTime_ += dt;
    updatePowerUps(dt, true);

    // Movement timings:
    // Both frozen for first 4.0s
    // Runner unlocks at 4.0s
    // Hunter released at 4.0s + 3.0s = 7.0s
    bool runnerCanMove = (roundElapsed_ >= cfg::MOVEMENT_FREEZE_TIME);
    bool hunterReleased = (roundElapsed_ >= (cfg::MOVEMENT_FREEZE_TIME + cfg::TWO_P_HUNTER_RELEASE));

    if (state_ == GameState::TwoPlayerPreview) {
        if (roundElapsed_ >= cfg::MAP_PREVIEW_DURATION) {
            state_ = GameState::TwoPlayerPlaying;
        }
    }

    // Controls bound to player, never role: P1 = WASD, P2 = Arrows
    Direction dirP1 = wasdInput_.update(window_);
    Direction dirP2 = arrowInput_.update(window_);

    Direction runnerDir = (twoPlayerRound_ == 1) ? dirP1 : dirP2;
    Direction hunterDir = (twoPlayerRound_ == 1) ? dirP2 : dirP1;

    int rDr = runnerCanMove ? runnerDir.dr : 0;
    int rDc = runnerCanMove ? runnerDir.dc : 0;

    int hDr = hunterReleased ? hunterDir.dr : 0;
    int hDc = hunterReleased ? hunterDir.dc : 0;

    int prevRunnerR = tpRunner_.gridRow();
    int prevRunnerC = tpRunner_.gridCol();
    int prevHunterR = tpHunter_.gridRow();
    int prevHunterC = tpHunter_.gridCol();

    if (runnerCanMove) {
        tpRunner_.update(dt, rDr, rDc, maze_);
        tpRunner_.discoverAround(maze_, cfg::TWO_P_RUNNER_LOS_RADIUS, 0);
    }

    if (hunterReleased) {
        tpHunter_.updateHuman(dt, hDr, hDc, maze_, true);
        tpHunter_.discoverAround(maze_, cfg::TWO_P_HUNTER_LOS_RADIUS);
    }

    // Camera follow for both viewports
    float p1TargetX = (twoPlayerRound_ == 1) ? (tpRunner_.posX() + cfg::TILE_SIZE / 2.0f) : (tpHunter_.posX() + cfg::TILE_SIZE / 2.0f);
    float p1TargetY = (twoPlayerRound_ == 1) ? (tpRunner_.posY() + cfg::TILE_SIZE / 2.0f) : (tpHunter_.posY() + cfg::TILE_SIZE / 2.0f);

    float p2TargetX = (twoPlayerRound_ == 1) ? (tpHunter_.posX() + cfg::TILE_SIZE / 2.0f) : (tpRunner_.posX() + cfg::TILE_SIZE / 2.0f);
    float p2TargetY = (twoPlayerRound_ == 1) ? (tpHunter_.posY() + cfg::TILE_SIZE / 2.0f) : (tpRunner_.posY() + cfg::TILE_SIZE / 2.0f);

    float alpha = 1.0f - std::exp(-cfg::CAMERA_LERP_SPEED * dt);
    p1CameraPos_.x += (p1TargetX - p1CameraPos_.x) * alpha;
    p1CameraPos_.y += (p1TargetY - p1CameraPos_.y) * alpha;
    p2CameraPos_.x += (p2TargetX - p2CameraPos_.x) * alpha;
    p2CameraPos_.y += (p2TargetY - p2CameraPos_.y) * alpha;

    float mazePixelW = static_cast<float>(cfg::MAZE_COLS * cfg::TILE_SIZE);
    float mazePixelH = static_cast<float>(cfg::MAZE_ROWS * cfg::TILE_SIZE);
    float halfViewW = 400.0f / 2.0f;
    float halfViewH = 450.0f / 2.0f;

    p1View_.setCenter(clampVal(p1CameraPos_.x, halfViewW, mazePixelW - halfViewW),
                      clampVal(p1CameraPos_.y, halfViewH, mazePixelH - halfViewH));
    p2View_.setCenter(clampVal(p2CameraPos_.x, halfViewW, mazePixelW - halfViewW),
                      clampVal(p2CameraPos_.y, halfViewH, mazePixelH - halfViewH));

    auto finishRound = [&](RoundOutcome outcome) {
        int rScore = 0;
        int hScore = 0;
        float limit = cfg::TWO_P_ROUND_TIME_LIMIT;

        if (outcome == RoundOutcome::Escape) {
            rScore = cfg::TWO_P_SCORE_ESCAPE_BASE + static_cast<int>(cfg::TWO_P_SCORE_ESCAPE_TIME_FACTOR * (limit - roundElapsed_));
            hScore = 0;
        } else if (outcome == RoundOutcome::Caught) {
            rScore = static_cast<int>(cfg::TWO_P_SCORE_SURVIVAL_FACTOR * roundElapsed_);
            hScore = cfg::TWO_P_SCORE_CATCH_BASE + static_cast<int>(cfg::TWO_P_SCORE_CATCH_TIME_FACTOR * (limit - roundElapsed_));
        } else if (outcome == RoundOutcome::TimeOut) {
            rScore = static_cast<int>(cfg::TWO_P_SCORE_LIMIT_FACTOR * limit);
            hScore = 0;
        }

        if (twoPlayerRound_ == 1) {
            round1Data_.outcome = outcome;
            round1Data_.time = roundElapsed_;
            round1Data_.runnerScore = rScore;
            round1Data_.hunterScore = hScore;
            round1Data_.p1Score = rScore;
            round1Data_.p2Score = hScore;
            round1Data_.p1EscapeTime = (outcome == RoundOutcome::Escape) ? roundElapsed_ : -1.0f;
            round1Data_.p2EscapeTime = -1.0f;
            state_ = GameState::TwoPlayerIntermission;
        } else {
            round2Data_.outcome = outcome;
            round2Data_.time = roundElapsed_;
            round2Data_.runnerScore = rScore;
            round2Data_.hunterScore = hScore;
            round2Data_.p1Score = hScore; // P1 was hunter
            round2Data_.p2Score = rScore; // P2 was runner
            round2Data_.p2EscapeTime = (outcome == RoundOutcome::Escape) ? roundElapsed_ : -1.0f;
            int p1Total = round1Data_.p1Score + round2Data_.p1Score;
            int p2Total = round1Data_.p2Score + round2Data_.p2Score;
            pendingNameIsTwoPlayer_ = true;
            pendingScore_ = std::max(p1Total, p2Total);
            pendingDetail_ = std::string("P") + (p1Total >= p2Total ? "1" : "2") + " match total";
            enteredName_.clear();
            state_ = GameState::NameEntry;
            twoPlayerMatchOverMenu_.setSelectedIndex(0);
        }
    };

    // 1. Check Escape
    if (tpRunner_.gridRow() == maze_.exitRow() && tpRunner_.gridCol() == maze_.exitCol()) {
        finishRound(RoundOutcome::Escape);
        return;
    }

    // 2. Check Catch (only when hunter is released)
    if (hunterReleased) {
        bool sameCell = (tpRunner_.gridRow() == tpHunter_.gridRow() && tpRunner_.gridCol() == tpHunter_.gridCol());
        bool swappedCells = (tpRunner_.gridRow() == prevHunterR && tpRunner_.gridCol() == prevHunterC &&
                             tpHunter_.gridRow() == prevRunnerR && tpHunter_.gridCol() == prevRunnerC);
        float diffX = tpRunner_.posX() - tpHunter_.posX();
        float diffY = tpRunner_.posY() - tpHunter_.posY();
        float pixelDist = std::sqrt(diffX * diffX + diffY * diffY);
        bool closePixels = (pixelDist < cfg::CATCH_DISTANCE_FACTOR * cfg::TILE_SIZE);

        if (sameCell || swappedCells || closePixels) {
            finishRound(RoundOutcome::Caught);
            return;
        }
    }

    // 3. Check Time limit (90s)
    if (roundElapsed_ >= cfg::TWO_P_ROUND_TIME_LIMIT) {
        finishRound(RoundOutcome::TimeOut);
        return;
    }
}

void Game::render() {
    if (state_ == GameState::Menu) {
        window_.setView(window_.getDefaultView());
        if (subScreen_ == MenuSubScreen::Main) {
            renderMenu();
        } else if (subScreen_ == MenuSubScreen::Leaderboard) {
            renderLeaderboard();
        } else if (subScreen_ == MenuSubScreen::HowToPlay) {
            renderHowToPlay();
        }
    } else if (state_ == GameState::TwoPlayerPreview || state_ == GameState::TwoPlayerPlaying ||
               state_ == GameState::TwoPlayerPaused) {
        renderTwoPlayer();
        if (state_ == GameState::TwoPlayerPaused) {
            renderPause();
        }
    } else if (state_ == GameState::Generating) {
        window_.setView(window_.getDefaultView());
        renderGeneration();
    } else if (state_ == GameState::NameEntry) {
        window_.setView(window_.getDefaultView());
        renderNameEntry();
    } else if (state_ == GameState::LevelCleared) {
        window_.setView(window_.getDefaultView());
        UI::drawAnimatedBackground(window_, menuTime_);
        UI::drawText(window_, fontOrbitron_, "LEVEL CLEARED", {640, 220}, 48, Theme::Green, true);
        UI::drawText(window_, fontRajdhaniBold_, "LEVEL " + std::to_string(level_) + "   RUN SCORE " + std::to_string(runScore_), {640, 295}, 26, Theme::Text, true);
        UI::drawText(window_, fontRajdhaniMedium_, "ENTER / SPACE: NEXT LEVEL", {640, 380}, 20, Theme::Cyan, true);
    } else if (state_ == GameState::TwoPlayerIntermission) {
        window_.setView(window_.getDefaultView());
        renderTwoPlayerIntermission();
    } else if (state_ == GameState::TwoPlayerMatchOver) {
        window_.setView(window_.getDefaultView());
        renderTwoPlayerMatchOver();
    } else {
        // Single player renders
        window_.clear(Theme::Background);
        bool isPreview = state_ == GameState::Preview ||
                         (state_ == GameState::Paused && stateBeforePause_ == GameState::Preview);
        const sf::View& camera = isPreview ? previewView_ : cameraView_;
        drawPlayerView({0.0f, 0.0f, static_cast<float>(cfg::WINDOW_WIDTH), static_cast<float>(cfg::WINDOW_HEIGHT)},
                       PlayerRole::Runner, player_, hunter_, camera, isPreview, false, 1);

        if (state_ == GameState::Paused) {
            renderPause();
        } else if (state_ == GameState::GameOver) {
            renderGameOver();
        }
    }

    // Controls test overlay on top of any state if requested
    if (showControlsTest_) {
        window_.setView(window_.getDefaultView());
        renderControlsTestOverlay();
    }

    window_.display();
}

void Game::drawPlayerView(const sf::FloatRect& rect, PlayerRole role, const Player& runner,
                          const Hunter& hunter, const sf::View& camera, bool isPreview,
                          bool twoPlayer, int playerNumber) {
    sf::View view = camera;
    view.setViewport({rect.left / cfg::WINDOW_WIDTH, rect.top / cfg::WINDOW_HEIGHT,
                      rect.width / cfg::WINDOW_WIDTH, rect.height / cfg::WINDOW_HEIGHT});
    window_.setView(view);

    float pulse = (std::sin(pulseTime_ * cfg::PULSE_SPEED) + 1.0f) * 0.5f;
    if (mazeTextureReady_) window_.draw(mazeSprite_);
    renderExitPortal(pulse);
    if (!isPreview) renderFog(role, runner, hunter, twoPlayer);
    renderPowerUps();

    bool runnerVisible = isPreview || role == PlayerRole::Runner;
    if (!runnerVisible && role == PlayerRole::Hunter) {
        bool inSight = hunter.canSeeNow(runner.gridRow(), runner.gridCol());
        bool blipActive = twoPlayer && std::fmod(roundElapsed_, cfg::TWO_P_RUNNER_BLIP_CYCLE) < cfg::TWO_P_RUNNER_BLIP_DURATION;
        bool pingActive = twoPlayer &&
            ((playerNumber == 1 && p1Charge_ == PowerUpType::Ping && p1PowerTime_ > 0.0f) ||
             (playerNumber == 2 && p2Charge_ == PowerUpType::Ping && p2PowerTime_ > 0.0f));
        runnerVisible = inSight || blipActive || pingActive;
        if (!twoPlayer) runnerVisible = false;
    }
    if (runnerVisible) {
        sf::Vector2f center(runner.posX() + cfg::TILE_SIZE / 2.0f, runner.posY() + cfg::TILE_SIZE / 2.0f);
        drawAnimatedPlayer(window_, center, runner.facingDr(), runner.facingDc(),
                           runner.animFrame(), runner.isMoving(), pulse);
    }

    bool hunterVisible = isPreview || role == PlayerRole::Hunter;
    if (!hunterVisible && role == PlayerRole::Runner) {
        hunterVisible = runner.canSeeNow(hunter.gridRow(), hunter.gridCol());
        if (!twoPlayer) {
            hunterVisible = hunterVisible && std::fmod(pulseTime_, cfg::HUNTER_BLINK_CYCLE) < cfg::HUNTER_BLINK_ON_TIME;
        }
    }
    if (hunterVisible) {
        sf::Vector2f center(hunter.posX() + cfg::TILE_SIZE / 2.0f, hunter.posY() + cfg::TILE_SIZE / 2.0f);
        bool dormant = twoPlayer
            ? roundElapsed_ < cfg::MOVEMENT_FREEZE_TIME + cfg::TWO_P_HUNTER_RELEASE
            : roundElapsed_ < std::max(5.0f, cfg::HUNTER_WAKE_TIME - static_cast<float>(level_ - 1));
        drawAnimatedHunter(window_, center, hunter.animFrame(), hunter.isMoving(), pulse, dormant);
    }

    radialGradientSprite_.setPosition(view.getCenter().x - view.getSize().x / 2.0f,
                                      view.getCenter().y - view.getSize().y / 2.0f);
    radialGradientSprite_.setScale(view.getSize().x / static_cast<float>(cfg::RADIAL_GRADIENT_SIZE),
                                   view.getSize().y / static_cast<float>(cfg::RADIAL_GRADIENT_SIZE));
    if (!lowFx_) {
        float dx = runner.posX() - hunter.posX(), dy = runner.posY() - hunter.posY();
        float proximity = std::max(0.0f, 1.0f - std::sqrt(dx * dx + dy * dy) /
                                   (cfg::DANGER_DISTANCE_TILES * cfg::TILE_SIZE));
        radialGradientSprite_.setColor(sf::Color(255, 35, 65, static_cast<sf::Uint8>(proximity * 180)));
        window_.draw(radialGradientSprite_);
    }
    radialGradientSprite_.setColor(sf::Color::White);
    window_.draw(radialGradientSprite_);

    if (!twoPlayer && showBfsDemo_) renderBfsDemo();

    window_.setView(window_.getDefaultView());
    drawPlayerHud(rect, role, runner, hunter, isPreview, twoPlayer, playerNumber);
}

void Game::renderFog(PlayerRole role, const Player& runner, const Hunter& hunter, bool twoPlayer) {
    sf::VertexArray fog(sf::Quads, cfg::MAZE_ROWS * cfg::MAZE_COLS * 4);
    const float maxDistance = static_cast<float>(role == PlayerRole::Runner
        ? (twoPlayer ? cfg::TWO_P_RUNNER_LOS_RADIUS : cfg::FOG_VISION_RADIUS + cfg::FOG_LIGHT_CONE_EXTRA)
        : cfg::TWO_P_HUNTER_LOS_RADIUS);
    const int viewerRow = role == PlayerRole::Runner ? runner.gridRow() : hunter.gridRow();
    const int viewerCol = role == PlayerRole::Runner ? runner.gridCol() : hunter.gridCol();
    std::size_t index = 0;
    for (int r = 0; r < cfg::MAZE_ROWS; ++r) for (int c = 0; c < cfg::MAZE_COLS; ++c) {
        int alpha = 245;
        bool discovered = role == PlayerRole::Runner ? runner.isDiscovered(r, c) : hunter.isDiscovered(r, c);
        bool visible = role == PlayerRole::Runner ? runner.canSeeNow(r, c) : hunter.canSeeNow(r, c);
        if (discovered) alpha = 190;
        if (visible) {
            float dr = static_cast<float>(r - viewerRow), dc = static_cast<float>(c - viewerCol);
            alpha = static_cast<int>(std::min(145.0f, 18.0f + 125.0f * std::sqrt(dr*dr + dc*dc) / maxDistance));
        }
        sf::Color color(4, 8, 17, static_cast<sf::Uint8>(alpha));
        float x = c * cfg::TILE_SIZE, y = r * cfg::TILE_SIZE, s = cfg::TILE_SIZE;
        fog[index++] = sf::Vertex({x,y}, color); fog[index++] = sf::Vertex({x+s,y}, color);
        fog[index++] = sf::Vertex({x+s,y+s}, color); fog[index++] = sf::Vertex({x,y+s}, color);
    }
    window_.draw(fog);
}

void Game::renderExitPortal(float pulse) {
    sf::Vector2f center((maze_.exitCol() + 0.5f) * cfg::TILE_SIZE, (maze_.exitRow() + 0.5f) * cfg::TILE_SIZE);
    for (int ring = 0; ring < 2; ++ring) {
        sf::CircleShape shape(11.0f + ring * 4.0f, 32); shape.setOrigin(shape.getRadius(), shape.getRadius()); shape.setPosition(center);
        shape.setFillColor(sf::Color::Transparent); shape.setOutlineThickness(1.5f); shape.setOutlineColor(sf::Color(0, 255, 135, static_cast<sf::Uint8>(130 + pulse * 100)));
        shape.setRotation((ring ? -1.0f : 1.0f) * pulseTime_ * cfg::PORTAL_ROTATION_SPEED); window_.draw(shape);
    }
    sf::CircleShape core(5.0f + pulse * 3.0f); core.setOrigin(core.getRadius(), core.getRadius()); core.setPosition(center); core.setFillColor(sf::Color(0,255,135,180)); window_.draw(core);
}

void Game::spawnPowerUps(bool twoPlayer) {
    powerUps_.clear(); p1Charge_ = p2Charge_ = PowerUpType::None; p1PowerTime_ = p2PowerTime_ = 0.0f;
    std::vector<std::pair<int,int>> candidates;
    int runnerR = twoPlayer ? tpRunner_.gridRow() : player_.gridRow();
    int runnerC = twoPlayer ? tpRunner_.gridCol() : player_.gridCol();
    int hunterR = twoPlayer ? tpHunter_.gridRow() : hunter_.gridRow();
    int hunterC = twoPlayer ? tpHunter_.gridCol() : hunter_.gridCol();
    for (int r = 1; r < cfg::MAZE_ROWS - 1; ++r) for (int c = 1; c < cfg::MAZE_COLS - 1; ++c) {
        if (maze_.isWall(r,c) || std::abs(r-runnerR)+std::abs(c-runnerC) < 10 || std::abs(r-hunterR)+std::abs(c-hunterC) < 10) continue;
        int open = !maze_.isWall(r-1,c) + !maze_.isWall(r+1,c) + !maze_.isWall(r,c-1) + !maze_.isWall(r,c+1);
        if (open == 1) candidates.push_back({r,c});
    }
    if (candidates.empty()) return;
    powerUps_.push_back({PowerUpType::Surge, candidates[0].first, candidates[0].second, 0.0f});
    if (twoPlayer) {
        powerUps_.push_back({PowerUpType::Dash, candidates[candidates.size()/3].first, candidates[candidates.size()/3].second, 0.0f});
        powerUps_.push_back({PowerUpType::Ping, candidates[candidates.size()*2/3].first, candidates[candidates.size()*2/3].second, 0.0f});
    }
}

void Game::activatePowerUp(int playerNumber) {
    PowerUpType& charge = playerNumber == 1 ? p1Charge_ : p2Charge_;
    float& timer = playerNumber == 1 ? p1PowerTime_ : p2PowerTime_;
    if (charge == PowerUpType::None || timer > 0.0f) return;
    timer = charge == PowerUpType::Surge ? cfg::SURGE_DURATION : charge == PowerUpType::Dash ? cfg::DASH_DURATION : cfg::PING_DURATION;
}

void Game::updatePowerUps(float dt, bool twoPlayer) {
    p1PowerTime_ = std::max(0.0f, p1PowerTime_ - dt); p2PowerTime_ = std::max(0.0f, p2PowerTime_ - dt);
    if (!twoPlayer) player_.setSpeedMultiplier(p1Charge_ == PowerUpType::Surge && p1PowerTime_ > 0 ? cfg::SURGE_MULTIPLIER : 1.0f);
    else {
        float p1Mult = p1PowerTime_ > 0 && p1Charge_ == PowerUpType::Surge ? cfg::SURGE_MULTIPLIER : p1PowerTime_ > 0 && p1Charge_ == PowerUpType::Dash ? cfg::DASH_MULTIPLIER : 1.0f;
        float p2Mult = p2PowerTime_ > 0 && p2Charge_ == PowerUpType::Surge ? cfg::SURGE_MULTIPLIER : p2PowerTime_ > 0 && p2Charge_ == PowerUpType::Dash ? cfg::DASH_MULTIPLIER : 1.0f;
        if (twoPlayerRound_ == 1) { tpRunner_.setSpeedMultiplier(p1Mult); tpHunter_.setSpeedMultiplier(p2Mult); }
        else { tpHunter_.setSpeedMultiplier(p1Mult); tpRunner_.setSpeedMultiplier(p2Mult); }
    }
    for (auto& pickup : powerUps_) {
        if (pickup.respawn > 0) { pickup.respawn -= dt; continue; }
        auto consume = [&](int number, bool isRunner, int r, int c) {
            if (r != pickup.row || c != pickup.col) return;
            if ((pickup.type == PowerUpType::Surge && !isRunner) || ((pickup.type == PowerUpType::Dash || pickup.type == PowerUpType::Ping) && isRunner)) return;
            PowerUpType& charge = number == 1 ? p1Charge_ : p2Charge_;
            if (charge == PowerUpType::None) { charge = pickup.type; pickup.respawn = cfg::POWERUP_RESPAWN_SECONDS; }
        };
        if (!twoPlayer) consume(1, true, player_.gridRow(), player_.gridCol());
        else { consume(1, twoPlayerRound_ == 1, twoPlayerRound_ == 1 ? tpRunner_.gridRow() : tpHunter_.gridRow(), twoPlayerRound_ == 1 ? tpRunner_.gridCol() : tpHunter_.gridCol()); consume(2, twoPlayerRound_ != 1, twoPlayerRound_ == 1 ? tpHunter_.gridRow() : tpRunner_.gridRow(), twoPlayerRound_ == 1 ? tpHunter_.gridCol() : tpRunner_.gridCol()); }
    }
}

void Game::renderPowerUps() {
    for (const auto& pickup : powerUps_) if (pickup.respawn <= 0.0f) {
        sf::CircleShape icon(6.0f); icon.setOrigin(6,6); icon.setPosition((pickup.col+.5f)*cfg::TILE_SIZE, (pickup.row+.5f)*cfg::TILE_SIZE);
        icon.setFillColor(pickup.type == PowerUpType::Surge ? Theme::Cyan : pickup.type == PowerUpType::Dash ? Theme::Crimson : Theme::Amber); window_.draw(icon);
    }
}

void Game::renderTwoPlayer() {
    window_.clear(Theme::Background);

    bool isPreview = state_ == GameState::TwoPlayerPreview ||
                     (state_ == GameState::TwoPlayerPaused && stateBeforePause_ == GameState::TwoPlayerPreview);
    PlayerRole p1Role = (twoPlayerRound_ == 1) ? PlayerRole::Runner : PlayerRole::Hunter;
    PlayerRole p2Role = (twoPlayerRound_ == 1) ? PlayerRole::Hunter : PlayerRole::Runner;
    const sf::View& p1Camera = isPreview ? p1PreviewView_ : p1View_;
    const sf::View& p2Camera = isPreview ? p2PreviewView_ : p2View_;
    drawPlayerView({0.0f, 0.0f, cfg::WINDOW_WIDTH / 2.0f, static_cast<float>(cfg::WINDOW_HEIGHT)},
                   p1Role, tpRunner_, tpHunter_, p1Camera, isPreview, true, 1);
    drawPlayerView({cfg::WINDOW_WIDTH / 2.0f, 0.0f, cfg::WINDOW_WIDTH / 2.0f,
                    static_cast<float>(cfg::WINDOW_HEIGHT)},
                   p2Role, tpRunner_, tpHunter_, p2Camera, isPreview, true, 2);

    window_.setView(window_.getDefaultView());
    UI::drawPanel(window_, {cfg::WINDOW_WIDTH / 2.0f - cfg::TWO_P_DIVIDER_WIDTH / 2.0f, 0.0f,
                            cfg::TWO_P_DIVIDER_WIDTH, static_cast<float>(cfg::WINDOW_HEIGHT)},
                  Theme::Panel, Theme::PanelOutline, 1.0f);
}

void Game::renderTwoPlayerIntermission() {
    UI::drawAnimatedBackground(window_, menuTime_);

    UI::drawText(window_, fontOrbitron_, "ROUND 1 FINISHED",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 130.0f), 46, Theme::Cyan, true);

    std::string outcomeStr;
    sf::Color outcomeCol;
    if (round1Data_.outcome == RoundOutcome::Escape) {
        outcomeStr = "P1 (RUNNER) ESCAPED THE MAZE!";
        outcomeCol = Theme::Green;
    } else if (round1Data_.outcome == RoundOutcome::Caught) {
        outcomeStr = "P2 (HUNTER) CAUGHT P1!";
        outcomeCol = Theme::Crimson;
    } else {
        outcomeStr = "TIME LIMIT EXPIRED (90s)!";
        outcomeCol = Theme::Amber;
    }

    UI::drawText(window_, fontOrbitron_, outcomeStr,
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 190.0f), 24, outcomeCol, true);

    // Score Board Card
    UI::drawPanel(window_, sf::FloatRect(340.0f, 240.0f, 600.0f, 220.0f),
                 Theme::Panel, Theme::PanelOutline, 2.0f);

    std::ostringstream ssR1P1, ssR1P2, ssTime;
    ssR1P1 << "Player 1 (Runner): " << round1Data_.p1Score << " pts";
    ssR1P2 << "Player 2 (Hunter): " << round1Data_.p2Score << " pts";
    ssTime << "Round Duration: " << std::fixed << std::setprecision(1) << round1Data_.time << "s";

    UI::drawText(window_, fontRajdhaniBold_, ssR1P1.str(), sf::Vector2f(380.0f, 270.0f), 24, Theme::Cyan);
    UI::drawText(window_, fontRajdhaniBold_, ssR1P2.str(), sf::Vector2f(380.0f, 315.0f), 24, Theme::Crimson);
    UI::drawText(window_, fontRajdhaniMedium_, ssTime.str(), sf::Vector2f(380.0f, 360.0f), 20, Theme::TextDim);

    // Swap Announcement
    UI::drawPanel(window_, sf::FloatRect(320.0f, 490.0f, 640.0f, 110.0f),
                 sf::Color(18, 28, 48, 240), Theme::Cyan, 2.0f);

    UI::drawText(window_, fontOrbitron_, "P1 is now HUNTER, P2 is now RUNNER",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 525.0f), 22, Theme::Cyan, true);

    UI::drawText(window_, fontRajdhaniMedium_, "Controls stay bound: P1 (WASD)  |  P2 (Arrows). Same maze seed.",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 565.0f), 18, Theme::TextDim, true);

    UI::drawText(window_, fontRajdhaniBold_, "Press ENTER or SPACE to start Round 2  |  ESC to Menu",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 650.0f), 20, Theme::Green, true);
}

void Game::renderTwoPlayerMatchOver() {
    UI::drawAnimatedBackground(window_, menuTime_);

    UI::drawText(window_, fontOrbitron_, "MATCH COMPLETED",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 90.0f), 48, Theme::Cyan, true);

    int p1Total = round1Data_.p1Score + round2Data_.p1Score;
    int p2Total = round1Data_.p2Score + round2Data_.p2Score;

    std::string winnerText;
    sf::Color winnerColor = Theme::Cyan;
    if (p1Total > p2Total) {
        winnerText = "PLAYER 1 WINS THE MATCH!";
        winnerColor = Theme::Cyan;
    } else if (p2Total > p1Total) {
        winnerText = "PLAYER 2 WINS THE MATCH!";
        winnerColor = Theme::Crimson;
    } else {
        // Tie breaker: faster escape time
        float t1 = round1Data_.p1EscapeTime;
        float t2 = round2Data_.p2EscapeTime;
        if (t1 > 0.0f && (t2 < 0.0f || t1 < t2)) {
            winnerText = "PLAYER 1 WINS! (Tie-breaker: Faster Escape)";
            winnerColor = Theme::Cyan;
        } else if (t2 > 0.0f && (t1 < 0.0f || t2 < t1)) {
            winnerText = "PLAYER 2 WINS! (Tie-breaker: Faster Escape)";
            winnerColor = Theme::Crimson;
        } else {
            winnerText = "THE MATCH IS A TIE!";
            winnerColor = Theme::Amber;
        }
    }

    UI::drawText(window_, fontOrbitron_, winnerText,
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 150.0f), 28, winnerColor, true);

    // Breakdown Panel
    UI::drawPanel(window_, sf::FloatRect(280.0f, 200.0f, 720.0f, 260.0f),
                 Theme::Panel, Theme::PanelOutline, 2.0f);

    std::ostringstream ssR1, ssR2, ssTotals;
    ssR1 << "ROUND 1: P1 (Runner) " << round1Data_.p1Score << " pts  |  P2 (Hunter) " << round1Data_.p2Score << " pts";
    ssR2 << "ROUND 2: P1 (Hunter) " << round2Data_.p1Score << " pts  |  P2 (Runner) " << round2Data_.p2Score << " pts";
    ssTotals << "FINAL TOTALS ->  P1: " << p1Total << " pts   |   P2: " << p2Total << " pts";

    UI::drawText(window_, fontRajdhaniBold_, ssR1.str(), sf::Vector2f(320.0f, 240.0f), 22, Theme::Text);
    UI::drawText(window_, fontRajdhaniBold_, ssR2.str(), sf::Vector2f(320.0f, 295.0f), 22, Theme::Text);

    // Separator
    UI::drawPanel(window_, sf::FloatRect(320.0f, 350.0f, 640.0f, 2.0f), Theme::PanelOutline);
    UI::drawText(window_, fontOrbitron_, ssTotals.str(), sf::Vector2f(320.0f, 385.0f), 22, winnerColor);

    twoPlayerMatchOverMenu_.draw(window_, fontRajdhaniBold_, fontRajdhaniMedium_,
                                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 520.0f), 60.0f, 360.0f);
}

void Game::renderControlsTestOverlay() {
    // Dim overlay
    UI::drawPanel(window_, sf::FloatRect(0.0f, 0.0f, static_cast<float>(cfg::WINDOW_WIDTH), static_cast<float>(cfg::WINDOW_HEIGHT)),
                 sf::Color(7, 10, 18, 245));

    UI::drawText(window_, fontOrbitron_, "CONTROLS & KEYBOARD GHOSTING TEST",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 80.0f), 32, Theme::Cyan, true);

    UI::drawText(window_, fontRajdhaniMedium_, "Verify that multiple keys register simultaneously without laptop hardware ghosting.",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 125.0f), 18, Theme::TextDim, true);

    auto drawKeyBox = [&](float x, float y, const std::string& label, bool isDown, sf::Color activeCol) {
        sf::FloatRect box(x - 28.0f, y - 28.0f, 56.0f, 56.0f);
        sf::Color bg = isDown ? activeCol : Theme::Panel;
        sf::Color border = isDown ? sf::Color::White : Theme::PanelOutline;
        sf::Color textCol = isDown ? sf::Color(10, 15, 25) : Theme::Text;

        UI::drawPanel(window_, box, bg, border, isDown ? 2.5f : 1.5f);
        UI::drawText(window_, fontOrbitron_, label, sf::Vector2f(x, y), 22, textCol, true);
    };

    // 1. Player 1 Panel (WASD)
    float p1CenterX = 360.0f;
    UI::drawPanel(window_, sf::FloatRect(p1CenterX - 220.0f, 170.0f, 440.0f, 370.0f),
                 Theme::Panel, Theme::Cyan, 2.0f);

    UI::drawText(window_, fontOrbitron_, "PLAYER 1: WASD", sf::Vector2f(p1CenterX, 205.0f), 22, Theme::Cyan, true);

    bool wDown = wasdInput_.isKeyUpDown();
    bool aDown = wasdInput_.isKeyLeftDown();
    bool sDown = wasdInput_.isKeyDownDown();
    bool dDown = wasdInput_.isKeyRightDown();

    drawKeyBox(p1CenterX, 290.0f, "W", wDown, Theme::Cyan);
    drawKeyBox(p1CenterX - 68.0f, 360.0f, "A", aDown, Theme::Cyan);
    drawKeyBox(p1CenterX, 360.0f, "S", sDown, Theme::Cyan);
    drawKeyBox(p1CenterX + 68.0f, 360.0f, "D", dDown, Theme::Cyan);

    std::string p1DirStr = "Resolved: " + wasdInput_.directionString();
    UI::drawText(window_, fontRajdhaniBold_, p1DirStr, sf::Vector2f(p1CenterX, 445.0f), 22, Theme::Text, true);

    std::ostringstream ssP1Held;
    ssP1Held << "Held Keys: " << wasdInput_.heldKeyCount();
    UI::drawText(window_, fontRajdhaniMedium_, ssP1Held.str(), sf::Vector2f(p1CenterX, 485.0f), 18, Theme::TextDim, true);

    // 2. Player 2 Panel (Arrows)
    float p2CenterX = 920.0f;
    UI::drawPanel(window_, sf::FloatRect(p2CenterX - 220.0f, 170.0f, 440.0f, 370.0f),
                 Theme::Panel, Theme::Crimson, 2.0f);

    UI::drawText(window_, fontOrbitron_, "PLAYER 2: ARROWS", sf::Vector2f(p2CenterX, 205.0f), 22, Theme::Crimson, true);

    bool upDown = arrowInput_.isKeyUpDown();
    bool leftDown = arrowInput_.isKeyLeftDown();
    bool downDown = arrowInput_.isKeyDownDown();
    bool rightDown = arrowInput_.isKeyRightDown();

    drawKeyBox(p2CenterX, 290.0f, "^", upDown, Theme::Crimson);
    drawKeyBox(p2CenterX - 68.0f, 360.0f, "<", leftDown, Theme::Crimson);
    drawKeyBox(p2CenterX, 360.0f, "v", downDown, Theme::Crimson);
    drawKeyBox(p2CenterX + 68.0f, 360.0f, ">", rightDown, Theme::Crimson);

    std::string p2DirStr = "Resolved: " + arrowInput_.directionString();
    UI::drawText(window_, fontRajdhaniBold_, p2DirStr, sf::Vector2f(p2CenterX, 445.0f), 22, Theme::Text, true);

    std::ostringstream ssP2Held;
    ssP2Held << "Held Keys: " << arrowInput_.heldKeyCount();
    UI::drawText(window_, fontRajdhaniMedium_, ssP2Held.str(), sf::Vector2f(p2CenterX, 485.0f), 18, Theme::TextDim, true);

    // 3. Simultaneous Status Bar
    bool bothActive = (!wasdInput_.currentDirection().isZero() && !arrowInput_.currentDirection().isZero());
    std::string simStatus;
    sf::Color simColor;
    if (bothActive) {
        simStatus = "SIMULTANEOUS DUAL INPUT ACTIVE - NO HARDWARE GHOSTING DETECTED";
        simColor = Theme::Green;
    } else if (!wasdInput_.currentDirection().isZero() || !arrowInput_.currentDirection().isZero()) {
        simStatus = "Single input active. Press WASD and Arrow Keys simultaneously to verify dual registration.";
        simColor = Theme::Amber;
    } else {
        simStatus = "Waiting for keypresses. Press keys on both sides simultaneously.";
        simColor = Theme::TextDim;
    }

    UI::drawPanel(window_, sf::FloatRect(200.0f, 570.0f, 880.0f, 48.0f),
                 Theme::Panel, simColor, 1.5f);
    UI::drawText(window_, fontRajdhaniBold_, simStatus,
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 594.0f), 19, simColor, true);

    UI::drawText(window_, fontRajdhaniBold_, "Press F1 or ESC to close Controls Test",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 665.0f), 18, Theme::Cyan, true);
}

void Game::drawPlayerHud(const sf::FloatRect& rect, PlayerRole role, const Player& runner,
                         const Hunter& hunter, bool isPreview, bool twoPlayer, int playerNumber) {
    constexpr float padding = 16.0f;
    const float right = rect.left + rect.width;
    const float bottom = rect.top + rect.height;
    const sf::Color roleColor = role == PlayerRole::Runner ? Theme::Cyan : Theme::Crimson;
    const std::string roleName = role == PlayerRole::Runner ? "RUNNER" : "HUNTER";
    UI::drawPanel(window_, {rect.left, rect.top, rect.width, 48.0f},
                  sf::Color(14, 20, 34, 235), Theme::PanelOutline, 1.5f);
    const std::string roleLabel = (twoPlayer
        ? "P" + std::to_string(playerNumber) + " " + roleName + (playerNumber == 1 ? " WASD" : " ARROWS")
        : roleName);
    UI::drawText(window_, fontOrbitron_, roleLabel,
                 {rect.left + padding, rect.top + 14.0f}, 17, roleColor);

    std::ostringstream timer;
    if (twoPlayer) {
        timer << "R" << twoPlayerRound_ << "/2  TIME " << std::fixed << std::setprecision(1)
              << std::max(0.0f, cfg::TWO_P_ROUND_TIME_LIMIT - roundElapsed_) << "s";
    } else {
        timer << "TIME " << std::fixed << std::setprecision(1) << roundElapsed_ << "s  |  STEPS " << runner.steps();
    }
    UI::drawText(window_, fontRajdhaniBold_, timer.str(),
                 {rect.left + rect.width * 0.50f, rect.top + 15.0f}, 18, Theme::Text, true);

    std::string status;
    sf::Color statusColor = Theme::TextDim;
    if (role == PlayerRole::Hunter) {
        float releaseAt = twoPlayer ? cfg::MOVEMENT_FREEZE_TIME + cfg::TWO_P_HUNTER_RELEASE
                                    : std::max(5.0f, cfg::HUNTER_WAKE_TIME - static_cast<float>(level_ - 1));
        if (roundElapsed_ < releaseAt) {
            std::ostringstream value;
            value << "WAKES " << std::fixed << std::setprecision(1) << releaseAt - roundElapsed_ << "s";
            status = value.str(); statusColor = Theme::Amber;
        } else { status = "HUNTING"; statusColor = Theme::Crimson; }
    } else if (twoPlayer) {
        status = "FIND THE EXIT"; statusColor = Theme::Green;
    } else {
        status = "SEED " + std::to_string(maze_.seed());
    }
    UI::drawText(window_, fontRajdhaniBold_, status,
                 {right - padding - 68.0f, rect.top + 15.0f}, 15, statusColor, true);

    const PowerUpType charge = playerNumber == 1 ? p1Charge_ : p2Charge_;
    const float powerTime = playerNumber == 1 ? p1PowerTime_ : p2PowerTime_;
    const char* activation = twoPlayer ? (playerNumber == 1 ? "SPACE" : "ENTER") : "SPACE";
    const std::string powerName = charge == PowerUpType::Surge ? "SURGE" : charge == PowerUpType::Dash ? "DASH" : charge == PowerUpType::Ping ? "PING" : "EMPTY";
    const float slotX = rect.left + padding;
    const float slotY = bottom - 54.0f;
    UI::drawPanel(window_, {slotX, slotY, 150.0f, 38.0f}, Theme::Panel,
                  powerTime > 0.0f ? roleColor : Theme::PanelOutline, 2.0f);
    UI::drawText(window_, fontRajdhaniBold_, powerName + " [" + activation + "]",
                 {slotX + 10.0f, slotY + 10.0f}, 15, powerTime > 0.0f ? roleColor : Theme::TextDim);

    UI::drawText(window_, fontRajdhaniMedium_, lowFx_ ? "F2 LOW FX" : "F2 FX HIGH",
                 {slotX, slotY - 23.0f}, 14, lowFx_ ? Theme::Amber : Theme::TextMuted);
    if (showFps_) UI::drawText(window_, fontRajdhaniBold_, "FPS " + std::to_string(static_cast<int>(fpsAverage_)),
                              {slotX + 100.0f, slotY - 23.0f}, 14, Theme::Green);
    if (toastTime_ > 0.0f) UI::drawText(window_, fontRajdhaniBold_, lowFx_ ? "LOW FX ENABLED" : "FX MODE CHANGED",
                                        {rect.left + rect.width * 0.5f, rect.top + 62.0f}, 15, Theme::Amber, true);

    const float mapSize = cfg::MINIMAP_SIZE;
    const float mapX = right - mapSize - cfg::MINIMAP_PADDING;
    const float mapY = bottom - mapSize - cfg::MINIMAP_PADDING;
    UI::drawPanel(window_, {mapX - 4.0f, mapY - 4.0f, mapSize + 8.0f, mapSize + 8.0f},
                  Theme::Panel, Theme::PanelOutline, 2.0f);
    UI::drawText(window_, fontRajdhaniBold_, "MINIMAP", {mapX, mapY - 20.0f}, 14, Theme::TextMuted);
    const float cellW = mapSize / static_cast<float>(cfg::MAZE_COLS);
    const float cellH = mapSize / static_cast<float>(cfg::MAZE_ROWS);
    const int viewerRow = role == PlayerRole::Runner ? runner.gridRow() : hunter.gridRow();
    const int viewerCol = role == PlayerRole::Runner ? runner.gridCol() : hunter.gridCol();
    auto discovered = [&](int row, int col) {
        return role == PlayerRole::Runner ? runner.isDiscovered(row, col) : hunter.isDiscovered(row, col);
    };
    for (int r = 0; r < cfg::MAZE_ROWS; ++r) for (int c = 0; c < cfg::MAZE_COLS; ++c) {
        if (!isPreview && !discovered(r, c)) continue;
        sf::RectangleShape tile({cellW, cellH});
        tile.setPosition(mapX + c * cellW, mapY + r * cellH);
        tile.setFillColor(maze_.isWall(r, c) ? sf::Color(25, 35, 55) : sf::Color(10, 15, 26));
        window_.draw(tile);
    }
    if (isPreview || discovered(maze_.exitRow(), maze_.exitCol())) {
        sf::RectangleShape exit({cellW * 1.6f, cellH * 1.6f});
        exit.setPosition(mapX + maze_.exitCol() * cellW, mapY + maze_.exitRow() * cellH);
        exit.setFillColor(Theme::Green); window_.draw(exit);
    }
    sf::CircleShape marker(cellW * 0.9f);
    marker.setOrigin(cellW * 0.9f, cellW * 0.9f);
    marker.setPosition(mapX + (viewerCol + 0.5f) * cellW, mapY + (viewerRow + 0.5f) * cellH);
    marker.setFillColor(roleColor); window_.draw(marker);

    bool opponentVisible = isPreview;
    int opponentRow = role == PlayerRole::Runner ? hunter.gridRow() : runner.gridRow();
    int opponentCol = role == PlayerRole::Runner ? hunter.gridCol() : runner.gridCol();
    if (!opponentVisible) {
        opponentVisible = role == PlayerRole::Runner
            ? runner.canSeeNow(opponentRow, opponentCol)
            : hunter.canSeeNow(opponentRow, opponentCol);
        if (role == PlayerRole::Hunter && twoPlayer) {
            opponentVisible = opponentVisible ||
                std::fmod(roundElapsed_, cfg::TWO_P_RUNNER_BLIP_CYCLE) < cfg::TWO_P_RUNNER_BLIP_DURATION ||
                (playerNumber == 1 ? p1Charge_ == PowerUpType::Ping && p1PowerTime_ > 0.0f
                                   : p2Charge_ == PowerUpType::Ping && p2PowerTime_ > 0.0f);
        }
        if (role == PlayerRole::Runner && !twoPlayer) {
            opponentVisible = opponentVisible &&
                std::fmod(pulseTime_, cfg::HUNTER_BLINK_CYCLE) < cfg::HUNTER_BLINK_ON_TIME;
        }
    }
    if (opponentVisible) {
        marker.setPosition(mapX + (opponentCol + 0.5f) * cellW, mapY + (opponentRow + 0.5f) * cellH);
        marker.setFillColor(role == PlayerRole::Runner ? Theme::Crimson : Theme::Cyan);
        window_.draw(marker);
    }

    if (isPreview) {
        const float previewLeft = std::max(0.0f, cfg::MAP_PREVIEW_DURATION - roundElapsed_);
        if (previewLeft > 0.0f) {
            const bool unlocked = roundElapsed_ >= cfg::MOVEMENT_FREEZE_TIME;
            const sf::Color accent = unlocked ? Theme::Green : Theme::Cyan;
            const float panelW = std::min(360.0f, rect.width - 2.0f * padding);
            const float panelX = rect.left + (rect.width - panelW) / 2.0f;
            const float panelY = rect.top + rect.height * 0.38f;
            UI::drawPanel(window_, {panelX, panelY, panelW, 140.0f}, sf::Color(8, 12, 22, 235), accent, 2.5f);
            UI::drawText(window_, fontOrbitron_, std::to_string(static_cast<int>(std::ceil(previewLeft))),
                         {rect.left + rect.width / 2.0f, panelY + 48.0f}, 54, accent, true);
            UI::drawText(window_, fontRajdhaniBold_, unlocked ? "MOVE UNLOCKED - RUN" : "MEMORIZE THE MAZE",
                         {rect.left + rect.width / 2.0f, panelY + 112.0f}, 17, Theme::Text, true);
        }
    }
}

void Game::renderMenu() {
    UI::drawAnimatedBackground(window_, menuTime_);

    UI::drawText(window_, fontOrbitron_, "MAZE CHASE",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 130.0f), 64, Theme::Cyan, true);

    UI::drawText(window_, fontRajdhaniMedium_, "CYBER-CHASE ESCAPE SIMULATOR",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 185.0f), 18, Theme::TextDim, true);

    mainMenu_.draw(window_, fontRajdhaniBold_, fontRajdhaniMedium_,
                   sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 260.0f), 55.0f, 440.0f);

    UI::drawText(window_, fontRajdhaniMedium_, "W/S or UP/DOWN to Navigate  |  ENTER to Select  |  F1 Controls Test",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 665.0f), 16, Theme::TextMuted, true);
}

void Game::renderLeaderboard() {
    UI::drawAnimatedBackground(window_, menuTime_);

    UI::drawText(window_, fontOrbitron_, "LEADERBOARD",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 160.0f), 48, Theme::Cyan, true);

    UI::drawPanel(window_, sf::FloatRect(55.0f, 210.0f, 560.0f, 430.0f),
                 Theme::Panel, Theme::PanelOutline, 2.0f);
    UI::drawText(window_, fontRajdhaniBold_, "TOP 10", sf::Vector2f(80, 230), 23, Theme::Amber);
    float y = 265.0f;
    int rank = 1;
    for (const auto& entry : leaderboard_.top()) {
        std::ostringstream row;
        row << rank++ << ". " << entry.name << "   " << entry.score << "   " << entry.mode << "  " << entry.detail;
        UI::drawText(window_, fontRajdhaniMedium_, row.str(), sf::Vector2f(82, y), 18, Theme::Text);
        y += 32.0f;
    }
    UI::drawPanel(window_, sf::FloatRect(650.0f, 210.0f, 575.0f, 430.0f), Theme::Panel, Theme::PanelOutline, 2.0f);
    UI::drawText(window_, fontRajdhaniBold_, "BST VIEW", sf::Vector2f(675, 230), 23, Theme::Cyan);
    std::ostringstream stats;
    stats << "Nodes: " << leaderboard_.tree().size() << "   Height: " << leaderboard_.tree().height();
    UI::drawText(window_, fontRajdhaniMedium_, stats.str(), sf::Vector2f(675, 260), 18, Theme::Text);
    const auto* root = leaderboard_.tree().getRoot();
    std::function<void(const dsa_lib::BST<LeaderboardEntry, LeaderboardCompare>::Node*, float, float, int)> drawTree;
    drawTree = [&](const auto* node, float x, float ty, int level) {
        if (!node || level >= 3) return;
        float offset = 145.0f / static_cast<float>(level + 1);
        if (node->left) { sf::Vertex line[] = {sf::Vertex({x, ty}), sf::Vertex({x-offset, ty+75})}; window_.draw(line, 2, sf::Lines); }
        if (node->right) { sf::Vertex line[] = {sf::Vertex({x, ty}), sf::Vertex({x+offset, ty+75})}; window_.draw(line, 2, sf::Lines); }
        drawTree(node->left, x-offset, ty+75, level+1); drawTree(node->right, x+offset, ty+75, level+1);
        sf::CircleShape circle(24); circle.setOrigin(24,24); circle.setPosition(x,ty); circle.setFillColor(Theme::Background); circle.setOutlineColor(Theme::Cyan); circle.setOutlineThickness(2); window_.draw(circle);
        UI::drawText(window_, fontRajdhaniBold_, std::to_string(node->data.score), {x,ty}, 14, Theme::Text, true);
    };
    drawTree(root, 937, 340, 0);
    std::ostringstream sequence; sequence << (leaderboardTraversal_ == 1 ? "IN" : leaderboardTraversal_ == 2 ? "PRE" : "POST") << ": ";
    for (const auto& e : leaderboard_.traversal(leaderboardTraversal_)) sequence << e.score << " ";
    UI::drawText(window_, fontRajdhaniMedium_, sequence.str(), {675, 585}, 16, Theme::TextDim);
    UI::drawText(window_, fontRajdhaniMedium_, "1 In-order  2 Pre-order  3 Post-order", {675, 610}, 15, Theme::TextMuted);

    UI::drawText(window_, fontRajdhaniBold_, "Press ESC or ENTER to return",
                 sf::Vector2f(640.0f, 430.0f), 18, Theme::Cyan, true);
}

void Game::renderHowToPlay() {
    UI::drawAnimatedBackground(window_, menuTime_);

    UI::drawText(window_, fontOrbitron_, "HOW TO PLAY",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 110.0f), 44, Theme::Cyan, true);

    UI::drawPanel(window_, sf::FloatRect(270.0f, 170.0f, 740.0f, 440.0f),
                 Theme::Panel, Theme::PanelOutline, 2.0f);

    float lineY = 195.0f;
    float stepY = 48.0f;

    UI::drawText(window_, fontRajdhaniBold_, "[1] SINGLE PLAYER", sf::Vector2f(300.0f, lineY), 18, Theme::Cyan);
    UI::drawText(window_, fontRajdhaniMedium_, "Navigate the maze to the Exit Gate. The AI Hunter awakens at 8.0s and hunts via BFS.",
                 sf::Vector2f(300.0f, lineY + 18.0f), 15, Theme::Text);

    lineY += stepY;
    UI::drawText(window_, fontRajdhaniBold_, "[2] LOCAL 2-PLAYER SPLIT SCREEN", sf::Vector2f(300.0f, lineY), 18, Theme::Cyan);
    UI::drawText(window_, fontRajdhaniMedium_, "Best of 2 rounds on the same maze! P1 is Left (WASD), P2 is Right (Arrows). Roles swap round 2.",
                 sf::Vector2f(300.0f, lineY + 18.0f), 15, Theme::Text);

    lineY += stepY;
    UI::drawText(window_, fontRajdhaniBold_, "[3] 2-PLAYER RULES & TIMERS", sf::Vector2f(300.0f, lineY), 18, Theme::Amber);
    UI::drawText(window_, fontRajdhaniMedium_, "5s map preview. 4s freeze. Runner starts at 4s; Hunter released 3s later (7s). Limit: 90s.",
                 sf::Vector2f(300.0f, lineY + 18.0f), 15, Theme::Text);

    lineY += stepY;
    UI::drawText(window_, fontRajdhaniBold_, "[4] FOG OF WAR & RADAR", sf::Vector2f(300.0f, lineY), 18, Theme::Crimson);
    UI::drawText(window_, fontRajdhaniMedium_, "Runner sees tight LOS (r=3). Hunter sees wide LOS (r=6) + periodic runner blip every 1.5s.",
                 sf::Vector2f(300.0f, lineY + 18.0f), 15, Theme::Text);

    lineY += stepY;
    UI::drawText(window_, fontRajdhaniBold_, "[5] CONTROLS TEST (F1)", sf::Vector2f(300.0f, lineY), 18, Theme::Green);
    UI::drawText(window_, fontRajdhaniMedium_, "Press F1 anytime to verify simultaneous keypresses on a single keyboard without ghosting.",
                 sf::Vector2f(300.0f, lineY + 18.0f), 15, Theme::Text);

    UI::drawText(window_, fontRajdhaniBold_, "Press ESC or ENTER to return to Main Menu",
                 sf::Vector2f(640.0f, 650.0f), 18, Theme::Cyan, true);
}

void Game::renderPause() {
    UI::drawPanel(window_, sf::FloatRect(0, 0, static_cast<float>(cfg::WINDOW_WIDTH), static_cast<float>(cfg::WINDOW_HEIGHT)),
                 Theme::OverlayDim);

    UI::drawText(window_, fontOrbitron_, "PAUSED",
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 210.0f), 48, Theme::Cyan, true);

    UI::MenuList& menu = state_ == GameState::TwoPlayerPaused ? twoPlayerPauseMenu_ : pauseMenu_;
    menu.draw(window_, fontRajdhaniBold_, fontRajdhaniMedium_,
              sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 330.0f), 60.0f, 360.0f);
}

void Game::renderGameOver() {
    UI::drawPanel(window_, sf::FloatRect(0, 0, static_cast<float>(cfg::WINDOW_WIDTH), static_cast<float>(cfg::WINDOW_HEIGHT)),
                 Theme::OverlayDim);

    std::string titleStr = won_ ? "YOU ESCAPED!" : "CAUGHT!";
    sf::Color titleCol = won_ ? Theme::Green : Theme::Crimson;

    UI::drawText(window_, fontOrbitron_, titleStr,
                 sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 180.0f), 52, titleCol, true);

    UI::drawPanel(window_, sf::FloatRect(420.0f, 250.0f, 440.0f, 130.0f),
                 Theme::Panel, Theme::PanelOutline, 2.0f);

    std::ostringstream ssT, ssS, ssM;
    ssT << "Time: " << std::fixed << std::setprecision(1) << finalTime_ << "s";
    ssS << "Steps: " << finalSteps_;
    ssM << "Seed: " << maze_.seed();

    UI::drawText(window_, fontRajdhaniBold_, ssT.str(), sf::Vector2f(460.0f, 275.0f), 22, Theme::Text);
    UI::drawText(window_, fontRajdhaniBold_, ssS.str(), sf::Vector2f(680.0f, 275.0f), 22, Theme::Text);
    UI::drawText(window_, fontRajdhaniMedium_, ssM.str(), sf::Vector2f(570.0f, 330.0f), 20, Theme::TextDim);

    gameOverMenu_.draw(window_, fontRajdhaniBold_, fontRajdhaniMedium_,
                       sf::Vector2f(static_cast<float>(cfg::WINDOW_WIDTH) / 2.0f, 440.0f), 60.0f, 360.0f);
}

void Game::renderGeneration() {
    window_.clear(Theme::Background);
    const float size = 720.0f / static_cast<float>(cfg::MAZE_ROWS);
    for (int r = 0; r < cfg::MAZE_ROWS; ++r) for (int c = 0; c < cfg::MAZE_COLS; ++c) {
        sf::RectangleShape tile({size, size}); tile.setPosition(c * size, r * size);
        tile.setFillColor(maze_.isWall(r,c) ? sf::Color(20, 28, 43) : sf::Color(8, 14, 25)); window_.draw(tile);
    }
    sf::RectangleShape head({size, size}); head.setPosition(maze_.currentCol() * size, maze_.currentRow() * size); head.setFillColor(sf::Color(0, 220, 230, 180)); window_.draw(head);
    UI::drawPanel(window_, {750, 35, 490, 650}, Theme::Panel, Theme::PanelOutline, 2);
    UI::drawText(window_, fontOrbitron_, "STACK (LIFO)", {995, 75}, 28, Theme::Cyan, true);
    auto stack = maze_.generationStackTop(cfg::GENERATION_STACK_VISIBLE);
    float y = 115;
    for (size_t i = 0; i < stack.size(); ++i, y += 35) {
        UI::drawPanel(window_, {790, y, 410, 28}, i == 0 ? sf::Color(0, 90, 110) : Theme::Background, i == 0 ? Theme::Cyan : Theme::PanelOutline, 1);
        UI::drawText(window_, fontRajdhaniBold_, "(" + std::to_string(stack[i].first) + ", " + std::to_string(stack[i].second) + ")", {810, y + 4}, 17, Theme::Text);
    }
    std::ostringstream info; info << "pushes " << maze_.generationPushes() << "  pops " << maze_.generationPops()
        << "\ndepth " << stack.size() << "  max depth " << maze_.generationMaxDepth() << "\nstep " << maze_.generationStep();
    UI::drawText(window_, fontRajdhaniMedium_, info.str(), {790, 555}, 18, Theme::Text);
    std::string phase = maze_.generationPhase() == GenerationPhase::DFS_CARVE ? "DFS CARVE" : maze_.generationPhase() == GenerationPhase::BACKTRACK ? "BACKTRACK" : maze_.generationPhase() == GenerationPhase::BRAID ? "BRAID" : "MAZE READY";
    UI::drawText(window_, fontRajdhaniBold_, phase, {995, 635}, 22, Theme::Amber, true);
    UI::drawText(window_, fontRajdhaniMedium_, maze_.lastAction(), {995, 660}, 17, Theme::Text, true);
    UI::drawText(window_, fontRajdhaniMedium_, "SPACE skip   +/- speed", {995, 700}, 15, Theme::TextMuted, true);
}

void Game::renderNameEntry() {
    UI::drawAnimatedBackground(window_, menuTime_);
    UI::drawText(window_, fontOrbitron_, "NEW HIGH SCORE", {640, 175}, 42, Theme::Green, true);
    UI::drawText(window_, fontRajdhaniBold_, "Score: " + std::to_string(pendingScore_), {640, 240}, 28, Theme::Amber, true);
    UI::drawPanel(window_, {390, 300, 500, 95}, Theme::Panel, Theme::Cyan, 2);
    UI::drawText(window_, fontRajdhaniBold_, enteredName_ + "_", {640, 347}, 32, Theme::Text, true);
    UI::drawText(window_, fontRajdhaniMedium_, "Type your name (max 10) and press ENTER", {640, 450}, 18, Theme::TextMuted, true);
}

void Game::renderBfsDemo() {
    const BfsResult& bfs = hunter_.latestBfs();
    if (bfs.visitOrder.empty()) return;
    int visible = std::min(static_cast<int>(bfs.visitOrder.size()), static_cast<int>(bfs.visitOrder.size() * std::min(1.0f, bfsDemoElapsed_ / cfg::BFS_DEMO_WAVE_SECONDS)) + 1);
    for (int i = 0; i < visible; ++i) {
        float t = static_cast<float>(i) / std::max(1, visible - 1);
        sf::RectangleShape mark({static_cast<float>(cfg::TILE_SIZE), static_cast<float>(cfg::TILE_SIZE)});
        mark.setPosition(bfs.visitOrder[i].second * cfg::TILE_SIZE, bfs.visitOrder[i].first * cfg::TILE_SIZE);
        mark.setFillColor(sf::Color(static_cast<sf::Uint8>(0 + 110*t), static_cast<sf::Uint8>(210 - 130*t), 255, 90)); window_.draw(mark);
    }
    if (bfsDemoElapsed_ >= cfg::BFS_DEMO_WAVE_SECONDS) for (const auto& cell : bfs.path) {
        sf::RectangleShape mark({static_cast<float>(cfg::TILE_SIZE - 8), static_cast<float>(cfg::TILE_SIZE - 8)});
        mark.setPosition(cell.second * cfg::TILE_SIZE + 4, cell.first * cfg::TILE_SIZE + 4); mark.setFillColor(sf::Color(170, 80, 255, 210)); window_.draw(mark);
    }
    window_.setView(window_.getDefaultView());
    UI::drawPanel(window_, {930, 80, 300, 145}, sf::Color(6, 12, 24, 225), Theme::Cyan, 2);
    UI::drawText(window_, fontRajdhaniBold_, "DEMO OVERLAY", {950, 96}, 18, Theme::Cyan);
    std::ostringstream s; s << "nodes visited: " << bfs.nodesVisited << "\npath length: " << bfs.path.size() << "\nmax queue: " << bfs.maxQueueSize << "\n" << bfs.micros << " microseconds";
    UI::drawText(window_, fontRajdhaniMedium_, s.str(), {950, 122}, 16, Theme::Text);
}

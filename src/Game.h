#pragma once
#include <SFML/Graphics.hpp>
#include "Maze.h"
#include "Player.h"
#include "Hunter.h"
#include "Pathfinder.h"
#include "DirectionInput.h"
#include "UI.h"
#include "Leaderboard.h"
#include "Config.h"
#include <string>
#include <vector>

enum class GameState {
    Menu,
    Preview,
    Playing,
    Paused,
    GameOver,
    TwoPlayerPreview,
    TwoPlayerPlaying,
    TwoPlayerPaused,
    TwoPlayerIntermission,
    TwoPlayerMatchOver,
    Generating,
    NameEntry,
    LevelCleared
};

enum class MenuSubScreen {
    Main,
    Leaderboard,
    HowToPlay
};

enum class PlayerRole {
    Runner,
    Hunter
};

enum class RoundOutcome {
    Escape,
    Caught,
    TimeOut
};

struct TwoPlayerRoundData {
    RoundOutcome outcome = RoundOutcome::TimeOut;
    float time = 0.0f;
    int runnerScore = 0;
    int hunterScore = 0;
    int p1Score = 0;
    int p2Score = 0;
    float p1EscapeTime = -1.0f;
    float p2EscapeTime = -1.0f;
};

enum class PowerUpType { None, Surge, Dash, Ping };
struct PowerUpPickup { PowerUpType type = PowerUpType::None; int row = -1; int col = -1; float respawn = 0.0f; };

class Game {
public:
    explicit Game(unsigned int seed = 0);

    void run();

private:
    void processEvents();
    void update(float dt);
    void render();

    void resetRound(unsigned int seed = 0);
    void startTwoPlayerMatch(unsigned int seed = 0);
    void startTwoPlayerRound(int roundNum);
    void loadFonts();
    bool tryLoadFont(sf::Font& font, const std::string& assetPath, const std::vector<std::string>& fallbacks);

    // Single Player renders
    void renderWorld();
    void renderHUD();
    void renderMenu();
    void renderLeaderboard();
    void renderHowToPlay();
    void renderPause();
    void renderGameOver();
    void renderGeneration();
    void renderNameEntry();
    void renderBfsDemo();
    void finishGeneratedRound();
    void rebuildMazeTexture();
    void buildRadialGradient();
    void renderFog(const Player& viewer);
    void renderExitPortal(float pulse);
    void spawnPowerUps(bool twoPlayer);
    void updatePowerUps(float dt, bool twoPlayer);
    void renderPowerUps();
    void activatePowerUp(int playerNumber);

    // Two Player renders
    void updateTwoPlayer(float dt);
    void renderTwoPlayer();
    void renderTwoPlayerHalf(PlayerRole role, bool isPreview);
    void renderTwoPlayerHeaderAndHUD();
    void renderTwoPlayerIntermission();
    void renderTwoPlayerMatchOver();

    // Controls test overlay (F1 in any state)
    void renderControlsTestOverlay();

    // Window & Views
    sf::RenderWindow window_;
    sf::View cameraView_;
    sf::View previewView_;
    sf::Vector2f cameraPos_{0.0f, 0.0f};

    // 2-Player Views
    sf::View p1View_;
    sf::View p2View_;
    sf::View p1PreviewView_;
    sf::View p2PreviewView_;
    sf::Vector2f p1CameraPos_{0.0f, 0.0f};
    sf::Vector2f p2CameraPos_{0.0f, 0.0f};
    sf::RenderTexture mazeTexture_;
    sf::Sprite mazeSprite_;
    sf::Texture radialGradientTexture_;
    sf::Sprite radialGradientSprite_;
    bool mazeTextureReady_ = false;

    // Fonts
    sf::Font fontOrbitron_;
    sf::Font fontRajdhaniMedium_;
    sf::Font fontRajdhaniBold_;

    // Single Player Entities
    Maze maze_;
    Player player_;
    Hunter hunter_;

    // Two Player Entities
    Player tpRunner_;
    Hunter tpHunter_;

    // Directional Inputs (Bound to players, never roles)
    DirectionInput wasdInput_;   // P1 = WASD always
    DirectionInput arrowInput_;  // P2 = Arrow keys always

    // States
    GameState state_ = GameState::Menu;
    GameState stateBeforePause_ = GameState::Playing;
    MenuSubScreen subScreen_ = MenuSubScreen::Main;
    bool showControlsTest_ = false;
    bool showBfsDemo_ = false;
    float bfsDemoElapsed_ = 0.0f;
    unsigned long long shownBfsRevision_ = 0;
    float generationSpeed_ = 1.0f;
    bool generationForTwoPlayer_ = false;
    bool pendingNameIsTwoPlayer_ = false;
    std::string enteredName_;
    int pendingScore_ = 0;
    std::string pendingDetail_;
    Leaderboard leaderboard_;
    int leaderboardTraversal_ = 1;
    bool lowFx_ = false;
    bool showFps_ = false;
    float fpsAverage_ = 60.0f;
    float lowFpsTime_ = 0.0f;
    float toastTime_ = 0.0f;
    std::vector<PowerUpPickup> powerUps_;
    PowerUpType p1Charge_ = PowerUpType::None;
    PowerUpType p2Charge_ = PowerUpType::None;
    float p1PowerTime_ = 0.0f;
    float p2PowerTime_ = 0.0f;
    int level_ = 1;
    int runScore_ = 0;

    // Single Player variables
    bool won_ = false;
    unsigned int initialSeed_ = 0;
    float roundElapsed_ = 0.0f;
    float pulseTime_ = 0.0f;
    float menuTime_ = 0.0f;
    int finalSteps_ = 0;
    float finalTime_ = 0.0f;

    // Two Player Match Data
    unsigned int twoPlayerMatchSeed_ = 0;
    int twoPlayerRound_ = 1; // 1 or 2
    TwoPlayerRoundData round1Data_;
    TwoPlayerRoundData round2Data_;

    // Menu components
    UI::MenuList mainMenu_;
    UI::MenuList pauseMenu_;
    UI::MenuList gameOverMenu_;
    UI::MenuList twoPlayerPauseMenu_;
    UI::MenuList twoPlayerMatchOverMenu_;
};

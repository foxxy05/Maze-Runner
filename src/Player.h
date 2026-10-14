#pragma once
#include "Maze.h"
#include <vector>

#define DEFAULT_CELL_W 20
#define DEFAULT_CELL_H 16
#define DEFAULT_PLAYER_SPEED 150.0f
#define DEFAULT_VISION_RADIUS 3
#define DEFAULT_FORWARD_EXTRA 4
#define ANIM_FRAME_TIME 0.10f
#define TOTAL_ANIM_FRAMES 4
#define SNAP_THRESHOLD 1.0f

class Player {
public:
    Player(int startR, int startC, int cellW = DEFAULT_CELL_W, int cellH = DEFAULT_CELL_H);

    void update(float dt, int inputDr, int inputDc, const Maze& maze);

    void discoverAround(const Maze& maze, int radius = DEFAULT_VISION_RADIUS, int forwardExtra = DEFAULT_FORWARD_EXTRA);
    void resetDiscovery(int rows, int cols);
    bool isDiscovered(int r, int c) const;

    int gridRow() const { return gridR_; }
    int gridCol() const { return gridC_; }
    float posX() const { return posX_; }
    float posY() const { return posY_; }
    int facingDr() const { return facingDr_; }
    int facingDc() const { return facingDc_; }
    int steps() const { return steps_; }

    bool isMoving() const { return isMoving_; }
    int animFrame() const { return animFrame_; }

    void setCellSize(int cellW, int cellH) { cellW_ = cellW; cellH_ = cellH; }
    void setSpeed(float speed) { speed_ = speed; }

private:
    int gridR_, gridC_;
    int targetR_, targetC_;
    float posX_, posY_;
    int facingDr_ = 0, facingDc_ = 1;
    int steps_ = 0;

    bool isMoving_ = false;
    float animTimer_ = 0.0f;
    int animFrame_ = 0;

    int cellW_ = DEFAULT_CELL_W, cellH_ = DEFAULT_CELL_H;
    float speed_ = DEFAULT_PLAYER_SPEED;

    std::vector<std::vector<bool>> discovered_;
};

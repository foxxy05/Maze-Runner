#pragma once
#include "Maze.h"

#define DEFAULT_HUNTER_SPEED 125.0f
#define HUNTER_ANIM_FRAME_TIME 0.10f
#define HUNTER_TOTAL_FRAMES 4
#define HUNTER_SNAP_THRESHOLD 1.0f

class Hunter {
public:
    Hunter(int startR, int startC, int cellW = 20, int cellH = 16);

    void step(const Maze& maze, int playerR, int playerC);
    void update(float dt);

    int gridRow() const { return gridR_; }
    int gridCol() const { return gridC_; }
    float posX() const { return posX_; }
    float posY() const { return posY_; }

    bool isMoving() const { return isMoving_; }
    int animFrame() const { return animFrame_; }

    void setCellSize(int cellW, int cellH) { cellW_ = cellW; cellH_ = cellH; }
    void setSpeed(float speed) { speed_ = speed; }

private:
    int gridR_, gridC_;
    int targetR_, targetC_;
    float posX_, posY_;

    bool isMoving_ = false;
    float animTimer_ = 0.0f;
    int animFrame_ = 0;

    int cellW_ = 20, cellH_ = 16;
    float speed_ = DEFAULT_HUNTER_SPEED;
};

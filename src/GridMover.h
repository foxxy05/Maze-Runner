#pragma once
#include "Config.h"
#include "Maze.h"

class GridMover {
public:
    GridMover(int startR, int startC, float cellsPerSec,
              int cellW = cfg::TILE_SIZE, int cellH = cfg::TILE_SIZE);

    // Queries
    int gridRow() const { return gridR_; }
    int gridCol() const { return gridC_; }
    float posX() const { return posX_; }
    float posY() const { return posY_; }
    int facingDr() const { return facingDr_; }
    int facingDc() const { return facingDc_; }
    int steps() const { return steps_; }
    bool isMoving() const { return isMoving_; }
    int animFrame() const { return animFrame_; }
    float effectiveSpeed() const { return cellsPerSec_ * speedMultiplier_ * static_cast<float>(cellW_); }
    bool isAtCellCenter() const { return !isMoving_; }

    // Mutators
    void setCellsPerSec(float cps) { cellsPerSec_ = cps; }
    void setSpeedMultiplier(float mult) { speedMultiplier_ = mult; }
    void setAnimSettings(float frameTime, int totalFrames) {
        animFrameTime_ = frameTime;
        totalAnimFrames_ = totalFrames;
    }
    void setSnapThreshold(float threshold) { snapThreshold_ = threshold; }

    // Movement operations
    bool setTargetCell(int nextR, int nextC);
    void updateWithInput(float dt, int inputDr, int inputDc, const Maze& maze);
    bool updateTowardsTarget(float dt);

private:
    int gridR_, gridC_;
    int targetR_, targetC_;
    float posX_, posY_;
    int facingDr_ = 0, facingDc_ = 1;
    int steps_ = 0;

    bool isMoving_ = false;
    float animTimer_ = 0.0f;
    int animFrame_ = 0;

    int cellW_ = cfg::TILE_SIZE, cellH_ = cfg::TILE_SIZE;
    float cellsPerSec_ = cfg::PLAYER_CELLS_PER_SEC;
    float speedMultiplier_ = 1.0f; // Multiplier for temporary movement effects.
    float snapThreshold_ = cfg::SNAP_THRESHOLD;
    float animFrameTime_ = cfg::ANIM_FRAME_TIME;
    int totalAnimFrames_ = cfg::TOTAL_ANIM_FRAMES;
};

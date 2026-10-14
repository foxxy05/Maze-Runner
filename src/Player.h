#pragma once
#include "Maze.h"
#include "Config.h"
#include "GridMover.h"
#include <vector>

class Player {
public:
    Player(int startR, int startC, int cellW = cfg::TILE_SIZE, int cellH = cfg::TILE_SIZE);

    void update(float dt, int inputDr, int inputDc, const Maze& maze);

    void discoverAround(const Maze& maze, int radius = cfg::FOG_VISION_RADIUS, int forwardExtra = cfg::FOG_LIGHT_CONE_EXTRA);
    void resetDiscovery(int rows, int cols);
    bool isDiscovered(int r, int c) const;
    bool canSeeNow(int r, int c) const;

    int gridRow() const { return mover_.gridRow(); }
    int gridCol() const { return mover_.gridCol(); }
    float posX() const { return mover_.posX(); }
    float posY() const { return mover_.posY(); }
    int facingDr() const { return mover_.facingDr(); }
    int facingDc() const { return mover_.facingDc(); }
    int steps() const { return mover_.steps(); }

    bool isMoving() const { return mover_.isMoving(); }
    int animFrame() const { return mover_.animFrame(); }

    void setCellSize(int cellW, int cellH) { mover_.setCellSize(cellW, cellH); }
    void setCellsPerSec(float cps) { mover_.setCellsPerSec(cps); }
    void setSpeedMultiplier(float mult) { mover_.setSpeedMultiplier(mult); }

    GridMover& mover() { return mover_; }
    const GridMover& mover() const { return mover_; }

private:
    GridMover mover_;
    std::vector<std::vector<bool>> discovered_;
    std::vector<std::vector<bool>> visibleNow_;
};

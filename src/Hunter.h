#pragma once
#include "Maze.h"
#include "Config.h"
#include "GridMover.h"
#include <vector>

class Hunter {
public:
    Hunter(int startR, int startC, int cellW = cfg::TILE_SIZE, int cellH = cfg::TILE_SIZE,
           float cellsPerSec = cfg::HUNTER_CELLS_PER_SEC);

    void step(const Maze& maze, int playerR, int playerC);
    void update(float dt, const Maze& maze, int playerR, int playerC, bool isAwake);
    void updateHuman(float dt, int inputDr, int inputDc, const Maze& maze, bool isReleased);

    // Fog and Line-of-Sight for 2-Player
    void discoverAround(const Maze& maze, int radius = cfg::TWO_P_HUNTER_LOS_RADIUS);
    void resetDiscovery(int rows, int cols);
    bool isDiscovered(int r, int c) const;
    bool canSeeNow(int r, int c) const;

    int gridRow() const { return mover_.gridRow(); }
    int gridCol() const { return mover_.gridCol(); }
    float posX() const { return mover_.posX(); }
    float posY() const { return mover_.posY(); }
    bool isMoving() const { return mover_.isMoving(); }
    int animFrame() const { return mover_.animFrame(); }
    int facingDr() const { return mover_.facingDr(); }
    int facingDc() const { return mover_.facingDc(); }

    void setCellSize(int cellW, int cellH) { mover_.setCellSize(cellW, cellH); }
    void setCellsPerSec(float cps) { mover_.setCellsPerSec(cps); }
    void setSpeedMultiplier(float mult) { mover_.setSpeedMultiplier(mult); }

    GridMover& mover() { return mover_; }
    const GridMover& mover() const { return mover_; }

    int getAndResetCellsMoved() {
        int moved = cellsMovedLastSecond_;
        cellsMovedLastSecond_ = 0;
        return moved;
    }

private:
    GridMover mover_;
    int cellsMovedTotal_ = 0;
    int cellsMovedLastSecond_ = 0;
    int lastLoggedGridR_ = 0;
    int lastLoggedGridC_ = 0;

    std::vector<std::vector<bool>> discovered_;
    std::vector<std::vector<bool>> visibleNow_;
};

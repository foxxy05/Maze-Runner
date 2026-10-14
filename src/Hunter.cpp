#include "Hunter.h"
#include "Pathfinder.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

Hunter::Hunter(int startR, int startC, int cellW, int cellH, float cellsPerSec)
    : mover_(startR, startC, cellsPerSec, cellW, cellH),
      cellsMovedTotal_(0),
      cellsMovedLastSecond_(0),
      lastLoggedGridR_(startR),
      lastLoggedGridC_(startC),
      discovered_(),
      visibleNow_() {
    mover_.setAnimSettings(cfg::HUNTER_ANIM_FRAME_TIME, cfg::HUNTER_TOTAL_FRAMES);
    mover_.setSnapThreshold(cfg::HUNTER_SNAP_THRESHOLD);
}

void Hunter::step(const Maze& maze, int playerR, int playerC) {
    if (mover_.gridRow() == playerR && mover_.gridCol() == playerC) return;
    BfsResult res = Pathfinder::findPath(maze, {mover_.gridRow(), mover_.gridCol()}, {playerR, playerC});
    latestBfs_ = res;
    ++bfsRevision_;
    if (res.found && res.path.size() >= 2) {
        mover_.setTargetCell(res.path[1].first, res.path[1].second);
    }
}

void Hunter::update(float dt, const Maze& maze, int playerR, int playerC, bool isAwake) {
    if (!isAwake) {
        return;
    }

    // AI Hunter: If at cell center, compute next BFS step towards player
    if (mover_.isAtCellCenter()) {
        if (mover_.gridRow() != playerR || mover_.gridCol() != playerC) {
            BfsResult res = Pathfinder::findPath(maze, {mover_.gridRow(), mover_.gridCol()}, {playerR, playerC});
            latestBfs_ = res;
            ++bfsRevision_;
            if (res.found && res.path.size() >= 2) {
                mover_.setTargetCell(res.path[1].first, res.path[1].second);
            }
        }
    }

    if (mover_.isMoving()) {
        bool reached = mover_.updateTowardsTarget(dt);
        if (reached) {
            cellsMovedTotal_++;
            cellsMovedLastSecond_++;

            // Chain next cell immediately at cell center
            if (mover_.gridRow() != playerR || mover_.gridCol() != playerC) {
                BfsResult res = Pathfinder::findPath(maze, {mover_.gridRow(), mover_.gridCol()}, {playerR, playerC});
                latestBfs_ = res;
                ++bfsRevision_;
                if (res.found && res.path.size() >= 2) {
                    mover_.setTargetCell(res.path[1].first, res.path[1].second);
                }
            }
        }
    }
}

void Hunter::updateHuman(float dt, int inputDr, int inputDc, const Maze& maze, bool isReleased) {
    if (!isReleased) {
        return;
    }

    mover_.updateWithInput(dt, inputDr, inputDc, maze);

    if (mover_.gridRow() != lastLoggedGridR_ || mover_.gridCol() != lastLoggedGridC_) {
        cellsMovedTotal_++;
        cellsMovedLastSecond_++;
        lastLoggedGridR_ = mover_.gridRow();
        lastLoggedGridC_ = mover_.gridCol();
    }
}

void Hunter::resetDiscovery(int rows, int cols) {
    discovered_.assign(rows, std::vector<bool>(cols, false));
    visibleNow_.assign(rows, std::vector<bool>(cols, false));
}

void Hunter::discoverAround(const Maze& maze, int radius) {
    int rows = maze.rows();
    int cols = maze.cols();

    if (visibleNow_.empty() || visibleNow_.size() != static_cast<size_t>(rows) || visibleNow_[0].size() != static_cast<size_t>(cols)) {
        visibleNow_.assign(rows, std::vector<bool>(cols, false));
    } else {
        for (auto& row : visibleNow_) {
            std::fill(row.begin(), row.end(), false);
        }
    }

    if (discovered_.empty() || discovered_.size() != static_cast<size_t>(rows) || discovered_[0].size() != static_cast<size_t>(cols)) {
        discovered_.assign(rows, std::vector<bool>(cols, false));
    }

    int gridR = mover_.gridRow();
    int gridC = mover_.gridCol();

    if (maze.inBounds(gridR, gridC)) {
        visibleNow_[gridR][gridC] = true;
        discovered_[gridR][gridC] = true;
    }

    auto castRay = [&](int targetR, int targetC) {
        int r0 = gridR;
        int c0 = gridC;
        int r1 = targetR;
        int c1 = targetC;

        int dr = r1 - r0;
        int dc = c1 - c0;
        int maxDist = std::max(std::abs(dr), std::abs(dc));
        if (maxDist == 0) return;

        int numSamples = maxDist * 3;
        int prevR = r0;
        int prevC = c0;

        for (int step = 1; step <= numSamples; step++) {
            float t = static_cast<float>(step) / static_cast<float>(numSamples);
            int curR = static_cast<int>(std::floor(static_cast<float>(r0) + 0.5f + static_cast<float>(dr) * t));
            int curC = static_cast<int>(std::floor(static_cast<float>(c0) + 0.5f + static_cast<float>(dc) * t));

            if (!maze.inBounds(curR, curC)) break;
            if (curR == prevR && curC == prevC) continue;

            if (curR != prevR && curC != prevC) {
                if (maze.isWall(prevR, curC) && maze.isWall(curR, prevC)) {
                    break;
                }
            }

            visibleNow_[curR][curC] = true;
            discovered_[curR][curC] = true;

            if (maze.isWall(curR, curC)) {
                break;
            }

            prevR = curR;
            prevC = curC;
        }
    };

    // Circular LOS raycast
    for (int dr = -radius; dr <= radius; dr++) {
        for (int dc = -radius; dc <= radius; dc++) {
            if (dr * dr + dc * dc <= (radius + 1) * (radius + 1)) {
                int tr = gridR + dr;
                int tc = gridC + dc;
                if (maze.inBounds(tr, tc)) {
                    castRay(tr, tc);
                }
            }
        }
    }
}

bool Hunter::isDiscovered(int r, int c) const {
    if (r < 0 || r >= static_cast<int>(discovered_.size()) || c < 0 || c >= static_cast<int>(discovered_[0].size()))
        return false;
    return discovered_[r][c];
}

bool Hunter::canSeeNow(int r, int c) const {
    if (r < 0 || r >= static_cast<int>(visibleNow_.size()) || c < 0 || c >= static_cast<int>(visibleNow_[0].size()))
        return false;
    return visibleNow_[r][c];
}

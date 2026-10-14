#include "Player.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

Player::Player(int startR, int startC, int cellW, int cellH)
    : mover_(startR, startC, cfg::PLAYER_CELLS_PER_SEC, cellW, cellH),
      discovered_(),
      visibleNow_() {
    mover_.setAnimSettings(cfg::ANIM_FRAME_TIME, cfg::TOTAL_ANIM_FRAMES);
    mover_.setSnapThreshold(cfg::SNAP_THRESHOLD);
}

void Player::resetDiscovery(int rows, int cols) {
    discovered_.assign(rows, std::vector<bool>(cols, false));
    visibleNow_.assign(rows, std::vector<bool>(cols, false));
}

void Player::update(float dt, int inputDr, int inputDc, const Maze& maze) {
    mover_.updateWithInput(dt, inputDr, inputDc, maze);
}

void Player::discoverAround(const Maze& maze, int radius, int forwardExtra) {
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

    // 1. Raycast to each cell within the base radius
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

    // 2. Raycast in forward light cone (if forwardExtra > 0)
    int facingDr = mover_.facingDr();
    int facingDc = mover_.facingDc();
    if (forwardExtra > 0 && (facingDr != 0 || facingDc != 0)) {
        int maxDist = radius + forwardExtra;
        int latDr = facingDc;
        int latDc = facingDr;

        for (int dist = 1; dist <= maxDist; dist++) {
            int fr = gridR + facingDr * dist;
            int fc = gridC + facingDc * dist;

            if (maze.inBounds(fr, fc)) castRay(fr, fc);
            if (maze.inBounds(fr + latDr, fc + latDc)) castRay(fr + latDr, fc + latDc);
            if (maze.inBounds(fr - latDr, fc - latDc)) castRay(fr - latDr, fc - latDc);
        }
    }
}

bool Player::isDiscovered(int r, int c) const {
    if (r < 0 || r >= static_cast<int>(discovered_.size()) || c < 0 || c >= static_cast<int>(discovered_[0].size()))
        return false;
    return discovered_[r][c];
}

bool Player::canSeeNow(int r, int c) const {
    if (r < 0 || r >= static_cast<int>(visibleNow_.size()) || c < 0 || c >= static_cast<int>(visibleNow_[0].size()))
        return false;
    return visibleNow_[r][c];
}

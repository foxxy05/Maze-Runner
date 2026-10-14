#include "Player.h"
#include <cmath>
#include <algorithm>

Player::Player(int startR, int startC, int cellW, int cellH)
    : gridR_(startR), gridC_(startC), targetR_(startR), targetC_(startC),
      cellW_(cellW), cellH_(cellH), facingDr_(0), facingDc_(1), steps_(0) {
    posX_ = startC * cellW_;
    posY_ = startR * cellH_;
}

void Player::resetDiscovery(int rows, int cols) {
    discovered_.assign(rows, std::vector<bool>(cols, false));
}

void Player::update(float dt, int inputDr, int inputDc, const Maze& maze) {
    float targetX = targetC_ * cellW_;
    float targetY = targetR_ * cellH_;

    float dx = targetX - posX_;
    float dy = targetY - posY_;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist < SNAP_THRESHOLD) {
        posX_ = targetX;
        posY_ = targetY;
        gridR_ = targetR_;
        gridC_ = targetC_;

        if (inputDr != 0 || inputDc != 0) {
            facingDr_ = inputDr;
            facingDc_ = inputDc;

            int nextR = gridR_ + inputDr;
            int nextC = gridC_ + inputDc;

            if (!maze.isWall(nextR, nextC)) {
                targetR_ = nextR;
                targetC_ = nextC;
                isMoving_ = true;
                steps_++;
            } else {
                isMoving_ = false;
            }
        } else {
            isMoving_ = false;
        }
    } else {
        isMoving_ = true;
        float moveDist = speed_ * dt;
        if (moveDist >= dist) {
            posX_ = targetX;
            posY_ = targetY;
            gridR_ = targetR_;
            gridC_ = targetC_;
        } else {
            posX_ += (dx / dist) * moveDist;
            posY_ += (dy / dist) * moveDist;
        }
    }

    if (isMoving_) {
        animTimer_ += dt;
        if (animTimer_ >= ANIM_FRAME_TIME) {
            animTimer_ = 0.0f;
            animFrame_ = (animFrame_ + 1) % TOTAL_ANIM_FRAMES;
        }
    } else {
        animFrame_ = 0;
        animTimer_ = 0.0f;
    }
}

void Player::discoverAround(const Maze& maze, int radius, int forwardExtra) {
    for (int dr = -radius; dr <= radius; dr++) {
        for (int dc = -radius; dc <= radius; dc++) {
            int r = gridR_ + dr, c = gridC_ + dc;
            if (maze.inBounds(r, c) && (std::abs(dr) + std::abs(dc) <= radius + 1)) {
                discovered_[r][c] = true;
            }
        }
    }

    if (facingDr_ != 0 || facingDc_ != 0) {
        int maxDist = radius + forwardExtra;
        for (int dist = 1; dist <= maxDist; dist++) {
            int fr = gridR_ + facingDr_ * dist;
            int fc = gridC_ + facingDc_ * dist;

            if (!maze.inBounds(fr, fc)) break;

            discovered_[fr][fc] = true;

            int sr1 = fr + facingDc_, sc1 = fc + facingDr_;
            int sr2 = fr - facingDc_, sc2 = fc - facingDr_;
            if (maze.inBounds(sr1, sc1)) discovered_[sr1][sc1] = true;
            if (maze.inBounds(sr2, sc2)) discovered_[sr2][sc2] = true;

            if (maze.isWall(fr, fc)) break;
        }
    }
}

bool Player::isDiscovered(int r, int c) const {
    if (r < 0 || r >= (int)discovered_.size() || c < 0 || c >= (int)discovered_[0].size())
        return false;
    return discovered_[r][c];
}

#include "GridMover.h"
#include <cmath>

GridMover::GridMover(int startR, int startC, float cellsPerSec, int cellW, int cellH)
    : gridR_(startR), gridC_(startC),
      targetR_(startR), targetC_(startC),
      posX_(static_cast<float>(startC * cellW)),
      posY_(static_cast<float>(startR * cellH)),
      facingDr_(0), facingDc_(1),
      steps_(0),
      isMoving_(false),
      animTimer_(0.0f),
      animFrame_(0),
      cellW_(cellW), cellH_(cellH),
      cellsPerSec_(cellsPerSec),
      speedMultiplier_(1.0f),
      snapThreshold_(cfg::SNAP_THRESHOLD),
      animFrameTime_(cfg::ANIM_FRAME_TIME),
      totalAnimFrames_(cfg::TOTAL_ANIM_FRAMES) {
}

bool GridMover::setTargetCell(int nextR, int nextC) {
    targetR_ = nextR;
    targetC_ = nextC;
    facingDr_ = targetR_ - gridR_;
    facingDc_ = targetC_ - gridC_;
    isMoving_ = (targetR_ != gridR_ || targetC_ != gridC_);
    if (isMoving_) steps_++;
    return isMoving_;
}

void GridMover::updateWithInput(float dt, int inputDr, int inputDc, const Maze& maze) {
    float targetX = static_cast<float>(targetC_ * cellW_);
    float targetY = static_cast<float>(targetR_ * cellH_);

    float dx = targetX - posX_;
    float dy = targetY - posY_;
    float dist = std::sqrt(dx * dx + dy * dy);
    float speed = effectiveSpeed();

    if (dist < snapThreshold_) {
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
        float moveDist = speed * dt;
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
        if (animTimer_ >= animFrameTime_) {
            animTimer_ = 0.0f;
            animFrame_ = (animFrame_ + 1) % totalAnimFrames_;
        }
    } else {
        animFrame_ = 0;
        animTimer_ = 0.0f;
    }
}

bool GridMover::updateTowardsTarget(float dt) {
    if (!isMoving_) return true;

    float targetX = static_cast<float>(targetC_ * cellW_);
    float targetY = static_cast<float>(targetR_ * cellH_);

    float dx = targetX - posX_;
    float dy = targetY - posY_;
    float dist = std::sqrt(dx * dx + dy * dy);
    float speed = effectiveSpeed();

    float moveDist = speed * dt;
    bool reached = false;

    if (moveDist >= dist || dist < snapThreshold_) {
        posX_ = targetX;
        posY_ = targetY;
        gridR_ = targetR_;
        gridC_ = targetC_;
        isMoving_ = false;
        reached = true;
    } else {
        posX_ += (dx / dist) * moveDist;
        posY_ += (dy / dist) * moveDist;
    }

    if (isMoving_) {
        animTimer_ += dt;
        if (animTimer_ >= animFrameTime_) {
            animTimer_ = 0.0f;
            animFrame_ = (animFrame_ + 1) % totalAnimFrames_;
        }
    } else {
        animFrame_ = 0;
        animTimer_ = 0.0f;
    }

    return reached;
}

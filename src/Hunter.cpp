#include "Hunter.h"
#include "dsa_lib/Queue.h"
#include <vector>
#include <utility>
#include <cmath>

#define BFS_DIRECTIONS 4

Hunter::Hunter(int startR, int startC, int cellW, int cellH)
    : gridR_(startR), gridC_(startC), targetR_(startR), targetC_(startC),
      cellW_(cellW), cellH_(cellH) {
    posX_ = startC * cellW_;
    posY_ = startR * cellH_;
}

void Hunter::update(float dt) {
    float targetX = targetC_ * cellW_;
    float targetY = targetR_ * cellH_;

    float dx = targetX - posX_;
    float dy = targetY - posY_;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist < HUNTER_SNAP_THRESHOLD) {
        posX_ = targetX;
        posY_ = targetY;
        gridR_ = targetR_;
        gridC_ = targetC_;
        isMoving_ = false;
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
        if (animTimer_ >= HUNTER_ANIM_FRAME_TIME) {
            animTimer_ = 0.0f;
            animFrame_ = (animFrame_ + 1) % HUNTER_TOTAL_FRAMES;
        }
    } else {
        animFrame_ = 0;
        animTimer_ = 0.0f;
    }
}

// ============================================================================
// DSA CORE: QUEUE-BASED BREADTH-FIRST SEARCH (BFS) FOR SHORTEST PATH PURSUIT
// ============================================================================
void Hunter::step(const Maze& maze, int targetR, int targetC) {
    int rows = maze.rows(), cols = maze.cols();
    std::vector<std::vector<std::pair<int,int>>> parent(rows, std::vector<std::pair<int,int>>(cols, std::make_pair(-1,-1)));
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));

    dsa_lib::Queue<std::pair<int,int>> q;
    q.push(std::make_pair(gridR_, gridC_));
    visited[gridR_][gridC_] = true;

    const int dr[BFS_DIRECTIONS] = {-1, 1, 0, 0};
    const int dc[BFS_DIRECTIONS] = {0, 0, -1, 1};

    bool found = false;

    while (!q.empty() && !found) {
        std::pair<int,int> front = q.front();
        q.pop();

        int r = front.first;
        int c = front.second;

        if (r == targetR && c == targetC) {
            found = true;
            break;
        }

        for (int i = 0; i < BFS_DIRECTIONS; i++) {
            int nr = r + dr[i], nc = c + dc[i];
            if (maze.inBounds(nr, nc) && !maze.isWall(nr, nc) && !visited[nr][nc]) {
                visited[nr][nc] = true;
                parent[nr][nc] = std::make_pair(r, c);
                q.push(std::make_pair(nr, nc));
            }
        }
    }

    if (!found) return;

    std::pair<int,int> cur = std::make_pair(targetR, targetC);
    std::pair<int,int> prev = cur;
    while (parent[cur.first][cur.second] != std::make_pair(gridR_, gridC_) &&
           parent[cur.first][cur.second] != std::make_pair(-1, -1)) {
        prev = cur;
        cur = parent[cur.first][cur.second];
    }
    std::pair<int,int> nextStep = (parent[cur.first][cur.second] == std::make_pair(gridR_, gridC_)) ? cur : prev;

    targetR_ = nextStep.first;
    targetC_ = nextStep.second;
}

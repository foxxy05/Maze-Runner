#include "Pathfinder.h"
#include "dsa_lib/Queue.h"
#include "Config.h"
#include <chrono>
#include <algorithm>
#include <cstdlib>

// ============================================================================
// DSA CORE: QUEUE-BASED BREADTH-FIRST SEARCH (BFS) FOR SHORTEST PATH PURSUIT
// ============================================================================
BfsResult Pathfinder::findPath(const Maze& maze, std::pair<int,int> start, std::pair<int,int> goal) {
    auto t0 = std::chrono::high_resolution_clock::now();

    BfsResult res;
    res.found = false;
    res.nodesVisited = 0;
    res.maxQueueSize = 0;
    res.micros = 0;

    int rows = maze.rows();
    int cols = maze.cols();

    if (!maze.inBounds(start.first, start.second) || !maze.inBounds(goal.first, goal.second) ||
        maze.isWall(start.first, start.second) || maze.isWall(goal.first, goal.second)) {
        auto t1 = std::chrono::high_resolution_clock::now();
        res.micros = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        return res;
    }

    if (start == goal) {
        res.found = true;
        res.path = {start};
        res.visitOrder = {start};
        res.nodesVisited = 1;
        auto t1 = std::chrono::high_resolution_clock::now();
        res.micros = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        return res;
    }

    std::vector<std::vector<std::pair<int,int>>> parent(rows, std::vector<std::pair<int,int>>(cols, std::make_pair(-1,-1)));
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));

    dsa_lib::Queue<std::pair<int,int>> q;
    q.push(start);
    visited[start.first][start.second] = true;
    res.maxQueueSize = q.size();

    const int dr[cfg::NUM_DIRECTIONS] = {-1, 1, 0, 0};
    const int dc[cfg::NUM_DIRECTIONS] = {0, 0, -1, 1};

    while (!q.empty()) {
        std::pair<int,int> cur = q.front();
        q.pop();

        res.nodesVisited++;
        res.visitOrder.push_back(cur);

        if (cur == goal) {
            res.found = true;
            break;
        }

        for (int i = 0; i < cfg::NUM_DIRECTIONS; i++) {
            int nr = cur.first + dr[i];
            int nc = cur.second + dc[i];
            if (maze.inBounds(nr, nc) && !maze.isWall(nr, nc) && !visited[nr][nc]) {
                visited[nr][nc] = true;
                parent[nr][nc] = cur;
                q.push(std::make_pair(nr, nc));
                if (q.size() > res.maxQueueSize) {
                    res.maxQueueSize = q.size();
                }
            }
        }
    }

    if (res.found) {
        std::vector<std::pair<int,int>> path;
        std::pair<int,int> cur = goal;
        while (cur != start) {
            path.push_back(cur);
            cur = parent[cur.first][cur.second];
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
        res.path = std::move(path);
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    res.micros = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    return res;
}

std::pair<int,int> Pathfinder::findSpawnCell(const Maze& maze, std::pair<int,int> playerStart, int targetDistance) {
    int rows = maze.rows();
    int cols = maze.cols();
    std::vector<std::vector<int>> dist(rows, std::vector<int>(cols, -1));

    dsa_lib::Queue<std::pair<int,int>> q;
    q.push(playerStart);
    dist[playerStart.first][playerStart.second] = 0;

    const int dr[cfg::NUM_DIRECTIONS] = {-1, 1, 0, 0};
    const int dc[cfg::NUM_DIRECTIONS] = {0, 0, -1, 1};

    std::pair<int,int> bestCell = playerStart;
    int bestDiff = 999999;

    while (!q.empty()) {
        std::pair<int,int> cur = q.front();
        q.pop();

        int d = dist[cur.first][cur.second];
        int diff = std::abs(d - targetDistance);
        if (diff < bestDiff && cur != playerStart) {
            bestDiff = diff;
            bestCell = cur;
            if (diff == 0) {
                break;
            }
        }

        for (int i = 0; i < cfg::NUM_DIRECTIONS; i++) {
            int nr = cur.first + dr[i];
            int nc = cur.second + dc[i];
            if (maze.inBounds(nr, nc) && !maze.isWall(nr, nc) && dist[nr][nc] == -1) {
                dist[nr][nc] = d + 1;
                q.push(std::make_pair(nr, nc));
            }
        }
    }

    return bestCell;
}

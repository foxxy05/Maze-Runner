#include "Maze.h"
#include "dsa_lib/Stack.h"
#include <random>
#include <chrono>
#include <algorithm>

#define NUM_DIRECTIONS 4

Maze::Maze(int rows, int cols) : rows_(rows), cols_(cols) {
    grid_.assign(rows_, std::vector<int>(cols_, WALL_TILE));
}

bool Maze::inBounds(int r, int c) const {
    return r >= 0 && r < rows_ && c >= 0 && c < cols_;
}

bool Maze::isWall(int r, int c) const {
    if (!inBounds(r, c)) return true;
    return grid_[r][c] == WALL_TILE;
}

// ============================================================================
// DSA CORE: STACK-BASED DEPTH-FIRST SEARCH (DFS) MAZE GENERATION
// ============================================================================
void Maze::generate() {
    for (auto& row : grid_) std::fill(row.begin(), row.end(), WALL_TILE);

    auto nano = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(nano ^ std::random_device{}()));

    dsa_lib::Stack<std::pair<int, int>> s;
    grid_[startRow()][startCol()] = PATH_TILE;
    s.push(std::make_pair(startRow(), startCol()));

    const int dr[NUM_DIRECTIONS] = {-STEP_DIST, 0, STEP_DIST, 0};
    const int dc[NUM_DIRECTIONS] = {0, STEP_DIST, 0, -STEP_DIST};

    while (!s.empty()) {
        std::pair<int, int> top = s.top();
        int r = top.first;
        int c = top.second;

        std::vector<int> options;
        for (int dir = 0; dir < NUM_DIRECTIONS; dir++) {
            int nr = r + dr[dir], nc = c + dc[dir];
            if (inBounds(nr, nc) && grid_[nr][nc] == WALL_TILE) {
                options.push_back(dir);
            }
        }

        if (!options.empty()) {
            int dir = options[rng() % options.size()];
            int nr = r + dr[dir], nc = c + dc[dir];
            grid_[r + dr[dir] / STEP_DIST][c + dc[dir] / STEP_DIST] = PATH_TILE;
            grid_[nr][nc] = PATH_TILE;
            s.push(std::make_pair(nr, nc));
        } else {
            s.pop();
        }
    }

    // Carve guaranteed exit region
    grid_[exitRow()][exitCol()] = PATH_TILE;
    if (inBounds(exitRow() - 1, exitCol())) grid_[exitRow() - 1][exitCol()] = PATH_TILE;
    if (inBounds(exitRow(), exitCol() - 1)) grid_[exitRow()][exitCol() - 1] = PATH_TILE;

    // Braid maze (wall removal for multi-path branching loops)
    std::vector<std::pair<int, int>> removableWalls;
    for (int r = 1; r < rows_ - 1; r++) {
        for (int c = 1; c < cols_ - 1; c++) {
            if (grid_[r][c] == WALL_TILE) {
                bool hPass = (!isWall(r, c - 1) && !isWall(r, c + 1) && isWall(r - 1, c) && isWall(r + 1, c));
                bool vPass = (!isWall(r - 1, c) && !isWall(r + 1, c) && isWall(r, c - 1) && isWall(r, c + 1));
                if (hPass || vPass) {
                    removableWalls.push_back(std::make_pair(r, c));
                }
            }
        }
    }

    std::shuffle(removableWalls.begin(), removableWalls.end(), rng);
    size_t toRemove = static_cast<size_t>(removableWalls.size() * BRAID_PERCENT);
    for (size_t i = 0; i < toRemove && i < removableWalls.size(); i++) {
        grid_[removableWalls[i].first][removableWalls[i].second] = PATH_TILE;
    }
}

#include "Maze.h"
#include "dsa_lib/Stack.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <cassert>

Maze::Maze(int rows, int cols) : rows_(rows), cols_(cols), seed_(0) {
    assert(rows % 2 != 0 && cols % 2 != 0 && "Maze dimensions must be odd");
    grid_.assign(rows_, std::vector<int>(cols_, cfg::WALL_TILE));
}

bool Maze::inBounds(int r, int c) const {
    return r >= 0 && r < rows_ && c >= 0 && c < cols_;
}

bool Maze::isWall(int r, int c) const {
    if (!inBounds(r, c)) return true;
    return grid_[r][c] == cfg::WALL_TILE;
}

// ============================================================================
// DSA CORE: INCREMENTAL STACK-BASED DEPTH-FIRST SEARCH (DFS) MAZE GENERATION
// ============================================================================
void Maze::beginGenerate(unsigned int seed) {
    if (seed == 0) {
        auto nano = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        seed_ = static_cast<unsigned int>(nano ^ std::random_device{}());
        if (seed_ == 0) seed_ = 1;
    } else {
        seed_ = seed;
    }

    generationRng_.seed(seed_);

    for (auto& row : grid_) std::fill(row.begin(), row.end(), cfg::WALL_TILE);

    generationStack_.clear();
    braidWalls_.clear();
    braidIndex_ = 0;
    braidRemoveCount_ = 0;
    generationPushes_ = 0;
    generationPops_ = 0;
    generationMaxDepth_ = 0;
    generationStep_ = 0;
    generationPhase_ = GenerationPhase::DFS_CARVE;
    grid_[startRow()][startCol()] = cfg::PATH_TILE;
    generationStack_.push(std::make_pair(startRow(), startCol()));
    generationPushes_ = 1;
    generationMaxDepth_ = 1;
    currentRow_ = startRow();
    currentCol_ = startCol();
    lastAction_ = "PUSH: start cell";
}

bool Maze::stepGenerate() {
    if (generationPhase_ == GenerationPhase::DONE) return false;
    ++generationStep_;

    const int dr[cfg::NUM_DIRECTIONS] = {-cfg::STEP_DIST, 0, cfg::STEP_DIST, 0};
    const int dc[cfg::NUM_DIRECTIONS] = {0, cfg::STEP_DIST, 0, -cfg::STEP_DIST};

    if (generationPhase_ == GenerationPhase::DFS_CARVE || generationPhase_ == GenerationPhase::BACKTRACK) {
        if (generationStack_.empty()) {
            generationPhase_ = GenerationPhase::BRAID;
            lastAction_ = "BRAID: preparing removable walls";
            for (int r = 1; r < rows_ - 1; r++) {
                for (int c = 1; c < cols_ - 1; c++) {
                    if (grid_[r][c] == cfg::WALL_TILE) {
                        bool hPass = (!isWall(r, c - 1) && !isWall(r, c + 1) && isWall(r - 1, c) && isWall(r + 1, c));
                        bool vPass = (!isWall(r - 1, c) && !isWall(r + 1, c) && isWall(r, c - 1) && isWall(r, c + 1));
                        if (hPass || vPass) braidWalls_.push_back(std::make_pair(r, c));
                    }
                }
            }
            std::shuffle(braidWalls_.begin(), braidWalls_.end(), generationRng_);
            braidRemoveCount_ = static_cast<size_t>(braidWalls_.size() * cfg::BRAID_PERCENT);
            return true;
        }

        std::pair<int, int> top = generationStack_.top();
        int r = top.first;
        int c = top.second;
        currentRow_ = r;
        currentCol_ = c;

        std::vector<int> options;
        for (int dir = 0; dir < cfg::NUM_DIRECTIONS; dir++) {
            int nr = r + dr[dir], nc = c + dc[dir];
            if (inBounds(nr, nc) && grid_[nr][nc] == cfg::WALL_TILE) {
                options.push_back(dir);
            }
        }

        if (!options.empty()) {
            int dir = options[generationRng_() % options.size()];
            int nr = r + dr[dir], nc = c + dc[dir];
            grid_[r + dr[dir] / cfg::STEP_DIST][c + dc[dir] / cfg::STEP_DIST] = cfg::PATH_TILE;
            grid_[nr][nc] = cfg::PATH_TILE;
            generationStack_.push(std::make_pair(nr, nc));
            ++generationPushes_;
            generationMaxDepth_ = std::max(generationMaxDepth_, generationStack_.size());
            currentRow_ = nr;
            currentCol_ = nc;
            const char* names[cfg::NUM_DIRECTIONS] = {"north", "east", "south", "west"};
            lastAction_ = std::string("PUSH: carve ") + names[dir];
            generationPhase_ = GenerationPhase::DFS_CARVE;
        } else {
            generationStack_.pop();
            ++generationPops_;
            lastAction_ = "BACKTRACK: pop (dead end)";
            generationPhase_ = GenerationPhase::BACKTRACK;
        }
        return true;
    }

    if (generationPhase_ == GenerationPhase::BRAID) {
        if (braidIndex_ < braidRemoveCount_) {
            const std::pair<int, int>& wall = braidWalls_[braidIndex_++];
            grid_[wall.first][wall.second] = cfg::PATH_TILE;
            currentRow_ = wall.first;
            currentCol_ = wall.second;
            lastAction_ = "BRAID: removing walls";
            return true;
        }
        generationPhase_ = GenerationPhase::DONE;
        lastAction_ = "MAZE READY";
        return false;
    }
    return false;
}

void Maze::generate(unsigned int seed) {
    beginGenerate(seed);
    while (stepGenerate()) {}
}

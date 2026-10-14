#pragma once
#include <vector>
#include <string>
#include <random>
#include "Config.h"
#include "dsa_lib/Stack.h"

enum class GenerationPhase { DFS_CARVE, BACKTRACK, BRAID, DONE };

class Maze {
public:
    Maze(int rows, int cols);

    void generate(unsigned int seed = 0);
    void beginGenerate(unsigned int seed = 0);
    bool stepGenerate();

    bool isWall(int r, int c) const;
    bool inBounds(int r, int c) const;

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    int startRow() const { return 1; }
    int startCol() const { return 1; }
    int exitRow() const { return rows_ - 2; }
    int exitCol() const { return cols_ - 2; }

    unsigned int seed() const { return seed_; }
    int currentRow() const { return currentRow_; }
    int currentCol() const { return currentCol_; }
    GenerationPhase generationPhase() const { return generationPhase_; }
    const std::string& lastAction() const { return lastAction_; }
    std::vector<std::pair<int, int>> generationStackTop(int count) const { return generationStack_.topEntries(count); }
    int generationPushes() const { return generationPushes_; }
    int generationPops() const { return generationPops_; }
    int generationMaxDepth() const { return generationMaxDepth_; }
    int generationStep() const { return generationStep_; }

private:
    int rows_, cols_;
    unsigned int seed_ = 0;
    std::vector<std::vector<int>> grid_;
    std::mt19937 generationRng_;
    dsa_lib::Stack<std::pair<int, int>> generationStack_;
    std::vector<std::pair<int, int>> braidWalls_;
    size_t braidIndex_ = 0;
    size_t braidRemoveCount_ = 0;
    GenerationPhase generationPhase_ = GenerationPhase::DONE;
    std::string lastAction_ = "READY";
    int currentRow_ = 1;
    int currentCol_ = 1;
    int generationPushes_ = 0;
    int generationPops_ = 0;
    int generationMaxDepth_ = 0;
    int generationStep_ = 0;
};

#pragma once
#include <vector>
#include "Config.h"

class Maze {
public:
    Maze(int rows, int cols);

    void generate(unsigned int seed = 0);

    bool isWall(int r, int c) const;
    bool inBounds(int r, int c) const;

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    int startRow() const { return 1; }
    int startCol() const { return 1; }
    int exitRow() const { return rows_ - 2; }
    int exitCol() const { return cols_ - 2; }

    unsigned int seed() const { return seed_; }

private:
    int rows_, cols_;
    unsigned int seed_ = 0;
    std::vector<std::vector<int>> grid_;
};

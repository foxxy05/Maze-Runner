#pragma once
#include <vector>

#define WALL_TILE 1
#define PATH_TILE 0
#define STEP_DIST 2
#define BRAID_PERCENT 0.18

class Maze {
public:
    Maze(int rows, int cols);

    void generate();

    bool isWall(int r, int c) const;
    bool inBounds(int r, int c) const;

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    int startRow() const { return 1; }
    int startCol() const { return 1; }
    int exitRow() const { return rows_ - 2; }
    int exitCol() const { return cols_ - 2; }

private:
    int rows_, cols_;
    std::vector<std::vector<int>> grid_;
};

#pragma once
#include <vector>
#include <utility>
#include "Maze.h"

struct BfsResult {
    bool found;
    std::vector<std::pair<int,int>> path;
    std::vector<std::pair<int,int>> visitOrder;
    int nodesVisited;
    int maxQueueSize;
    long long micros;
};

class Pathfinder {
public:
    static BfsResult findPath(const Maze& maze, std::pair<int,int> start, std::pair<int,int> goal);
    static std::pair<int,int> findSpawnCell(const Maze& maze, std::pair<int,int> playerStart, int targetDistance);
};

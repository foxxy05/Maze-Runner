#pragma once
#include <string>
#include <vector>
#include "dsa_lib/BST.h"

struct LeaderboardEntry {
    std::string name;
    int score = 0;
    std::string mode;
    std::string detail;
    unsigned long long insertionId = 0;
};

struct LeaderboardCompare {
    // Ascending tree order; reverse in-order is score descending, name ascending, id ascending.
    bool operator()(const LeaderboardEntry& a, const LeaderboardEntry& b) const;
};

class Leaderboard {
public:
    Leaderboard();

    void load();
    bool save() const;
    void add(std::string name, int score, const std::string& mode, const std::string& detail);

    const dsa_lib::BST<LeaderboardEntry, LeaderboardCompare>& tree() const { return entries_; }
    unsigned long long latestInsertionId() const { return latestInsertionId_; }
    std::vector<LeaderboardEntry> top(int limit = 10) const;
    std::vector<LeaderboardEntry> traversal(int order) const; // 1=in, 2=pre, 3=post

private:
    std::string filePath() const;
    dsa_lib::BST<LeaderboardEntry, LeaderboardCompare> entries_;
    unsigned long long nextInsertionId_ = 1;
    unsigned long long latestInsertionId_ = 0;
};


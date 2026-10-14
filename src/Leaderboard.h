#pragma once
#include "dsa_lib/BST.h"
#include <string>
#include <vector>

struct LeaderboardEntry {
    std::string name;    // max 10 chars
    int score = 0;
    std::string mode;    // "SP" or "2P"
    std::string detail;  // e.g. "Escape 32.5s"
    unsigned long long id = 0;
};

// Standard comparator: order by score ascending, then name, then id.
// With this comparator, reverseInorder visits from highest score to lowest score.
struct EntryComparator {
    bool operator()(const LeaderboardEntry& a, const LeaderboardEntry& b) const {
        if (a.score != b.score) return a.score < b.score;
        if (a.name != b.name) return a.name < b.name;
        return a.id < b.id;
    }
};

class Leaderboard {
public:
    Leaderboard();
    explicit Leaderboard(const std::string& filename);

    void addEntry(const std::string& name, int score, const std::string& mode, const std::string& detail);

    std::vector<LeaderboardEntry> getTop(int count = 10) const;
    std::vector<LeaderboardEntry> getInorderList() const;
    std::vector<LeaderboardEntry> getPreorderList() const;
    std::vector<LeaderboardEntry> getPostorderList() const;

    int size() const { return tree_.size(); }
    int height() const { return tree_.height(); }
    bool empty() const { return tree_.empty(); }

    const dsa_lib::BST<LeaderboardEntry, EntryComparator>& getTree() const { return tree_; }

    void saveToFile(const std::string& filename = "leaderboard.txt") const;
    void loadFromFile(const std::string& filename = "leaderboard.txt");

private:
    dsa_lib::BST<LeaderboardEntry, EntryComparator> tree_;
    unsigned long long nextId_ = 1;
    std::string filePath_ = "leaderboard.txt";
};

#include "Leaderboard.h"
#include <fstream>
#include <sstream>
#include <iostream>

Leaderboard::Leaderboard() : tree_(), nextId_(1), filePath_("leaderboard.txt") {
    loadFromFile(filePath_);
}

Leaderboard::Leaderboard(const std::string& filename) : tree_(), nextId_(1), filePath_(filename) {
    loadFromFile(filePath_);
}

void Leaderboard::addEntry(const std::string& name, int score, const std::string& mode, const std::string& detail) {
    std::string safeName = name;
    if (safeName.length() > 10) {
        safeName = safeName.substr(0, 10);
    }
    if (safeName.empty()) {
        safeName = "RUNNER";
    }

    LeaderboardEntry entry;
    entry.name = safeName;
    entry.score = score;
    entry.mode = mode;
    entry.detail = detail;
    entry.id = nextId_++;

    tree_.insert(entry);
    saveToFile(filePath_);
}

std::vector<LeaderboardEntry> Leaderboard::getTop(int count) const {
    std::vector<LeaderboardEntry> list;
    tree_.reverseInorder([&](const LeaderboardEntry& entry) {
        if (static_cast<int>(list.size()) < count) {
            list.push_back(entry);
        }
    });
    return list;
}

std::vector<LeaderboardEntry> Leaderboard::getInorderList() const {
    std::vector<LeaderboardEntry> list;
    tree_.inorder([&](const LeaderboardEntry& entry) {
        list.push_back(entry);
    });
    return list;
}

std::vector<LeaderboardEntry> Leaderboard::getPreorderList() const {
    std::vector<LeaderboardEntry> list;
    tree_.preorder([&](const LeaderboardEntry& entry) {
        list.push_back(entry);
    });
    return list;
}

std::vector<LeaderboardEntry> Leaderboard::getPostorderList() const {
    std::vector<LeaderboardEntry> list;
    tree_.postorder([&](const LeaderboardEntry& entry) {
        list.push_back(entry);
    });
    return list;
}

void Leaderboard::saveToFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "[WARNING] Could not open " << filename << " to save leaderboard." << std::endl;
        return;
    }

    // Save in PRE-ORDER traversal so reloading reproduces the exact same BST tree shape!
    tree_.preorder([&](const LeaderboardEntry& e) {
        out << e.name << "\t"
            << e.score << "\t"
            << e.mode << "\t"
            << e.detail << "\t"
            << e.id << "\n";
    });
}

void Leaderboard::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        // If file doesn't exist yet, seed with a few fun default high scores!
        addEntry("CIPHER", 1550, "SP", "Escape 22.0s");
        addEntry("NEO", 1480, "SP", "Escape 25.4s");
        addEntry("TRINITY", 1390, "SP", "Escape 28.2s");
        addEntry("MORPHEUS", 1310, "SP", "Escape 31.0s");
        addEntry("GHOST", 1120, "2P", "P1 Win 34.5s");
        return;
    }

    tree_.clear();
    nextId_ = 1;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string name, scoreStr, mode, detail, idStr;

        if (std::getline(iss, name, '\t') &&
            std::getline(iss, scoreStr, '\t') &&
            std::getline(iss, mode, '\t') &&
            std::getline(iss, detail, '\t') &&
            std::getline(iss, idStr, '\t')) {

            LeaderboardEntry entry;
            entry.name = name;
            try {
                entry.score = std::stoi(scoreStr);
                entry.id = std::stoull(idStr);
            } catch (...) {
                continue;
            }
            entry.mode = mode;
            entry.detail = detail;

            if (entry.id >= nextId_) {
                nextId_ = entry.id + 1;
            }

            tree_.insert(entry);
        }
    }
}

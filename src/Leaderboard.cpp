#include "Leaderboard.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif

inline bool LeaderboardCompare::operator()(const LeaderboardEntry& a, const LeaderboardEntry& b) const {
    if (a.score != b.score) return a.score < b.score;
    if (a.name != b.name) return a.name > b.name;
    return a.insertionId > b.insertionId;
}

inline Leaderboard::Leaderboard() {
    load();
}

inline std::string Leaderboard::filePath() const {
#ifdef _WIN32
    char path[MAX_PATH] = {};
    DWORD length = GetModuleFileNameA(nullptr, path, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        std::string executable(path);
        size_t slash = executable.find_last_of("\\/");
        return (slash == std::string::npos) ? "leaderboard.txt" : executable.substr(0, slash + 1) + "leaderboard.txt";
    }
#endif
    return "leaderboard.txt";
}

inline void Leaderboard::load() {
    entries_.clear();
    nextInsertionId_ = 1;
    std::ifstream input(filePath());
    std::string line;
    while (std::getline(input, line)) {
        std::istringstream row(line);
        LeaderboardEntry entry;
        std::string scoreText, idText;
        if (!std::getline(row, entry.name, '\t') || !std::getline(row, scoreText, '\t') ||
            !std::getline(row, entry.mode, '\t') || !std::getline(row, entry.detail, '\t') ||
            !std::getline(row, idText)) continue;
        try {
            entry.score = std::stoi(scoreText);
            entry.insertionId = std::stoull(idText);
        } catch (...) { continue; }
        entry.name = entry.name.substr(0, 10);
        entries_.insert(entry); // File is pre-order, preserving the exact BST shape.
        nextInsertionId_ = std::max(nextInsertionId_, entry.insertionId + 1);
    }
}

inline bool Leaderboard::save() const {
    std::ofstream output(filePath(), std::ios::trunc);
    if (!output) return false;
    entries_.preorder([&output](const LeaderboardEntry& entry) {
        output << entry.name << '\t' << entry.score << '\t' << entry.mode << '\t'
               << entry.detail << '\t' << entry.insertionId << '\n';
    });
    return static_cast<bool>(output);
}

inline void Leaderboard::add(std::string name, int score, const std::string& mode, const std::string& detail) {
    name.erase(std::remove_if(name.begin(), name.end(), [](char c) { return c == '\t' || c == '\n' || c == '\r'; }), name.end());
    if (name.empty()) name = "PLAYER";
    entries_.insert({name.substr(0, 10), score, mode, detail, nextInsertionId_++});
    save();
}

inline std::vector<LeaderboardEntry> Leaderboard::top(int limit) const {
    std::vector<LeaderboardEntry> results;
    entries_.reverseInorder([&results, limit](const LeaderboardEntry& entry) {
        if (static_cast<int>(results.size()) < limit) results.push_back(entry);
    });
    return results;
}

inline std::vector<LeaderboardEntry> Leaderboard::traversal(int order) const {
    std::vector<LeaderboardEntry> results;
    if (order == 2) entries_.preorder([&results](const LeaderboardEntry& entry) { results.push_back(entry); });
    else if (order == 3) entries_.postorder([&results](const LeaderboardEntry& entry) { results.push_back(entry); });
    else entries_.inorder([&results](const LeaderboardEntry& entry) { results.push_back(entry); });
    return results;
}

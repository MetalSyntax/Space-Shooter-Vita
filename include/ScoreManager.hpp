#pragma once

#include <cmath>
#include <ctime>
#include <cstdio>
#include <algorithm>
#include "SaveSystem.hpp"

/** Score, combo multiplier and local high-score table (port of ScoreManager.ts). */
namespace Game {

constexpr float COMBO_WINDOW = 2500.0f; // ms to chain the next kill

class ScoreManager {
public:
    int score = 0;
    int comboKills = 0;
    float comboTimer = 0.0f;

    ScoreManager(int difficultyMulti = 1, int initialScore = 0) : score(initialScore), diffMulti(difficultyMulti) {}

    float multiplier() const {
        if (comboKills >= 15) return 3.0f;
        if (comboKills >= 8) return 2.0f;
        if (comboKills >= 3) return 1.5f;
        return 1.0f;
    }

    /** Registers a kill; returns the points awarded and sets tierUp when the multiplier went up. */
    int addKill(int basePoints, bool crit, bool& tierUp) {
        float before = multiplier();
        comboKills++;
        comboTimer = COMBO_WINDOW;
        int points = static_cast<int>(std::lround(basePoints * diffMulti * multiplier() * (crit ? 2 : 1)));
        score += points;
        tierUp = multiplier() > before;
        return points;
    }

    int addPoints(int basePoints) {
        int points = basePoints * diffMulti;
        score += points;
        return points;
    }

    void resetCombo() {
        comboKills = 0;
        comboTimer = 0.0f;
    }

    void update(float deltaMs) {
        if (comboTimer <= 0.0f) return;
        comboTimer -= deltaMs;
        if (comboTimer <= 0.0f) resetCombo();
    }

    static int best() {
        auto& d = Storage::SaveSystem::data();
        return d.scoreCount > 0 ? d.scores[0].score : 0;
    }

    /** Saves the score and returns true if it is the new #1. */
    static bool submit(int score, int difficulty) {
        auto& d = Storage::SaveSystem::data();
        bool isRecord = score > 0 && (d.scoreCount == 0 || score > d.scores[0].score);
        if (score <= 0) return false;
        Storage::HighScoreEntry e;
        e.score = score;
        e.difficulty = difficulty;
        std::time_t now = std::time(nullptr);
        std::tm* tm = std::localtime(&now);
        if (tm) std::snprintf(e.date, sizeof(e.date), "%04d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
        int n = d.scoreCount;
        if (n < Storage::MAX_SCORES) d.scores[n++] = e;
        else if (score > d.scores[n - 1].score) d.scores[n - 1] = e;
        std::stable_sort(d.scores, d.scores + n, [](const Storage::HighScoreEntry& a, const Storage::HighScoreEntry& b) { return a.score > b.score; });
        d.scoreCount = n;
        Storage::SaveSystem::flush();
        return isRecord;
    }

private:
    int diffMulti = 1;
};

} // namespace Game

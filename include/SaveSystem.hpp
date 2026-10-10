#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <sys/stat.h>

#ifdef __vita__
#include <psp2/io/stat.h>
#endif

namespace Storage {

constexpr int MAX_SCORES = 10;

struct HighScoreEntry {
    int32_t score = 0;
    int32_t difficulty = 1; // Game::Difficulty
    char date[12] = {0};    // YYYY-MM-DD
};

struct CampaignSave {
    int32_t valid = 0;
    int32_t difficulty = 1;
    int32_t ship = 0;
    int32_t level = 1;
    int32_t score = 0;
    int32_t lives = 5;
    int32_t perks[4] = {0, 0, 0, 0};
};

/** Everything that persists between runs (Phaser used localStorage keys for each of these). */
struct SaveData {
    uint32_t magic = 0x53535632; // "SSV2"
    int32_t scoreCount = 0;
    HighScoreEntry scores[MAX_SCORES];
    CampaignSave campaign;
    int32_t lang = -1; // -1 = not chosen yet (use system language)
    int32_t musicMuted = 0;
    int32_t sfxMuted = 0;
    int32_t selectedShip = 0;
    int32_t highestSector = 1;
    uint32_t trophies = 0; // bitmask of Game::TrophyId
};

/** Layout of the first Vita release (save.dat with only these 3 ints). */
struct LegacySaveData {
    int32_t highScore;
    int32_t highestSector;
    int32_t selectedShip;
};

class SaveSystem {
public:
    static std::string getSavePath() {
#ifdef __vita__
        // Directorio estándar de persistencia en Vita
        sceIoMkdir("ux0:data/shooterspace", 0777);
        return "ux0:data/shooterspace/save.dat";
#else
        mkdir("save_data", 0777);
        return "save_data/save.dat";
#endif
    }

    /** Single in-memory copy; call flush() after changing it. */
    static SaveData& data() {
        static SaveData s_data = loadFromDisk();
        return s_data;
    }

    static bool flush() {
        std::ofstream out(getSavePath(), std::ios::binary | std::ios::trunc);
        if (!out.is_open()) return false;
        out.write(reinterpret_cast<const char*>(&data()), sizeof(SaveData));
        return out.good();
    }

private:
    static SaveData loadFromDisk() {
        SaveData d;
        std::ifstream in(getSavePath(), std::ios::binary | std::ios::ate);
        if (!in.is_open()) return d;
        std::streamsize size = in.tellg();
        in.seekg(0);
        if (size == (std::streamsize)sizeof(SaveData)) {
            SaveData tmp;
            in.read(reinterpret_cast<char*>(&tmp), sizeof(SaveData));
            if (in.good() && tmp.magic == d.magic && tmp.scoreCount >= 0 && tmp.scoreCount <= MAX_SCORES) d = tmp;
        } else if (size == (std::streamsize)sizeof(LegacySaveData)) {
            LegacySaveData old{};
            in.read(reinterpret_cast<char*>(&old), sizeof(old));
            if (old.highScore > 0) {
                d.scoreCount = 1;
                d.scores[0].score = old.highScore;
                std::strcpy(d.scores[0].date, "----------");
            }
            if (old.highestSector >= 1 && old.highestSector <= 10) d.highestSector = old.highestSector;
        }
        return d;
    }
};

} // namespace Storage

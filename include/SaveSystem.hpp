#pragma once

#include <string>
#include <fstream>
#include <sys/stat.h>

#ifdef __vita__
#include <psp2/io/stat.h>
#endif

namespace Storage {

struct SaveData {
    int highScore = 0;
    int highestSector = 1;
    int selectedShip = 0;
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

    static bool save(const SaveData& data) {
        std::ofstream out(getSavePath(), std::ios::binary);
        if (!out.is_open()) return false;
        out.write(reinterpret_cast<const char*>(&data), sizeof(SaveData));
        return true;
    }

    static SaveData load() {
        SaveData data;
        std::ifstream in(getSavePath(), std::ios::binary);
        if (in.is_open()) {
            in.read(reinterpret_cast<char*>(&data), sizeof(SaveData));
        }
        return data;
    }
};

} // namespace Storage

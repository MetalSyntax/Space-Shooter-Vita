#pragma once

#include <string>
#include <vector>
#include "SaveSystem.hpp"
#include "Renderer.hpp"

namespace Game {

enum class TrophyId {
    FIRST_BLOOD,      // Destruir el primer enemigo
    BOSS_SLAYER,      // Derrotar al primer Jefe
    SCORE_10K,        // Alcanzar 10,000 puntos
    HYPER_BOMB,       // Detonar una hiperbomba
    UNTOUCHABLE,      // Sobrevivir con escudo intacto
    SECTOR_MASTER     // Completar el sector 5
};

struct Trophy {
    TrophyId id;
    std::string title;
    std::string desc;
    bool unlocked = false;
};

class TrophyManager {
public:
    static TrophyManager& instance();

    void init();
    void unlock(TrophyId id);
    bool isUnlocked(TrophyId id) const;

    void update(float dt);
    void renderNotification(Graphics::Renderer& renderer);

    const std::vector<Trophy>& getTrophies() const { return trophies; }

private:
    TrophyManager();

    std::vector<Trophy> trophies;
    std::string popupText = "";
    float popupTimer = 0.0f;
};

} // namespace Game

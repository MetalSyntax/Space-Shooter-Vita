#include "TrophyManager.hpp"
#include <iostream>

namespace Game {

TrophyManager& TrophyManager::instance() {
    static TrophyManager s_instance;
    return s_instance;
}

TrophyManager::TrophyManager() {
    trophies = {
        {TrophyId::FIRST_BLOOD,   "PRIMERA SANGRE",   "Destruye tu primera nave enemiga", false},
        {TrophyId::BOSS_SLAYER,   "CAZA JEFES",       "Derrota al Jefe del Sector 1", false},
        {TrophyId::SCORE_10K,     "PILOTO AS",        "Alcanza 10,000 puntos en una partida", false},
        {TrophyId::HYPER_BOMB,    "SUPERNOVA",        "Detona una Hiperbomba", false},
        {TrophyId::UNTOUCHABLE,   "INTOCABLE",        "Absorbe un proyectil con el Escudo", false},
        {TrophyId::SECTOR_MASTER, "VETERANO ESPACIAL","Alcanza el Sector 5", false}
    };
}

void TrophyManager::init() {
    auto data = Storage::SaveSystem::load();
    if (data.highScore >= 10000) trophies[2].unlocked = true;
    if (data.highestSector >= 5) trophies[5].unlocked = true;
}

void TrophyManager::unlock(TrophyId id) {
    for (auto& t : trophies) {
        if (t.id == id && !t.unlocked) {
            t.unlocked = true;
            popupText = "TROFEO: " + t.title;
            popupTimer = 3.5f;

            auto data = Storage::SaveSystem::load();
            Storage::SaveSystem::save(data);
            break;
        }
    }
}

bool TrophyManager::isUnlocked(TrophyId id) const {
    for (const auto& t : trophies) {
        if (t.id == id) return t.unlocked;
    }
    return false;
}

void TrophyManager::update(float dt) {
    if (popupTimer > 0.0f) {
        popupTimer -= dt;
    }
}

void TrophyManager::renderNotification(Graphics::Renderer& renderer) {
    if (popupTimer <= 0.0f) return;

    int boxW = 320;
    int boxH = 40;
    int boxX = Graphics::SCREEN_WIDTH - boxW - 20;
    int boxY = 60;

    // Fondo azul translúcido
    renderer.drawRect(boxX, boxY, boxW, boxH, 10, 40, 90, 230);

    // Ícono de trofeo dorado
    renderer.drawRect(boxX + 10, boxY + 10, 20, 20, 255, 215, 0, 255);

    // Texto del trofeo
    UI::BitmapFont::drawText(renderer, popupText, boxX + 38, boxY + 14, 2, 255, 255, 255);
}

} // namespace Game

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
    // Cargar trofeos desbloqueados si están marcados
    if (data.highScore >= 10000) trophies[2].unlocked = true;
    if (data.highestSector >= 5) trophies[5].unlocked = true;
}

void TrophyManager::unlock(TrophyId id) {
    for (auto& t : trophies) {
        if (t.id == id && !t.unlocked) {
            t.unlocked = true;
            popupText = "TROFEO DESBLOQUEADO: " + t.title;
            popupTimer = 3.5f; // Mostrar notificación durante 3.5 segundos

            // Persistir
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

    SDL_Renderer* sdl = renderer.getSDLRenderer();
    if (!sdl) return;

    // Caja de notificación en la esquina superior derecha al estilo PS Vita
    int boxW = 340;
    int boxH = 44;
    int boxX = Graphics::SCREEN_WIDTH - boxW - 20;
    int boxY = 60;

    SDL_SetRenderDrawBlendMode(sdl, SDL_BLENDMODE_BLEND);

    // Fondo azul translúcido PlayStation
    SDL_Rect bg = {boxX, boxY, boxW, boxH};
    SDL_SetRenderDrawColor(sdl, 10, 40, 90, 230);
    SDL_RenderFillRect(sdl, &bg);

    // Borde brillante
    SDL_SetRenderDrawColor(sdl, 0, 150, 255, 255);
    SDL_RenderDrawRect(sdl, &bg);

    // Ícono de trofeo dorado
    SDL_Rect trophyIcon = {boxX + 12, boxY + 12, 20, 20};
    SDL_SetRenderDrawColor(sdl, 255, 215, 0, 255);
    SDL_RenderFillRect(sdl, &trophyIcon);

    // Texto del trofeo
    UI::BitmapFont::drawText(renderer, popupText, boxX + 42, boxY + 16, 2, 255, 255, 255);
}

} // namespace Game

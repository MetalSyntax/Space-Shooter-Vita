#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include "Renderer.hpp"
#include "InputManager.hpp"
#include "generators/PixelArtGenerator.hpp"
#include "generators/OriginalSprites.hpp"
#include "generators/BossGenerator.hpp"
#include "entities/Player.hpp"
#include "entities/Bullet.hpp"
#include "entities/Enemy.hpp"
#include "entities/CollisionSystem.hpp"
#include "LevelManager.hpp"
#include "WaveManager.hpp"
#include "ParticlePool.hpp"
#include "audio/Synth.hpp"
#include "HUD.hpp"
#include "SaveSystem.hpp"
#include "TrophyManager.hpp"

#ifdef __vita__
#include <psp2/kernel/processmgr.h>
#endif

int main(int argc, char* argv[]) {
    auto& renderer = Graphics::Renderer::instance();
    if (!renderer.init("Shooter Space - 960x544 [PS Vita / Desktop]")) {
        return 1;
    }

    // Inicializar controles físicos de PS Vita y gamepad
    auto& inputMgr = Input::InputManager::instance();
    inputMgr.init();

    // Inicializar sintetizador de audio
    auto& synth = Audio::Synth::instance();
    synth.init();

    // Inicializar sistema de trofeos
    auto& trophyMgr = Game::TrophyManager::instance();
    trophyMgr.init();

    // Cargar datos persistentes
    auto saveData = Storage::SaveSystem::load();

    // 1. Gestor de Niveles y Campaña
    Game::CampaignState campaign;
    campaign.level = 1;
    campaign.score = 0;
    campaign.lives = 5;

    Game::WaveManager waveManager;
    waveManager.init(campaign.level);
    const auto& currentLevelDef = waveManager.getCurrentLevel();

    // 2. Hornear fondo espacial procedural específico del sector
    PixelArt::RawTexture rawBg = PixelArt::generateBackground(
        42 + currentLevelDef.id,
        currentLevelDef.spaceColor,
        currentLevelDef.nebula1,
        currentLevelDef.nebula2,
        256
    );
    SDL_Texture* texBg = renderer.createTextureFromRaw(rawBg);

    // 3. Hornear sprites de jugador
    PixelArt::Grid shipGrid = PixelArt::generateShip(1337, 12, 12);
    SDL_Texture* texShip = renderer.createTextureFromRaw(PixelArt::bakeTexture(shipGrid, PixelArt::PALETTES.at("player"), 3));

    // 4. Proyectiles y Powerups
    PixelArt::Grid laserGrid = PixelArt::makeGrid(2, 6, PixelArt::CORE);
    laserGrid[0][0] = laserGrid[0][1] = PixelArt::LIGHT;
    SDL_Texture* texLaser = renderer.createTextureFromRaw(PixelArt::bakeTexture(laserGrid, PixelArt::PALETTES.at("player"), 3));

    PixelArt::Grid missileGrid = PixelArt::generateMissile();
    SDL_Texture* texMissile = renderer.createTextureFromRaw(PixelArt::bakeTexture(missileGrid, PixelArt::PALETTES.at("player"), 3));

    PixelArt::Grid orbGrid = PixelArt::generateOrb(8);
    SDL_Texture* texEnemyOrb = renderer.createTextureFromRaw(PixelArt::bakeTexture(orbGrid, PixelArt::PALETTES.at("toxic"), 3));

    PixelArt::Grid shieldGrid = PixelArt::generateOrb(16, true);
    SDL_Texture* texShield = renderer.createTextureFromRaw(PixelArt::bakeTexture(shieldGrid, PixelArt::PALETTES.at("rift"), 2));

    PixelArt::Grid droneGrid = PixelArt::generateShip(99, 6, 6);
    SDL_Texture* texDrone = renderer.createTextureFromRaw(PixelArt::bakeTexture(droneGrid, PixelArt::PALETTES.at("gold"), 2));

    // 5. Hornear Sprites de Enemigos
    PixelArt::Palette pWeaver, pGunship;
    PixelArt::Grid weaverGrid = OriginalSprites::decodeSprite(OriginalSprites::enemyWeaver, pWeaver);
    SDL_Texture* texEnemyWeaver = renderer.createTextureFromRaw(PixelArt::bakeTexture(weaverGrid, pWeaver, 2));

    PixelArt::Grid enemyShipGrid = OriginalSprites::decodeSprite(OriginalSprites::enemyShip, pGunship);
    SDL_Texture* texEnemyGunship = renderer.createTextureFromRaw(PixelArt::bakeTexture(enemyShipGrid, pGunship, 2));

    // 6. Hornear Boss del Sector
    PixelArt::Grid bossGrid = BossGenerator::generateBossHull(currentLevelDef.boss);
    SDL_Texture* texBoss = renderer.createTextureFromRaw(PixelArt::bakeTexture(bossGrid, PixelArt::PALETTES.at("volcanic"), 2));

    // 7. Instanciar Jugador y Pools
    Game::Player player;
    player.init(Game::ShipId::VANGUARD);
    player.activatePower(Game::PowerKind::SPREAD, 9999999.0f);

    Game::BulletPool<512> bulletPool;
    Game::EnemyPool<128> enemyPool;
    Graphics::ParticlePool<1024> particlePool;

    bool running = true;
    SDL_Event event;
    float bgScrollY = 0.0f;
    uint32_t lastTicks = SDL_GetTicks();

    while (running) {
        uint32_t currentTicks = SDL_GetTicks();
        float dt = (currentTicks - lastTicks) / 1000.0f;
        if (dt > 0.05f) dt = 0.05f;
        lastTicks = currentTicks;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Leer controles físicos de PS Vita (SceCtrl) o Teclado/Gamepad
        Input::InputState input = inputMgr.poll();
        if (input.pause) {
            running = false;
        }

        if (input.bomb) {
            player.useBomb();
            synth.playBomb();
            particlePool.explode(Graphics::SCREEN_WIDTH / 2.0f, Graphics::SCREEN_HEIGHT / 2.0f, 4.0f);
            trophyMgr.unlock(Game::TrophyId::HYPER_BOMB);
        }

        // 1. Gestor de Oleadas y Boss
        waveManager.update(dt, player.getX(), player.getY(), enemyPool, bulletPool);

        // 2. Estela de propulsor del jugador
        particlePool.spawnThruster(player.getX(), player.getY() + 18.0f, player.getAimAngle(), 250.0f);

        // 3. Actualizar Jugador, Balas, Enemigos, Partículas y Trofeos
        player.update(dt, input, bulletPool);
        bulletPool.update(dt);
        enemyPool.update(dt, player.getX(), player.getY(), bulletPool);
        particlePool.update(dt);
        trophyMgr.update(dt);

        // 4. Detección de colisiones
        int prevScore = campaign.score;
        campaign.score += Game::CollisionSystem::checkBulletVsEnemies(bulletPool, enemyPool, waveManager, particlePool);
        if (campaign.score > prevScore) {
            synth.playExplosion(false);
            trophyMgr.unlock(Game::TrophyId::FIRST_BLOOD);
            if (campaign.score >= 10000) {
                trophyMgr.unlock(Game::TrophyId::SCORE_10K);
            }
        }

        if (waveManager.isLevelClear()) {
            trophyMgr.unlock(Game::TrophyId::BOSS_SLAYER);
        }

        Game::CollisionSystem::checkEnemiesVsPlayer(bulletPool, enemyPool, player, particlePool);

        // Guardar record si se supera
        if (campaign.score > saveData.highScore) {
            saveData.highScore = campaign.score;
            Storage::SaveSystem::save(saveData);
        }

        bgScrollY += 60.0f * dt;

        // 5. Renderizado
        renderer.clear();

        // Fondo del sector
        renderer.drawBackgroundTiled(texBg, 0, static_cast<int>(bgScrollY));

        // Partículas (estelas y chispas)
        particlePool.render(renderer);

        // Esbirros
        enemyPool.render(renderer, texEnemyWeaver, texEnemyGunship);

        // Boss del Sector si está activo
        float bossHpPct = -1.0f;
        if (waveManager.isBossPhase() && !waveManager.isLevelClear()) {
            renderer.drawTexture(texBoss, static_cast<int>(waveManager.getBossX()), static_cast<int>(waveManager.getBossY()), 0.0, 1.2f, true);

            bossHpPct = std::max(0.0f, static_cast<float>(waveManager.getBossHp()) / waveManager.getBossMaxHp());

            // Barra de vida del Boss
            SDL_Renderer* sdl = renderer.getSDLRenderer();
            SDL_Rect barBg = {Graphics::SCREEN_WIDTH / 2 - 150, 20, 300, 12};
            SDL_SetRenderDrawColor(sdl, 50, 50, 60, 200);
            SDL_RenderFillRect(sdl, &barBg);

            SDL_Rect barFg = {Graphics::SCREEN_WIDTH / 2 - 148, 22, static_cast<int>(296 * bossHpPct), 8};
            SDL_SetRenderDrawColor(sdl, 255, 60, 60, 255);
            SDL_RenderFillRect(sdl, &barFg);
        }

        // Balas
        bulletPool.render(renderer, texLaser, texMissile, texEnemyOrb);

        // Jugador
        player.render(renderer, texShip, texShield, texDrone);

        // HUD e interfaz pixel-art
        UI::HUD::render(renderer, campaign.score, player.getLives(), player.getBombs(), campaign.level, bossHpPct);

        // Notificación de Trofeo emergente estilo PS Vita
        trophyMgr.renderNotification(renderer);

        renderer.present();
    }

    // Limpieza
    synth.shutdown();

    SDL_DestroyTexture(texBg);
    SDL_DestroyTexture(texShip);
    SDL_DestroyTexture(texLaser);
    SDL_DestroyTexture(texMissile);
    SDL_DestroyTexture(texEnemyOrb);
    SDL_DestroyTexture(texShield);
    SDL_DestroyTexture(texDrone);
    SDL_DestroyTexture(texEnemyWeaver);
    SDL_DestroyTexture(texEnemyGunship);
    SDL_DestroyTexture(texBoss);

    renderer.shutdown();

#ifdef __vita__
    sceKernelExitProcess(0);
#endif
    return 0;
}

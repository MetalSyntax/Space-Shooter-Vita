#include <cstdlib>
#include <cstring>
#include <ctime>
#include <SDL2/SDL.h>
#include "Renderer.hpp"
#include "InputManager.hpp"
#include "Assets.hpp"
#include "GameScene.hpp"
#include "Menus.hpp"
#include "Text.hpp"
#include "I18n.hpp"
#include "SaveSystem.hpp"
#include "TrophyManager.hpp"
#include "audio/Synth.hpp"

#ifdef __vita__
#include <psp2/kernel/processmgr.h>
#include <psp2/apputil.h>
#include <psp2/system_param.h>
#endif

enum class Scene { MENU, HANGAR, GUIDE, GAME, PAUSE, LEVEL_CLEAR, RESULTS };

/** First run: follow the system language (Phaser used navigator.language). */
static I18n::Lang systemLanguage() {
#ifdef __vita__
    SceAppUtilInitParam init;
    SceAppUtilBootParam boot;
    std::memset(&init, 0, sizeof(init));
    std::memset(&boot, 0, sizeof(boot));
    sceAppUtilInit(&init, &boot);
    int lang = 0;
    if (sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &lang) == 0 && lang != SCE_SYSTEM_PARAM_LANG_SPANISH) return I18n::Lang::EN;
    return I18n::Lang::ES;
#else
    const char* env = std::getenv("LANG");
    return (env && std::strncmp(env, "es", 2) == 0) ? I18n::Lang::ES : I18n::Lang::EN;
#endif
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    auto& renderer = Graphics::Renderer::instance();
    if (!renderer.init("Shooter Space - 960x544 [PS Vita / Desktop]")) {
        return 1;
    }

    auto& inputMgr = Input::InputManager::instance();
    inputMgr.init();

    auto& save = Storage::SaveSystem::data();
    I18n::g_lang = save.lang < 0 ? systemLanguage() : static_cast<I18n::Lang>(save.lang == 1);

    auto& synth = Audio::Synth::instance();
    synth.init();
    synth.setMusicMuted(save.musicMuted);
    synth.setSfxMuted(save.sfxMuted);

    auto& trophyMgr = Game::TrophyManager::instance();
    trophyMgr.init();

    UI::initFont(renderer);
    Game::assets().load(renderer);

    Game::GameScene game;
    Menus::MainMenu menu;
    Menus::Hangar hangar;
    Menus::Guide guide;
    Menus::PauseMenu pause;
    Menus::LevelClear levelClear;
    Menus::Results results;

    Scene scene = Scene::MENU;
    Scene guideReturn = Scene::MENU;
    menu.enter();

    auto startGame = [&](const Game::CampaignState& c) {
        synth.stopMusic();
        game.start(c);
        scene = Scene::GAME;
    };

    bool running = true;
    SDL_Event event;
    uint32_t lastTicks = SDL_GetTicks();

    while (running) {
        uint32_t currentTicks = SDL_GetTicks();
        float dt = (currentTicks - lastTicks) / 1000.0f;
        if (dt > 0.05f) dt = 0.05f;
        lastTicks = currentTicks;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        Input::InputState input = inputMgr.poll(dt);

        switch (scene) {
            case Scene::MENU: {
                auto action = menu.update(dt, input);
                if (action == Menus::MainMenu::Action::CONTINUE) {
                    startGame(menu.saved);
                } else if (action == Menus::MainMenu::Action::NEW_GAME) {
                    Game::clearCampaign();
                    auto ship = static_cast<Game::ShipId>(std::clamp(save.selectedShip, 0, Game::SHIP_COUNT - 1));
                    startGame(Game::newCampaign(menu.difficulty, ship));
                } else if (action == Menus::MainMenu::Action::HANGAR) {
                    hangar.enter();
                    scene = Scene::HANGAR;
                } else if (action == Menus::MainMenu::Action::GUIDE) {
                    guide.enter();
                    guideReturn = Scene::MENU;
                    scene = Scene::GUIDE;
                }
                break;
            }
            case Scene::HANGAR:
                if (hangar.update(dt, input)) {
                    menu.refresh();
                    scene = Scene::MENU;
                }
                break;
            case Scene::GUIDE:
                if (guide.update(dt, input)) {
                    if (guideReturn == Scene::MENU) menu.refresh();
                    scene = guideReturn;
                }
                break;
            case Scene::GAME: {
                Game::Outcome outcome = game.update(dt, input);
                if (outcome == Game::Outcome::PAUSE) {
                    pause.enter();
                    scene = Scene::PAUSE;
                } else if (outcome == Game::Outcome::LEVEL_CLEAR) {
                    levelClear.enter(game.resultCampaign, game.stats);
                    scene = Scene::LEVEL_CLEAR;
                } else if (outcome == Game::Outcome::GAME_OVER) {
                    results.enter(false, game.finalScore, game.difficulty(), game.newRecord, &game.campaignAtStart());
                    scene = Scene::RESULTS;
                } else if (outcome == Game::Outcome::VICTORY) {
                    results.enter(true, game.finalScore, game.difficulty(), game.newRecord, nullptr);
                    scene = Scene::RESULTS;
                }
                break;
            }
            case Scene::PAUSE: {
                auto action = pause.update(dt, input);
                if (action == Menus::PauseMenu::Action::RESUME) {
                    scene = Scene::GAME;
                } else if (action == Menus::PauseMenu::Action::RESTART) {
                    Game::CampaignState c = game.campaignAtStart();
                    startGame(c);
                } else if (action == Menus::PauseMenu::Action::GUIDE) {
                    guide.enter();
                    guideReturn = Scene::PAUSE;
                    scene = Scene::GUIDE;
                } else if (action == Menus::PauseMenu::Action::MENU) {
                    menu.enter();
                    scene = Scene::MENU;
                }
                break;
            }
            case Scene::LEVEL_CLEAR:
                if (levelClear.update(dt, input)) startGame(levelClear.next);
                break;
            case Scene::RESULTS: {
                auto action = results.update(dt, input);
                if (action == Menus::Results::Action::RETRY) {
                    startGame(results.retryCampaign());
                } else if (action == Menus::Results::Action::MENU) {
                    menu.enter();
                    scene = Scene::MENU;
                }
                break;
            }
        }

        switch (scene) {
            case Scene::MENU: menu.render(renderer); break;
            case Scene::HANGAR: hangar.render(renderer); break;
            case Scene::GUIDE: guide.render(renderer); break;
            case Scene::GAME: game.render(renderer); break;
            case Scene::PAUSE: game.render(renderer); pause.render(renderer); break;
            case Scene::LEVEL_CLEAR: levelClear.render(renderer); break;
            case Scene::RESULTS: results.render(renderer); break;
        }

        trophyMgr.update(dt);
        trophyMgr.renderNotification(renderer);
        renderer.present();
    }

    synth.shutdown();
    Game::assets().unload(renderer);
    UI::freeFont(renderer);
    renderer.shutdown();

#ifdef __vita__
    sceKernelExitProcess(0);
#endif
    return 0;
}

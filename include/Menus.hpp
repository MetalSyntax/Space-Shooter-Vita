#pragma once

#include <array>
#include <vector>
#include "Renderer.hpp"
#include "InputManager.hpp"
#include "LevelManager.hpp"
#include "GameScene.hpp"

/** Menu screens (port of MainMenu, Hangar, Guide, Pause, LevelClear, GameOver and Victory scenes). */
namespace Menus {

using Game::CampaignState;
using Game::Difficulty;

class MainMenu {
public:
    enum class Action { NONE, CONTINUE, NEW_GAME, HANGAR, GUIDE };

    void enter();   // fresh entry: (re)starts menu music
    void refresh(); // back from Hangar/Guide
    Action update(float dt, const Input::InputState& in);
    void render(Graphics::Renderer& r);

    Difficulty difficulty = Difficulty::MEDIUM;
    bool hasSave = false;
    CampaignState saved;

private:
    enum class Row { TOP, DIFF, CONTINUE, START, NAV };
    std::vector<Row> rows() const;
    int itemsIn(Row row) const;
    void activate(Row row, int col, Action& out);

    float t = 0;
    int focusRow = 0;
    int focusCol = 0;
    std::array<float, 60 * 3> stars{}; // x, y, phase
};

class Hangar {
public:
    void enter();
    bool update(float dt, const Input::InputState& in); // true = close
    void render(Graphics::Renderer& r);

private:
    int current = 0;
    float t = 0;
};

class Guide {
public:
    void enter();
    bool update(float dt, const Input::InputState& in);
    void render(Graphics::Renderer& r);

private:
    struct Row {
        Graphics::TextureType* tex;
        std::string title;
        uint32_t color;
        std::string desc;
    };
    std::vector<Row> buildRows() const;
    float contentHeight() const;

    int tab = 0;
    float scroll = 0;
};

class PauseMenu {
public:
    enum class Action { NONE, RESUME, RESTART, GUIDE, MENU };
    void enter();
    Action update(float dt, const Input::InputState& in);
    void render(Graphics::Renderer& r);

private:
    int focus = 0;
};

class LevelClear {
public:
    void enter(const CampaignState& c, const Game::SectorStats& s);
    bool update(float dt, const Input::InputState& in); // true = perk chosen, see next
    void render(Graphics::Renderer& r);
    CampaignState next;

private:
    CampaignState campaign;
    Game::SectorStats stats;
    std::array<Game::PerkKind, 3> perks{};
    float t = 0;
    int shownLines = 0;
    int focus = 0;
};

class Results {
public:
    enum class Action { NONE, RETRY, MENU };
    void enter(bool victory, int score, Difficulty d, bool newRecord, const CampaignState* retry);
    Action update(float dt, const Input::InputState& in);
    void render(Graphics::Renderer& r);
    /** Campaign to start when RETRY is chosen. */
    CampaignState retryCampaign() const;

private:
    bool victory = false;
    int score = 0;
    Difficulty difficulty = Difficulty::MEDIUM;
    bool newRecord = false;
    bool hasRetry = false;
    CampaignState retry;
    float t = 0;
    int focus = 0;
};

} // namespace Menus

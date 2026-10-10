// Desktop-only sanity checks for the campaign/score rules ported from the Phaser tests.
#undef NDEBUG // keep asserts in Release builds
#include <cassert>
#include <cstdio>
#include <set>
#include "LevelManager.hpp"
#include "ScoreManager.hpp"

int main() {
    using namespace Game;

    // Combo tiers: x1.5 at 3 kills, x2 at 8, x3 at 15; difficulty multiplies points.
    ScoreManager s(2, 100);
    bool tierUp = false;
    assert(s.addKill(10, false, tierUp) == 20 && !tierUp);
    s.addKill(10, false, tierUp);
    assert(s.addKill(10, false, tierUp) == 30 && tierUp && s.multiplier() == 1.5f);
    assert(s.addKill(10, true, tierUp) == 60); // crit doubles
    s.update(COMBO_WINDOW + 1);
    assert(s.comboKills == 0 && s.multiplier() == 1.0f);

    // Perks: three distinct choices; nanotech caps lives.
    for (int i = 0; i < 50; ++i) {
        auto p = rollPerks();
        assert(std::set<PerkKind>(p.begin(), p.end()).size() == 3);
    }
    CampaignState c = newCampaign(Difficulty::HARD, ShipId::BULWARK);
    assert(c.lives == 7 && c.level == 1);
    c.lives = MAX_LIVES;
    assert(applyPerk(c, PerkKind::NANOTECH).lives == MAX_LIVES);
    assert(applyPerk(c, PerkKind::MAGAZINE).perks[(int)PerkKind::MAGAZINE] == 1);

    // Roster grows with each sector's unlock.
    assert(roster(1).size() == 3);
    assert(roster(10).size() == 3 + 7); // sectors 2-8 each unlock one
    assert(getLevel(0).id == 1 && getLevel(99).id == LEVEL_COUNT);

    std::puts("selfcheck OK");
    return 0;
}

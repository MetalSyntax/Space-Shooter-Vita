#include "Menus.hpp"
#include "Assets.hpp"
#include "I18n.hpp"
#include "MathUtil.hpp"
#include "SaveSystem.hpp"
#include "ScoreManager.hpp"
#include "Text.hpp"
#include "audio/Synth.hpp"

namespace Menus {

using Graphics::Color;
using Graphics::Renderer;
using Graphics::rgb;
using Graphics::SCREEN_WIDTH;
using Graphics::SCREEN_HEIGHT;
using I18n::L;
using UI::Align;

static constexpr float CX = SCREEN_WIDTH / 2.0f;

static Audio::Synth& synth() { return Audio::Synth::instance(); }
static Storage::SaveData& save() { return Storage::SaveSystem::data(); }

static Game::ShipId selectedShip() {
    return static_cast<Game::ShipId>(std::clamp(save().selectedShip, 0, Game::SHIP_COUNT - 1));
}

static void drawSpace(Renderer& r, int level, float t, uint8_t alpha) {
    r.clear(5, 8, 20);
    r.drawBackgroundTiled(Game::assets().background(level), 0, (int)(t * 30), alpha);
}

/** Focus moves with a wrap-around (up/down). */
static int step(int value, int dir, int count) {
    return (value + dir + count) % count;
}

// ===================================================================== MainMenu

void MainMenu::enter() {
    refresh();
    for (size_t i = 0; i < stars.size(); i += 3) {
        stars[i] = Game::frand(0, SCREEN_WIDTH);
        stars[i + 1] = Game::frand(0, SCREEN_HEIGHT);
        stars[i + 2] = Game::frand(0, 6.28f);
    }
    auto rs = rows();
    focusRow = (int)(std::find(rs.begin(), rs.end(), hasSave ? Row::CONTINUE : Row::START) - rs.begin());
    focusCol = 0;
    synth().playMenuMusic();
}

void MainMenu::refresh() {
    hasSave = Game::loadCampaign(saved);
    auto rs = rows();
    focusRow = std::min(focusRow, (int)rs.size() - 1);
}

std::vector<MainMenu::Row> MainMenu::rows() const {
    std::vector<Row> rs = {Row::TOP, Row::DIFF};
    if (hasSave) rs.push_back(Row::CONTINUE);
    rs.push_back(Row::START);
    rs.push_back(Row::NAV);
    return rs;
}

int MainMenu::itemsIn(Row row) const {
    switch (row) {
        case Row::TOP: return 3;
        case Row::DIFF: return 3;
        case Row::NAV: return 2;
        default: return 1;
    }
}

void MainMenu::activate(Row row, int col, Action& out) {
    auto& d = save();
    switch (row) {
        case Row::TOP:
            if (col == 0) { synth().setMusicMuted(!synth().musicMuted()); d.musicMuted = synth().musicMuted(); }
            if (col == 1) { synth().setSfxMuted(!synth().sfxMuted()); d.sfxMuted = synth().sfxMuted(); }
            if (col == 2) { I18n::g_lang = I18n::isEn() ? I18n::Lang::ES : I18n::Lang::EN; d.lang = (int)I18n::g_lang; }
            Storage::SaveSystem::flush();
            break;
        case Row::DIFF: difficulty = static_cast<Difficulty>(col); break;
        case Row::CONTINUE: out = Action::CONTINUE; break;
        case Row::START: out = Action::NEW_GAME; break;
        case Row::NAV: out = col == 0 ? Action::HANGAR : Action::GUIDE; break;
    }
}

MainMenu::Action MainMenu::update(float dt, const Input::InputState& in) {
    t += dt;
    auto rs = rows();
    Row row = rs[focusRow];
    Action out = Action::NONE;

    if (in.up) { focusRow = step(focusRow, -1, (int)rs.size()); synth().playCombo(0); }
    if (in.down) { focusRow = step(focusRow, 1, (int)rs.size()); synth().playCombo(0); }
    row = rs[focusRow];
    if (row == Row::DIFF) focusCol = (int)difficulty;
    focusCol = std::min(focusCol, itemsIn(row) - 1);

    if (in.left || in.right) {
        int dir = in.left ? -1 : 1;
        if (row == Row::TOP || row == Row::NAV) {
            focusCol = step(focusCol, dir, itemsIn(row));
        } else {
            // Like the keyboard in the original: left/right always cycles difficulty.
            difficulty = static_cast<Difficulty>(step((int)difficulty, dir, 3));
            if (row == Row::DIFF) focusCol = (int)difficulty;
        }
        synth().playCombo(0);
    }
    if (in.confirm) activate(row, focusCol, out);
    if (in.pause) out = hasSave ? Action::CONTINUE : Action::NEW_GAME;
    return out;
}

void MainMenu::render(Renderer& r) {
    drawSpace(r, 1, -t, 180);
    for (size_t i = 0; i < stars.size(); i += 3) {
        float a = 0.45f + 0.35f * std::sin(t * 2.2f + stars[i + 2]);
        r.drawSprite(Game::assets().star, stars[i], stars[i + 1], 0, 1, 1, rgb(0xffffff, (uint8_t)(255 * a)));
    }

    auto rs = rows();
    Row frow = rs[focusRow];
    auto focused = [&](Row row, int col) { return frow == row && focusCol == col; };

    // Top bar: music / sfx / language
    std::string music = std::string("♪ ") + (synth().musicMuted() ? "OFF" : "ON");
    std::string sfx = std::string("SFX ") + (synth().sfxMuted() ? "OFF" : "ON");
    UI::drawButton(r, 16 + 60, 26, 120, 34, music, 0xffffff, focused(Row::TOP, 0), false, 2);
    UI::drawButton(r, 16 + 120 + 10 + 70, 26, 140, 34, sfx, 0xffffff, focused(Row::TOP, 1), false, 2);
    UI::drawButton(r, 16 + 270 + 20 + 40, 26, 80, 34, I18n::isEn() ? "EN" : "ES", 0x3dff6e, focused(Row::TOP, 2), false, 2);

    UI::drawTitle(r, "SPACE SHOOTER", CX, 52, 6, 0xfff27a, 0xff9d00, 0xb3160b);
    UI::drawText(r, "REMASTERED", CX, 108, 2, rgb(0x4dd9ff), Align::CENTER);

    Game::Tex* shipTex = Game::assets().ships[(int)selectedShip()];
    r.drawSprite(shipTex, CX, 156 + std::sin(t * 3.5f) * 4, -90, 1.0f, 1.0f);

    UI::drawText(r, L("SELECCIONA DIFICULTAD", "SELECT DIFFICULTY"), CX, 192, 2, rgb(0x8892b0), Align::CENTER);
    for (int i = 0; i < 3; ++i) {
        const auto& d = Game::difficultyDef(static_cast<Difficulty>(i));
        UI::drawButton(r, CX + (i - 1) * 196.0f, 236, 184, 42, L(d.nameEs, d.nameEn), d.color, focused(Row::DIFF, i), (int)difficulty == i, 2);
    }
    const auto& dd = Game::difficultyDef(difficulty);
    UI::drawText(r, L(dd.descEs, dd.descEn), CX, 268, 2, rgb(dd.color), Align::CENTER);

    uint8_t pulse = (uint8_t)(200 + 55 * std::sin(t * 5));
    if (hasSave) {
        std::string label = std::string(L("▶ CONTINUAR", "▶ CONTINUE")) + " S" + std::to_string(saved.level);
        UI::drawButton(r, CX, 322, 380, 48, label, 0xffdd33, focused(Row::CONTINUE, 0), true, 3, frow == Row::CONTINUE ? pulse : 255);
        UI::drawButton(r, CX, 376, 300, 36, L("NUEVA PARTIDA", "NEW GAME"), 0x4dd9ff, focused(Row::START, 0), false, 2);
    } else {
        UI::drawButton(r, CX, 340, 380, 52, L("▶ JUGAR", "▶ PLAY"), 0x4dd9ff, focused(Row::START, 0), true, 3, frow == Row::START ? pulse : 255);
    }

    int best = Game::ScoreManager::best();
    std::string record = std::string(L("RECORD", "HIGH SCORE")) + " ";
    if (best > 0) record += std::to_string(best) + " · " + Game::difficultyName(static_cast<Difficulty>(save().scores[0].difficulty));
    else record += "--";
    UI::drawText(r, record, CX, 408, 2, rgb(0xffdd33), Align::CENTER);

    std::string shipLabel = std::string(L("NAVE:", "SHIP:")) + " " + Game::getShipDef(selectedShip()).name;
    UI::drawButton(r, CX - 90, 456, 320, 36, shipLabel, 0xffdd33, focused(Row::NAV, 0), false, 2);
    UI::drawButton(r, CX + 170, 456, 180, 36, L("GUIA", "GUIDE"), 0x4dd9ff, focused(Row::NAV, 1), false, 2);

    UI::drawText(r, L("CRUZ: ACEPTAR · CRUCETA/STICK: ELEGIR · START: JUGAR", "CROSS: SELECT · D-PAD/STICK: MOVE · START: PLAY"), CX, 514, 1.5f, rgb(0x5a6488), Align::CENTER);
}

// ===================================================================== Hangar

void Hangar::enter() {
    current = std::clamp(save().selectedShip, 0, Game::SHIP_COUNT - 1);
    t = 0;
}

bool Hangar::update(float dt, const Input::InputState& in) {
    t += dt;
    if (in.left || in.right) {
        current = step(current, in.left ? -1 : 1, Game::SHIP_COUNT);
        save().selectedShip = current;
        Storage::SaveSystem::flush();
        synth().playCombo(0);
    }
    return in.confirm || in.back;
}

void Hangar::render(Renderer& r) {
    r.clear(5, 8, 20);
    const Game::ShipDef& s = Game::getShipDef(static_cast<Game::ShipId>(current));
    UI::drawTitle(r, "HANGAR", CX, 20, 4, 0xfff27a, 0xff9d00, 0x5a2a00);
    UI::drawText(r, std::to_string(current + 1) + " / " + std::to_string(Game::SHIP_COUNT), CX, 66, 2, rgb(0x8892b0), Align::CENTER);

    float shipY = 148 + std::sin(t * 3.5f) * 6;
    r.drawSprite(Game::assets().ships[current], CX, shipY, -90, 2, 2);
    UI::drawButton(r, CX - 220, 148, 48, 48, "◀", 0x4dd9ff, false, false, 2);
    UI::drawButton(r, CX + 220, 148, 48, 48, "▶", 0x4dd9ff, false, false, 2);

    UI::drawText(r, s.name, CX, 214, 3, rgb(s.color), Align::CENTER);
    UI::drawText(r, L(s.abilityEs, s.abilityEn), CX, 246, 2, rgb(s.color), Align::CENTER);
    float descH = UI::drawWrapped(r, L(s.descEs, s.descEn), CX, 270, 2, 640, rgb(0xc8d0e8), Align::CENTER);

    struct Stat { const char* es; const char* en; float value; float max; };
    const Stat stats[4] = {
        {"VELOCIDAD", "SPEED", s.speed, 1.3f},
        {"CADENCIA", "FIRE RATE", s.fireRate * (1 + s.extraShots), 2.3f},
        {"VIDAS", "LIVES", (float)s.lives, 7},
        {"BOMBAS", "BOMBS", (float)s.bombs, 3},
    };
    float barW = 300, x0 = CX - (barW + 180) / 2 + 180;
    float y0 = 270 + descH + 22;
    for (int i = 0; i < 4; ++i) {
        float y = y0 + i * 28;
        UI::drawText(r, L(stats[i].es, stats[i].en), x0 - 180, y, 2, rgb(0xc8d0e8));
        r.drawRect(x0, y, barW, 14.0f, rgb(0x1e293b));
        r.drawRect(x0, y, barW * std::min(1.0f, stats[i].value / stats[i].max), 14.0f, rgb(s.color));
    }

    Game::CampaignState tmp;
    if (Game::loadCampaign(tmp)) UI::drawText(r, L("SE APLICA AL EMPEZAR UNA NUEVA PARTIDA", "APPLIES WHEN STARTING A NEW GAME"), CX, 462, 2, rgb(0x8892b0), Align::CENTER);
    UI::drawButton(r, CX, 506, 260, 44, L("LISTO", "READY"), 0xffdd33, true, false, 2);
}

// ===================================================================== Guide

static const char* BOSS_DESC[10][2] = {
    {"CANON PESADO EN V Y ASTEROIDES QUE LO ORBITAN COMO ESCUDO", "HEAVY V-CANNON & ORBITING ASTEROID SHIELDS"},
    {"EMBESTIDAS EN ZIG-ZAG Y ABANICOS DE PUAS DE PLASMA", "ZIG-ZAG RAM DASHES & PLASMA SPIKE FANS"},
    {"ORBES GIRATORIOS BLOQUEAN TUS DISPAROS: ATACA POR LOS FLANCOS", "ROTATING ORBS BLOCK SHOTS: ATTACK FROM FLANKS"},
    {"DOS TORRETAS ALARES; SI PIERDE UNA DISPARA RAYOS CONCENTRADOS", "TWIN WING TURRETS; DESTROYING ONE TRIGGERS BEAMS"},
    {"SIEMBRA MINAS QUE ESTALLAN EN ANILLOS", "LAYS PROXIMITY MINES THAT BURST INTO BULLET RINGS"},
    {"SE TELETRANSPORTA Y CREA SENUELOS CON DISPAROS FALSOS", "TELEPORTS AND GENERATES HOLOGRAPHIC DECOYS"},
    {"NAVE NODRIZA QUE DESPLIEGA ENJAMBRES KAMIKAZE", "CARRIER DEPLOYING PERSISTENT KAMIKAZE SWARMS"},
    {"LASER DE BARRIDO ANUNCIADO CON UNA LINEA GUIA", "SWEEPING MEGA-LASER WITH GUIDE TELEGRAPH LINE"},
    {"VORTICE GRAVITATORIO QUE TE ATRAE MIENTRAS DISPARA ESPIRALES", "GRAVITATIONAL VORTEX PULLS PLAYER WHILE FIRING SPIRALS"},
    {"3 FASES: FORTALEZA, ESPIRAL (50%) Y AUTODESTRUCCION (20%)", "3 PHASES: CITADEL, SPIRAL (50%) & SELF-DESTRUCT (20%)"},
};

static constexpr float GUIDE_TOP = 104;
static constexpr float GUIDE_BOTTOM = 470;
static constexpr float GUIDE_COL = 760;

static int firstSector(Game::EnemyKind k) {
    for (const auto& l : Game::getLevels()) {
        if (l.unlock == k) return l.id;
    }
    return 1;
}

std::vector<Guide::Row> Guide::buildRows() const {
    using Game::Ability;
    using Game::abilityDef;
    const auto& a = Game::assets();
    std::vector<Row> rows;
    auto secs = [](int i) { return std::to_string((int)(Game::POWER_DURATIONS[i] / 1000)) + "S"; };
    auto ab = [](Ability c) { return std::string(L(abilityDef(c).nameEs, abilityDef(c).nameEn)); };
    auto sector = [](Game::EnemyKind k) { return " · SECTOR " + std::to_string(firstSector(k)); };

    if (tab == 0) {
        rows.push_back({a.powerups[(int)Game::Power::SHIELD], std::string(L("ESCUDO", "SHIELD")) + " · " + secs(0), 0x3d7bff, L("ABSORBE IMPACTOS Y DESTRUYE A LOS ENEMIGOS QUE TE CHOCAN", "ABSORBS IMPACTS AND DESTROYS COLLIDING ENEMIES")});
        rows.push_back({a.powerups[(int)Game::Power::RAPID], std::string(L("DISPARO RAPIDO", "RAPID FIRE")) + " · " + secs(1), 0xffd21f, L("DOBLA LA CADENCIA DE FUEGO", "DOUBLES FIRING RATE")});
        rows.push_back({a.powerups[(int)Game::Power::BOMB], "HYPER BOMB +1", 0xff3333, L("CARGA EXTRA (MAX 5). LANZALA CON CUADRADO O CIRCULO: BORRA LAS BALAS ENEMIGAS", "EXTRA STOCK (MAX 5). TRIGGER WITH SQUARE OR CIRCLE: CLEARS ALL ENEMY BULLETS")});
        rows.push_back({a.powerups[(int)Game::Power::SPREAD], std::string(L("ABANICO", "SPREAD FAN")) + " · " + secs(2), 0x3dff6e, L("3 A 5 PROYECTILES EN ARCO", "3 TO 5 PROJECTILES IN AN ARC")});
        rows.push_back({a.powerups[(int)Game::Power::MISSILE], std::string(L("MISILES", "MISSILES")) + " · " + secs(3), 0xff8a1f, L("MISILES TELEDIRIGIDOS DESDE LAS ALAS", "HOMING MISSILES LAUNCHED FROM SHIP WINGS")});
        rows.push_back({a.powerups[(int)Game::Power::DRONE], std::string(L("DRON", "DRONE")) + " · " + secs(4), 0xc8d0e8, L("SATELITE QUE ORBITA Y DISPARA ORBES TELEDIRIGIDOS", "ESCORT SATELLITE FIRING HOMING PLASMA ORBS")});
        rows.push_back({a.powerups[(int)Game::Power::MAGNET], std::string(L("IMAN", "MAGNET")) + " · " + secs(5), 0xb04dff, L("ATRAE LOS POWER-UPS HACIA TU NAVE", "DRAWS POWER-UPS DIRECTLY TOWARD YOUR SHIP")});
        for (int p = 0; p < 4; ++p) {
            const auto& info = Game::perkInfo(static_cast<Game::PerkKind>(p));
            rows.push_back({nullptr, std::string(L("MEJORA: ", "UPGRADE: ")) + L(info.titleEs, info.titleEn), 0x4dd9ff,
                            std::string(L(info.descEs, info.descEn)) + L(" · SE ELIGE AL LIMPIAR UN SECTOR", " · CHOSEN AFTER CLEARING A SECTOR")});
        }
    } else if (tab == 1) {
        using K = Game::EnemyKind;
        rows.push_back({a.enDrone, L("DRONE · BLANCO", "DRONE · WHITE"), 0xe0e0e0, L("SIN HABILIDAD. VUELA RECTO, EN ZIG-ZAG O EN FORMACION V · SECTOR 1", "STANDARD HULL. FLIES STRAIGHT, ZIG-ZAG OR V-FORMATION · SECTOR 1")});
        rows.push_back({a.enGunshipRed, L("GUNSHIP · ROJO", "GUNSHIP · RED"), abilityDef(Ability::RED).color, ab(Ability::RED) + L(": TE APUNTA CADA 2 SEGUNDOS · SECTOR 1", ": AIMS AT YOU EVERY 2 SECONDS · SECTOR 1")});
        rows.push_back({a.enLancerYellow, L("LANCER · AMARILLO", "LANCER · YELLOW"), abilityDef(Ability::YELLOW).color, ab(Ability::YELLOW) + L(": SE DETIENE, PARPADEA Y SE LANZA HACIA TI", ": STOPS, CHARGES AND RAMS TOWARD YOU") + sector(K::LANCER)});
        rows.push_back({a.enPodBlue, L("POD · AZUL", "POD · BLUE"), abilityDef(Ability::BLUE).color, ab(Ability::BLUE) + L(": SU BURBUJA ABSORBE 3 IMPACTOS", ": BUBBLE SHIELD ABSORBS 3 HITS") + sector(K::POD)});
        rows.push_back({a.enGunshipOrange, L("GUNSHIP · NARANJA", "GUNSHIP · ORANGE"), abilityDef(Ability::ORANGE).color, ab(Ability::ORANGE) + L(": RAFAGA DE 3 A 5 DISPAROS", ": BURST OF 3 TO 5 SPREAD SHOTS") + sector(K::SPREADER)});
        rows.push_back({a.enSpiderGreen, L("SPIDERLING · VERDE", "SPIDERLING · GREEN"), abilityDef(Ability::GREEN).color, ab(Ability::GREEN) + L(": AL MORIR SUELTA 2 DRONES", ": SPLITS INTO 2 ESCORT DRONES ON DEATH") + sector(K::SPLITTER)});
        rows.push_back({a.enDronePurple, L("BLINKER · PURPURA", "BLINKER · PURPLE"), abilityDef(Ability::PURPLE).color, ab(Ability::PURPLE) + L(": SALTA DE LADO Y DISPARA AL REAPARECER", ": TELEPORTS SIDEWAYS AND SHOOTS ON RETURN") + sector(K::BLINKER)});
        rows.push_back({a.enDroneYellow, L("KAMIKAZE · AMARILLO", "KAMIKAZE · YELLOW"), abilityDef(Ability::YELLOW).color, std::string(L("PERSIGUE Y CHOCA, LLEGA EN ENJAMBRES", "FAST SWARM DART LOCKING IN FOR IMPACT")) + sector(K::KAMIKAZE)});
        rows.push_back({a.enGunshipCyan, L("GUNSHIP · CIAN", "GUNSHIP · CYAN"), abilityDef(Ability::CYAN).color, ab(Ability::CYAN) + L(": SE DETIENE Y DISPARA UN RAYO ANUNCIADO", ": STOPS AND FIRES TELEGRAPHED LASER BEAM") + sector(K::LASER)});
        rows.push_back({a.asteroid, L("ASTEROIDE", "ASTEROID"), 0xc8d0e8, L("GIRA A LA DERIVA Y SE ROMPE EN FRAGMENTOS", "TUMBLES DRIFTING AND SPLINTERS INTO FRAGMENTS")});
        rows.push_back({a.levels[4].mine, L("MINA", "PROXIMITY MINE"), 0xff3333, L("TE PERSIGUE Y A LOS 3 SEGUNDOS ESTALLA EN 8 PROYECTILES", "HOMES IN AND DETONATES INTO AN 8-WAY RING")});
    } else {
        for (const auto& l : Game::getLevels()) {
            rows.push_back({a.levels[l.id - 1].boss, "S" + std::to_string(l.id) + " · " + l.bossName, 0xff6666,
                            l.name + ": " + BOSS_DESC[l.id - 1][I18n::isEn() ? 1 : 0]});
        }
    }
    return rows;
}

static float rowHeight(const std::string& desc, float icon) {
    float textW = GUIDE_COL - icon - 12;
    float descH = UI::wrap(desc, 2, textW).size() * (UI::lineHeight(2) + 4);
    return std::max(icon, 16 + 8 + descH) + 16;
}

float Guide::contentHeight() const {
    float icon = tab == 2 ? 72.0f : 52.0f, h = 8;
    for (const auto& row : buildRows()) h += rowHeight(row.desc, icon);
    return h;
}

void Guide::enter() {
    tab = 0;
    scroll = 0;
}

bool Guide::update(float dt, const Input::InputState& in) {
    if (in.tabLeft || in.left) { tab = step(tab, -1, 3); scroll = 0; synth().playCombo(0); }
    if (in.tabRight || in.right) { tab = step(tab, 1, 3); scroll = 0; synth().playCombo(0); }
    float maxScroll = std::max(0.0f, contentHeight() - (GUIDE_BOTTOM - GUIDE_TOP));
    float axis = std::fabs(in.aimY) > 0.25f ? in.aimY : in.moveY;
    scroll = std::clamp(scroll + axis * 520 * dt, 0.0f, maxScroll);
    return in.back || in.confirm;
}

void Guide::render(Renderer& r) {
    r.clear(5, 8, 20);
    UI::drawTitle(r, L("GUIA", "GUIDE"), CX, 16, 4, 0x9ff6ff, 0x2f9fff, 0x062a4a);
    const char* tabs[3][2] = {{"PODERES", "POWERS"}, {"ENEMIGOS", "ENEMIES"}, {"JEFES", "BOSSES"}};
    float tabW = (GUIDE_COL - 16) / 3;
    for (int i = 0; i < 3; ++i) UI::drawButton(r, CX + (i - 1) * (tabW + 8), 74, tabW, 36, L(tabs[i][0], tabs[i][1]), 0x4dd9ff, false, tab == i, 2);

    float left = CX - GUIDE_COL / 2;
    float icon = tab == 2 ? 72.0f : 52.0f;
    r.setClip(0, (int)GUIDE_TOP, SCREEN_WIDTH, (int)(GUIDE_BOTTOM - GUIDE_TOP));
    float y = GUIDE_TOP + 8 - scroll;
    for (const auto& row : buildRows()) {
        float h = rowHeight(row.desc, icon);
        if (y + h > GUIDE_TOP && y < GUIDE_BOTTOM) {
            if (row.tex) {
                float tw = (float)Renderer::texWidth(row.tex), th = (float)Renderer::texHeight(row.tex);
                float s = std::min({icon / tw, icon / th, tab == 2 ? 1.0f : 1.2f});
                r.drawSprite(row.tex, left + icon / 2, y + icon / 2, 0, s, s);
            }
            float tx = left + (row.tex ? icon + 12 : 0);
            UI::drawText(r, row.title, tx, y + 4, 2, rgb(row.color));
            UI::drawWrapped(r, row.desc, tx, y + 28, 2, left + GUIDE_COL - tx, rgb(0xc8d0e8));
        }
        y += h;
    }
    r.clearClip();

    float maxScroll = std::max(0.0f, contentHeight() - (GUIDE_BOTTOM - GUIDE_TOP));
    if (maxScroll > 0) {
        float trackH = GUIDE_BOTTOM - GUIDE_TOP;
        float barH = std::max(30.0f, trackH * trackH / (trackH + maxScroll));
        r.drawRect(left + GUIDE_COL + 14, GUIDE_TOP, 4.0f, trackH, rgb(0x1e293b));
        r.drawRect(left + GUIDE_COL + 14, GUIDE_TOP + (trackH - barH) * scroll / maxScroll, 4.0f, barH, rgb(0x4dd9ff));
    }
    UI::drawButton(r, CX, 506, 260, 40, L("VOLVER", "BACK"), 0xffdd33, true, false, 2);
    UI::drawText(r, L("L/R: PESTANA · STICK: DESPLAZAR", "L/R: TAB · STICK: SCROLL"), CX, 532, 1, rgb(0x5a6488), Align::CENTER);
}

// ===================================================================== Pause

void PauseMenu::enter() {
    focus = 0;
}

PauseMenu::Action PauseMenu::update(float, const Input::InputState& in) {
    if (in.up) focus = step(focus, -1, 6);
    if (in.down) focus = step(focus, 1, 6);
    if (in.pause || in.back) return Action::RESUME;
    if (!in.confirm) return Action::NONE;
    auto& d = save();
    switch (focus) {
        case 0: return Action::RESUME;
        case 1: return Action::RESTART;
        case 2: synth().setMusicMuted(!synth().musicMuted()); d.musicMuted = synth().musicMuted(); Storage::SaveSystem::flush(); break;
        case 3: synth().setSfxMuted(!synth().sfxMuted()); d.sfxMuted = synth().sfxMuted(); Storage::SaveSystem::flush(); break;
        case 4: return Action::GUIDE;
        case 5: return Action::MENU;
    }
    return Action::NONE;
}

void PauseMenu::render(Renderer& r) {
    r.drawRect(0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, rgb(0x000000, 178));
    UI::drawText(r, L("PAUSA", "PAUSE"), CX, 70, 4, rgb(0x4dd9ff), Align::CENTER);
    std::string items[6] = {
        L("▶ REANUDAR", "▶ RESUME"),
        L("REINICIAR", "RESTART"),
        std::string("♪ ") + L("MUSICA", "MUSIC") + ": " + (synth().musicMuted() ? "OFF" : "ON"),
        std::string("SFX: ") + (synth().sfxMuted() ? "OFF" : "ON"),
        L("GUIA", "GUIDE"),
        L("MENU PRINCIPAL", "MAIN MENU"),
    };
    for (int i = 0; i < 6; ++i) {
        UI::drawButton(r, CX, 150 + i * 56.0f, 340, 46, items[i], i == 0 ? 0x4dd9ff : 0xffdd33, focus == i, i == 0, 2);
    }
}

// ===================================================================== LevelClear

void LevelClear::enter(const CampaignState& c, const Game::SectorStats& s) {
    campaign = c;
    stats = s;
    perks = Game::rollPerks();
    t = 0;
    shownLines = 0;
    focus = 0;
    synth().playVictoryMusic();
}

bool LevelClear::update(float dt, const Input::InputState& in) {
    t += dt;
    // Count the stat lines in one at a time.
    int lines = std::min(4, (int)(t / 0.25f));
    while (shownLines < lines) {
        shownLines++;
        synth().playCombo(0);
    }
    if (t < 1.3f) return false;
    if (in.up) focus = step(focus, -1, 3);
    if (in.down) focus = step(focus, 1, 3);
    if (!in.confirm) return false;
    synth().playPowerUp();
    next = Game::applyPerk(campaign, perks[focus]);
    next.level = campaign.level + 1;
    Game::saveCampaign(next);
    synth().stopMusic();
    return true;
}

void LevelClear::render(Renderer& r) {
    const Game::LevelDef& lv = Game::getLevel(campaign.level);
    const Game::LevelDef& nx = Game::getLevel(campaign.level + 1);
    drawSpace(r, nx.id, t, 150);
    UI::drawTitle(r, L("SECTOR SUPERADO", "SECTOR CLEARED"), CX, 24, 4, 0x9dffb0, 0x22cc55, 0x06351a);
    UI::drawText(r, lv.name + " · " + lv.bossName + " " + L("DESTRUIDO", "DESTROYED"), CX, 70, 2, rgb(0xc8d0e8), Align::CENTER);

    auto pad = [](std::string s) {
        while (s.size() < 11) s += ' ';
        return s;
    };
    std::string lines[4] = {
        pad(L("ENEMIGOS", "ENEMIES")) + " " + std::to_string(stats.kills),
        pad(L("PRECISION", "ACCURACY")) + " " + std::to_string((int)std::lround(stats.accuracy * 100)) + "%",
        pad("BONUS") + " +" + std::to_string(stats.bonus),
        pad(L("PUNTOS", "SCORE")) + " " + std::to_string(campaign.score),
    };
    for (int i = 0; i < shownLines; ++i) UI::drawText(r, lines[i], CX - 150, 100 + i * 26.0f, 2, rgb(0xffffff));

    UI::drawText(r, L("ELIGE UNA MEJORA DE SECTOR", "CHOOSE A SECTOR UPGRADE"), CX, 222, 2, rgb(0xffdd33), Align::CENTER);
    for (int i = 0; i < 3; ++i) {
        float appear = std::clamp((t - 1.3f - i * 0.15f) / 0.25f, 0.0f, 1.0f);
        if (appear <= 0) continue;
        const auto& info = Game::perkInfo(perks[i]);
        std::string label = std::to_string(i + 1) + ". " + L(info.titleEs, info.titleEn) + "\n" + L(info.descEs, info.descEn);
        UI::drawButton(r, CX, 280 + i * 72.0f, 580, 62, label, 0x4dd9ff, t >= 1.3f && focus == i, false, 2, (uint8_t)(255 * appear));
    }
    UI::drawText(r, std::string(L("SIGUIENTE: SECTOR", "NEXT: SECTOR")) + " " + std::to_string(nx.id) + " · " + nx.name, CX, 506, 2, rgb(0x8892b0), Align::CENTER);
}

// ===================================================================== GameOver / Victory

void Results::enter(bool v, int s, Difficulty d, bool record, const CampaignState* r) {
    victory = v;
    score = s;
    difficulty = d;
    newRecord = record;
    hasRetry = r != nullptr;
    if (r) retry = *r;
    t = 0;
    focus = 0;
    if (victory) synth().playVictoryMusic();
    else synth().playGameOverMusic();
}

CampaignState Results::retryCampaign() const {
    // Retry the same sector from its auto-save (with at least 3 lives), or a new run after victory.
    if (hasRetry) {
        CampaignState c = retry;
        c.lives = std::max(c.lives, 3);
        return c;
    }
    return Game::newCampaign(difficulty, selectedShip());
}

Results::Action Results::update(float dt, const Input::InputState& in) {
    t += dt;
    if (in.up || in.down) focus ^= 1;
    if (in.back) return Action::MENU;
    if (in.confirm) return focus == 0 ? Action::RETRY : Action::MENU;
    return Action::NONE;
}

void Results::render(Renderer& r) {
    drawSpace(r, hasRetry ? retry.level : 10, t, 128);
    if (victory) UI::drawTitle(r, L("VICTORIA", "VICTORY"), CX, 40, 6, 0xfff27a, 0xffc400, 0x7a4a00);
    else UI::drawTitle(r, "GAME OVER", CX, 40, 6, 0xff8a8a, 0xff1a1a, 0x4a0000);
    UI::drawText(r, std::string(L("PUNTOS", "SCORE")) + " " + std::to_string(score), CX, 112, 3, rgb(0xffffff), Align::CENTER);
    if (newRecord && ((int)(t / 0.4f) % 2 == 0)) UI::drawText(r, L("¡NUEVO RECORD!", "NEW HIGH SCORE!"), CX, 146, 2, rgb(0xffdd33), Align::CENTER);

    // Top 5 local scores with difficulty and date
    UI::drawText(r, L("MEJORES PUNTUACIONES", "HIGH SCORES"), CX, 182, 2, rgb(0xaaccff), Align::CENTER);
    const auto& d = save();
    for (int i = 0; i < std::min(5, d.scoreCount); ++i) {
        char buf[64];
        std::string diff = Game::difficultyName(static_cast<Difficulty>(std::clamp(d.scores[i].difficulty, 0, 2)));
        while (diff.size() < 7) diff += ' ';
        std::snprintf(buf, sizeof(buf), "%d. %6d %s %s", i + 1, d.scores[i].score, diff.c_str(), d.scores[i].date);
        UI::drawText(r, buf, CX, 214 + i * 26.0f, 2, rgb(0xaaccff), Align::CENTER);
    }

    std::string retryLabel = hasRetry ? std::string(L("REINTENTAR S", "RETRY S")) + std::to_string(retry.level) : L("JUGAR DE NUEVO", "PLAY AGAIN");
    UI::drawButton(r, CX, 430, 340, 52, retryLabel, 0x4dd9ff, focus == 0, true, 2);
    UI::drawButton(r, CX, 494, 204, 40, L("MENU", "MENU"), 0xffffff, focus == 1, false, 2);
}

} // namespace Menus

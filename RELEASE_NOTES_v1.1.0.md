# Shooter Space v1.1.0 — PS Vita

Full parity with the web version (Phaser): every screen, ship, enemy and boss from the original, now running natively on PS Vita.

**Install:** copy `ShooterSpace.vpk` to your console and install it with VitaShell (Title ID `SHTR00001`, version 01.10). If you have v1.0 installed, uninstall it first. Your previous high score is kept.

## What's new

### Screens
- **Main menu:** difficulty (Easy / Normal / Hard), Continue from the saved sector or New Game, high score, plus music, SFX and language toggles.
- **Hangar:** 5 ships (Vanguard, Striker, Phantom, Bulwark, Nova) with their original art, unique ability and stats.
- **Guide:** power-ups, enemies and bosses.
- **Pause:** resume, restart, music, SFX, guide and main menu.
- **Sector Cleared:** stats (kills, accuracy, bonus) and a choice of 1 out of 3 upgrades.
- **Game Over and Victory:** local top 5 high scores.

### Gameplay
- 10-sector campaign that autosaves at the start of each sector.
- Combos (x1.5 / x2 / x3), critical hits and 7 power-ups: shield, rapid fire, spread shot, missiles, drone, magnet and Hyper Bomb.
- Full enemy roster, where each color means an ability: drones, gunships, lancers, shielded pods, spreaders, splitting spiders, blinkers, kamikazes, laser ships, asteroids and mines.
- All 10 bosses with their own attacks: telegraphed beams, turrets, shield orbs, decoys, swarms, a gravity vortex and a 3-phase final boss.
- Per-sector scenery and effects (asteroids, plasma, rings, structures, lightning, pulse).
- Full HUD: score, combo, lives, bombs, power-up timers, sector progress and boss health bar.

### Presentation
- Press Start 2P font on all text.
- Spanish and English; on first launch the game follows the system language.
- Procedural music: menu theme, combat theme with its own tempo per sector (faster during boss fights), and victory and defeat jingles.
- Every sound effect from the web version.
- Trophies persist between sessions.

## Controls

| Action | In game | In menus |
|---|---|---|
| Move / navigate | Left stick or D-pad | Left stick or D-pad |
| Aim | Right stick | — |
| Fire / confirm | R or Cross | Cross |
| Hyper Bomb / back | Square or Circle | Circle |
| Pause | Start | Start (main menu: play) |
| Switch tab | — | L / R (Guide) |

## Fixes
- Error `0x8010113D` when launching the app: `icon0.png` is an 8-bit indexed PNG again.

## Notes
- The touch controls from the mobile web version are not included.
- Save file: `ux0:data/shooterspace/save.dat` (format v2, the v1.0 save is migrated automatically).

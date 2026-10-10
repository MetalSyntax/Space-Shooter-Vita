# Shooter Space v1.1.0 — PS Vita

Paridad completa con la versión web (Phaser): todas las pantallas, naves, enemigos y jefes del original, ahora nativos en PS Vita.

**Instalación:** copia `ShooterSpace.vpk` a la consola e instálalo con VitaShell (Title ID `SHTR00001`, versión 01.10). Si tenías la v1.0, desinstálala antes. Tu récord anterior se conserva.

## Novedades

### Pantallas
- **Menú principal:** dificultad (Fácil / Normal / Difícil), Continuar desde el sector guardado o Nueva partida, récord, y botones de música, SFX e idioma.
- **Hangar:** 5 naves (Vanguard, Striker, Phantom, Bulwark, Nova) con su arte original, habilidad única y estadísticas.
- **Guía:** poderes, enemigos y jefes.
- **Pausa:** reanudar, reiniciar, música, SFX, guía y menú principal.
- **Sector superado:** estadísticas (enemigos, precisión, bonus) y elección de 1 de 3 mejoras.
- **Game Over y Victoria:** top 5 de puntuaciones locales.

### Juego
- Campaña de 10 sectores con guardado automático al empezar cada uno.
- Combos (x1.5 / x2 / x3), golpes críticos y 7 power-ups: escudo, disparo rápido, abanico, misiles, dron, imán y Hyper Bomb.
- Roster completo de enemigos, cada color con su habilidad: drones, gunships, lancers, pods con escudo, spreaders, arañas que se dividen, blinkers, kamikazes, naves láser, asteroides y minas.
- Los 10 jefes con sus ataques propios: rayos anunciados, torretas, orbes escudo, señuelos, enjambres, vórtice gravitatorio y 3 fases en el jefe final.
- Decorado y efectos por sector (asteroides, plasma, anillos, estructuras, relámpagos, pulso).
- HUD completo: puntuación, combo, vidas, bombas, temporizadores de poderes, progreso del sector y barra del jefe.

### Presentación
- Fuente Press Start 2P en todos los textos.
- Español e inglés; el primer arranque usa el idioma del sistema.
- Música procedural: tema de menú, tema de combate con tempo propio por sector (más rápido contra el jefe) y jingles de victoria y derrota.
- Todos los efectos de sonido de la versión web.
- Trofeos guardados entre partidas.

## Controles

| Acción | En juego | En menús |
|---|---|---|
| Moverse / navegar | Stick izquierdo o cruceta | Stick izquierdo o cruceta |
| Apuntar | Stick derecho | — |
| Disparar / aceptar | R o Cruz | Cruz |
| Hyper Bomb / volver | Cuadrado o Círculo | Círculo |
| Pausa | Start | Start (en el menú: jugar) |
| Cambiar pestaña | — | L / R (Guía) |

## Correcciones
- Error `0x8010113D` al abrir la app: `icon0.png` vuelve a ser PNG indexado de 8 bits.

## Notas
- No incluye los controles táctiles de la versión móvil web.
- Guardado en `ux0:data/shooterspace/save.dat` (formato v2, migra automáticamente el de la v1.0).

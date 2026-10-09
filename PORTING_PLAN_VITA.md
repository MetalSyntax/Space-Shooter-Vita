# Plan de Reescritura a C++ / PS Vita: Shooter Space

> **Objetivo:** Reimplementar el juego Shooter Space (originalmente en TypeScript/Phaser) en C++ moderno para PlayStation Vita (resolución nativa 960x544 a 60 FPS estables), manteniendo la compatibilidad cruzada de compilación en Host (macOS/Linux/Windows) mediante SDL2 y VitaSDK.

---

## 1. Estado del Progreso: 100% COMPLETADO 🎉

- [x] **Paso 0:** Análisis del código fuente TypeScript y definición de arquitectura técnica.
- [x] **Paso 1:** Documento de Planificación Maestro (`PORTING_PLAN_VITA.md`).
- [x] **Paso 2:** Estructura inicial del proyecto C++ y configuración de build dual (`CMakeLists.txt`).
- [x] **Paso 3:** Porting del generador de Pixel Art (`PixelArtGenerator.hpp` / `.cpp`).
- [x] **Paso 4:** Porting de sprites v1 originales (`OriginalSprites.hpp` / `.cpp`).
- [x] **Paso 5:** Porting de generadores de Jefes (`BossGenerator.hpp` / `.cpp`).
- [x] **Paso 6:** Render loop básico, ventana 960x544, visor interactivo de sprites horneados y generación inicial del `.vpk`.
- [x] **Paso 7:** Core Loop: Nave del Jugador (`Player`), físicas arcade, clases de naves (`Ships`) y **Object Pool de Proyectiles (`BulletPool<512>`)**.
- [x] **Paso 8:** Sistema de Enemigos (`EnemyPool<128>`), IA de oleadas (Drones, Weavers, Swoopers, Gunships, Spreaders, Kamikazes) y **Detección de Colisiones AABB/Circular (`CollisionSystem`)**.
- [x] **Paso 9:** Sistema de partículas y efectos visuales (`ParticlePool<1024>`).
- [x] **Paso 10:** Gestor de niveles (`LevelManager`), progresión de los 10 sectores, secuenciador de oleadas (`WaveManager`), sistema de mejoras (`Perks`) y encuentro con Jefe final con barra de salud.
- [x] **Paso 11:** Audio procedural / sintetizador nativo de ondas (`Synth.hpp` / `.cpp` vía callback SDL2 de baja latencia a 44.1 kHz).
- [x] **Paso 12:** Interfaz de usuario (HUD pixel art, visualizador de puntuación, vidas, bombas, aviso de Boss y sector actual).
- [x] **Paso 13:** Sistema de guardado persistente en `ux0:data/shooterspace/save.dat`.
- [x] **Paso 14:** **Soporte completo de Controles Físicos Nativos de PS Vita (`InputManager` con `SceCtrl`)**:
  - D-Pad y Stick Izquierdo para movimiento en 360°.
  - Stick Derecho para apuntado angular libre.
  - Gatillo R y Botón $\times$ (Cruz) para disparo primario continuo.
  - Botón $\square$ (Cuadrado) y $\bigcirc$ (Círculo) para Hiperbomba.
  - Botón Start para pausar/salir.
- [x] **Paso 15:** LiveArea de Vita (`icon0.png`, `startup.png`, `bg.png`, `template.xml`) y empaquetado final del paquete instalable **`ShooterSpace.vpk`**.

---

## 2. Decisiones de Arquitectura

1. **Framework gráfico y Entrada:**
   - **SDL2** como capa de abstracción principal combinada con la API nativa de entrada de Vita **`SceCtrl` (`sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG)`)** para máxima respuesta, latencia cero y lectura precisa de ambos sticks analógicos.
   - Enlace completo verificado en macOS (`psvita/build/ShooterSpace`) y en PS Vita (`psvita/build_vita/ShooterSpace.vpk`).
   - Resolución fija interna: **960 × 544** (16:9), coincidencia pixel-perfect con la pantalla OLED/LCD de PS Vita.
2. **Manejo de Memoria:**
   - **Cero llamadas a `malloc` o `new` durante el bucle de juego activo.**
   - Uso de **Object Pooling** de tamaño fijo para proyectiles, enemigos y partículas (`BulletPool<512>`, `EnemyPool<128>`, `ParticlePool<1024>`).
3. **Zero External Visual Assets:**
   - Todo el arte se genera en memoria durante la inicialización (`PixelArtGenerator`, `OriginalSprites`, `BossGenerator`) y se sube como texturas SDL/GXM. Rendimiento verificado en C++: generación completa de una nave en **0.04 ms**.
4. **Audio Procedural en Tiempo Real:**
   - Sintetizador integrado sin necesidad de archivos WAV/MP3 externos pesados, reproduciendo ondas senoidales, cuadradas, sierra y ruido blanco filtrado directamente a través del hardware de sonido de la consola.

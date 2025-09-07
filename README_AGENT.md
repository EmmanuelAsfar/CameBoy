Agent notes - CameBoy
=====================

Build & Logs
------------
- Main script: `cameboy.bat`
- Build outputs: `build\bin\cameboy.exe`, `build\bin\cameboy_gui.exe`
- Build log: `build\build.log` (always overwritten per build)
- Unit test log: `logs\test_results.log`
- Emulator logs: `logs\emulator.log`, `logs\emulator_gui.log`

Resources (GUI)
---------------
- Source resources live in `resources/` at project root.
- Build copies `resources/` to `build\resources/`.
- GUI loads background image via `resource_manager`:
  - `GetModuleFileNameA` → remove exe → remove `bin` → append `resources`
  - Loads BMP with `LoadImageA` using `LR_LOADFROMFILE|LR_CREATEDIBSECTION`.
- Expected file: `build\resources\gameboy_bg.bmp`.

Running
-------
- `cameboy.bat build` compiles both simple and GUI versions.
- `cameboy.bat gui <rom>` launches GUI; window stays open; close with Esc or X.

GUI layout
----------
- Background scaled to window height (left side console area).
- LCD drawn over background using percentages of console area:
  - x1 = 24% width, x2 = 76% width
  - y1 = 21% height, y2 = 68% height
- Two right panels/logs are disabled in GUI_BG_ONLY mode; LCD always renders on top.

Known caveats
-------------
- Do not embed resources for now; use file-based loading.
- If Access Denied on build, script kills running `cameboy*.exe` before link.
## CameBoy – Guide Agent (Vue d'ensemble, commandes, état)

### 1) Projet en bref
- **CameBoy**: émulateur Game Boy (DMG) en C99, conforme Pan Docs, objectifs Blargg/Mooneye.
- **Langage/OS**: C99 portable; scripts Windows (`.bat`) et Make/SH.
- **Réfs**: `docs/pandocs/` (copie locale Pan Docs).

### 2) Architecture (src/)
- `common.h`: types, constantes (IO regs, IE/IF, flags), utilitaires.
- `cpu.h/.c`: LR35902 (fetch/decode/execute), EI delay, HALT bug, tables `cpu_tables*.c`.
- `mmu.h/.c`: mapping mémoire, MBC (placeholder), IO (Timer/APU), ROM loader.
- `ppu.h/.c`: modes OAM/Transfer/HBlank/VBlank, registres LCD/STAT, rendu BG simple.
- `timer.h/.c`: DIV/TIMA/TMA/TAC, overflow → IRQ Timer.
- `joypad.h/.c`: P1 (sélection lignes), lecture boutons/directions.
- `interrupt.h/.c`: gestion IE/IF/priorités, service routines.
- `emulator_simple.c`: boucle simple (CPU/timer/PPU/APU/joypad/interrupts), chargement ROM.

### 3) Scripts & commandes
Windows (recommandé):
```cmd
:: Build + tests (par défaut)
cameboy.bat

:: Build seul
cameboy.bat build

:: Tests (recompile tests, exécute, génère logs)
cameboy.bat test

:: Run avec ROM (détecte quelques ROMs tests sinon préciser chemin)
cameboy.bat run path\to\rom.gb

:: Clean (build/ et logs/*.log)
cameboy.bat clean
```
Make/SH (Linux/macOS) – si utile:
```bash
make            # build
make test       # tests unitaires
./build.sh      # build
./build.sh test # tests
```
Tests unitaires dédiés (binaire unique):
```cmd
build\bin\test_cpu.exe
build\bin\test_mmu.exe
build\bin\test_ppu.exe
build\bin\test_timer.exe
build\bin\test_interrupt.exe
build\bin\test_joypad.exe
```

### 4) Logs utiles
- `logs/build.log`: compilation (horodaté).
- `logs/test_build.log`: compil des tests (OK/FAIL + timestamps).
- `logs/test_results.log`: résultats d'exécution tests.
- `logs/emulator.log`: sortie runtime de l’émulateur (`run`).

### 5) État actuel (à jour)
- ✅ CPU: PASS (tests + `test_cpu_flags`)
- ✅ MMU: PASS
- ✅ Interrupt: PASS
- ✅ Timer: PASS
- ✅ PPU: PASS (tests modes simples OK)
- ✅ Joypad: PASS (lecture P1 et IRQ Joypad; `test_joypad_irq`)

### 6) Correctifs en cours/proposés
- CPU: DAA exhaustif, JR/JP/CALL/RET conditionnels (cycles), HALT bug.
- PPU: tests timing/STAT plus stricts (456 cycles/ligne, LY/LYC, IRQ STAT).
- Joypad: scénarios HALT/STOP et réveil par IRQ Joypad.

### 7) Standards et priorités
- C99 strict; erreurs explicites; commentaires métier en français; noms en anglais.
- Suivre Pan Docs (docs/pandocs/). Priorité: précision timing CPU/PPU/Timers/Interrupts.
- Ordre de dev recommandé: MMU → CPU → Timers → PPU → MBC → APU.

### 8) Workflow rapide pour contribuer
1. `cameboy.bat clean` (optionnel) → `cameboy.bat build` → `cameboy.bat test`.
2. Corriger un module à la fois; relancer le test ciblé via `.bat test` (recompile) ou binaire direct.
3. Vérifier `logs/test_results.log` avant commit.

### 9) Notes Pan Docs (rappels piégeux)
- EI prend effet après l’instruction suivante.
- HALT bug: IME=0 et `[IE]&[IF]!=0` → PC n’incrémente pas, double lecture probable.
- PPU: 456 cycles/ligne (OAM≈80→XFER≈172→HBLANK), VBlank lignes 144–153.
- OAM DMA: copie 160 octets (non implémenté ici).



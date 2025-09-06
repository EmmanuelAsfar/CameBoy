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

### 5) État actuel (06/09/2025)
- ✅ CPU: PASS (21/21 dans `test_cpu`) – EI delay ok, HALT bug rudimentaire.
- ✅ MMU: PASS (7/7) – RAM initialisée 0xFF, IO de base ok.
- ✅ Interrupt: PASS (8/8).
- ❌ PPU: FAIL (modes – transitions/stat) dans `test_ppu` (Modes).
- ❌ Timer: FAIL (Overflow – rechargement/IRQ) dans `test_timer` (Overflow).
- ❌ Joypad: FAIL (Buttons – lecture bits bas avec A/B/START) dans `test_joypad`.

### 6) Correctifs en cours/proposés
- PPU
  - Mettre à jour `STAT` bits 0–2 au moment des changements de mode; resynchroniser `mode_cycles/line_cycles` à chaque transition; VBlank IRQ à LY=144.
- Timer
  - Overflow: rechargement TIMA depuis TMA et déclenchement IRQ; ajuster timing (reload immédiat attendu par test unitaire).
- Joypad
  - Alignement lecture des 4 bits bas selon sélection P15/P14; comportement START/SELECT pour satisfaire l’attendu de `test_joypad`.

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
- PPU: 456 cycles/ligne, VBlank lignes 144–153, transitions OAM(80)→XFER(172)→HBLANK.
- OAM DMA: copie 160 octets (non implémenté ici).



Agent notes - CameBoy
=====================

Agent Operating Guide
---------------------
This project uses a spec-first workflow aligned with Pan Docs. When changing code, documentation and tests must be kept in sync. This section codifies how the agent works and must be updated whenever new working directives are provided.

Principles
- Spec-first: read the relevant Pan Docs section before any change and link it from our specs.
- Keep docs in sync: update our component spec Markdown files alongside code and tests.
- Plain-language code comments: explain concepts without emulator jargon; if a technical term is used, define it inline.
- Tests-first mindset: add/update unit tests and ROM tests when behavior changes; do not rely on manual testing only.
- Windows-first scripts: build, test and ROM flows use the `.bat` scripts (do not bypass them in docs/instructions).
- Minimal, focused changes: keep interfaces stable; avoid unrelated edits.
- Traceability: reference files touched and rationale in PR descriptions or commit messages.

Standard Structure For Component Spec Docs
Each spec Markdown under `docs/specs/` must follow this structure:
1) Component Logic (from Pan Docs, simplified): describe the behavior in clear, non-expert terms; include small diagrams; add external links to the precise Pan Docs pages.
2) Implementation Choices: how the spec is implemented here, trade-offs and internal data flows; include diagrams.
3) Status & TODOs: what is done, what is missing or to improve.
4) Unit Tests: list of our unit tests that cover this component.
5) Test ROMs: list of our ROM tests that exercise this component.

Workflow (Per Change)
1) Before coding
   - Read Pan Docs for the topic; note edge cases/timings.
   - Review/update the corresponding spec file under `docs/specs/` (ensure it follows the structure above).
   - Define or update tests (unit + ROM) that will validate the change.
2) Implementation
   - Implement minimal, targeted changes; keep code commented in plain language.
   - Keep interfaces consistent with headers; verify all required components are wired (CPU, MMU, PPU, Timer, Interrupts, Joypad, APU).
   - If any build script changes are required, modify the `.bat` scripts and update `docs/scripts.md` (and `docs/usage.md` if needed).
3) Validation
   - Run `cameboy.bat test`; check `logs\test_results.log` until green.
   - Run ROM tests via `cameboy.bat testrom` and review outputs under `logs\rom\<romname>`.
   - For GUI checks, use `cameboy.bat gui <rom>`; for console frame dumps, use the CLI options provided by the console emulator.
4) Documentation & Agent Notes
   - Ensure the relevant `docs/specs/*.md` are updated (logic, implementation, status, unit tests, ROM tests).
   - Update the "Current Status" section after each step: reflect what is done and what remains (in this file and in each component spec's Status & TODOs).
   - If new working directives were introduced, update this Agent Operating Guide.

Testing & Logging Expectations
- Unit tests produce aggregate results in `logs\test_results.log` and per-binary stdout.
- ROM tests write runtime logs, serial output and optional frame dumps under `logs\rom/<romname>/`.
- Prefer deterministic tests; where timing-sensitive, document tolerances in the spec.

When Build Scripts Change
- Update `docs/scripts.md` and `docs/usage.md` to reflect new commands, parameters or outputs.
- Keep `.bat` the source of truth for Windows flows; ensure Make/SH remain optional mirrors only.

Update Policy
- Every time new working rules are provided by maintainers, amend this section to keep future contributors and agents aligned.

Policy-to-Tasks Rule
--------------------
After completing work (code, tests, or docs), always update `STATUS.md` to reflect the current state (what works, what fails, and next actions). This keeps the project’s live status accurate for everyone.

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
- Character encoding (French accents): use UTF-8 for source, docs and logs. On Windows consoles, prefer PowerShell (UTF-8) or run `chcp 65001` in `.bat` before emitting text; ensure editors save files as UTF-8. If mojibake appears in logs/GUI, verify file encodings and console code page.
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



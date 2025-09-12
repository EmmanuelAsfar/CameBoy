Project Status - CameBoy
========================

Overview
--------
This file tracks the live state of the project for all contributors: what passes, what fails, and what to do next. Update it after each meaningful change (code, tests, or docs).

Unit Test Status
----------------
- **Résultats détaillés :** [TEST_RESULTS.md](TEST_RESULTS.md) *(généré automatiquement)*
- **Logs complets :** [logs/test_results.log](logs/test_results.log)
- **Rapport ROM :** [logs/rom_test_results.log](logs/rom_test_results.log) *(tests ROM)*

**⚠️ IMPORTANT pour les agents IA :** Toujours relancer `cameboy.bat test` avant d'entreprendre des tâches pour avoir l'état le plus récent des tests.

ROM Test Status
---------------
- Serial ROMs: working (console prints / GUI serial panel OK)
- Visual ROMs: many render partially or blank; prioritize Timer/PPU/IRQ fixes
- Focus ROMs to run (and capture logs/frames):
  - `tests/rom/toto_counter.gb` (serial counter 0..10)
  - `tests/rom/visual_grid.gb`, `tests/rom/vertical_lines.gb`
  - `tests/rom/pass.gb`, `tests/rom/key_serial.gb`

Open Tasks (Next Actions)
-------------------------
Use checkboxes to track progress. Keep items concise and actionable.

- [ ] Unify IRQ handling across console/GUI
  - Files: `src/interrupt.c`, `src/emulator_simple.c`, `src/emulator_win32_gui.c`
  - Ensure single path: priority select, clear IF bit, exit HALT, push PC, jump to vector
  - Acceptance: interrupts fire identically in both binaries; unit tests remain green

- [ ] Wire MMU peripherals in console build
  - Files: `src/emulator_simple.c`
  - Add `mmu_set_joypad(&emu->mmu, &emu->joypad)` and `mmu_set_ppu(&emu->mmu, &emu->ppu)`; optional serial callback
  - Acceptance: P1 reads reflect joypad; VRAM/OAM access rules apply; serial writes captured

- [ ] Fix Timer overflow timing
  - File: `src/timer.c`
  - Pan Docs: TIMA increments on falling edge of selected DIV input; on overflow, TIMA = TMA and request Timer IRQ
  - Acceptance: `test_timer` passes; no regressions in other unit tests

- [ ] Fix PPU sprite priority rules
  - File: `src/ppu.c`
  - Ensure BG color 0 is transparent to sprites; respect OBJ priority bit and overdraw rules
  - Acceptance: target case in `test_ppu.c:918` passes; no regressions in other PPU tests

- [ ] Fix `toto_counter.gb` generation logic
  - File: `tests/rom/source/gen_toto_counter.c`
  - Condition should halt only when counter >= 11 without halting early
  - Acceptance: ROM prints TOTO: 0..10 via serial then halts; recorded under `logs/rom/toto_counter/`

- [ ] Add short IRQ/timing trace for first 1–2k cycles (optional)
  - File: `src/emulator_simple.c`
  - CLI flag to log PC, IF, IE, IME, mode; useful for early-boot triage

 - [ ] ROM Test Framework Overhaul (spec-first)
  - Goal: migrate to ROM-based assessment per component; integrate GBDK/RGBDS; unified logs + Markdown report.
  - Design/Specs:
    - Update `docs/testing.md` (formats `.expect` structuré: `TEST.<n>.SERIAL`, `SERIAL_SEQ`, `SERIAL_COUNT`, `TEST.<n>.CYCLES`, `CONFIG.*`; oracle visuel optionnel `EXPECTED_PPM`).
    - Update `docs/roms.md` (toolchains, layout `tests/rom/source/{gbdk,rgbds}/...`, conventions de sortie série: SUITE/TEST START|PASS|FAIL).
    - Cross-link Pan Docs sections (Serial, Timers, STAT/LCDC, Interrupts) and per-component specs `docs/specs/*`.
  - Toolchains & Build:
    - Add `tests/rom/source/build_roms_gbdk.bat` (detect `tools\\gbdk\\bin\\lcc.exe`, build all `gbdk/*`),
      and `tests/rom/source/build_roms_rgbds.bat` (detect `tools\\rgbds\\bin\\{rgbasm,rgblink,rgbfix}`) with repo-local PATH.
    - Update `tests/rom/run_rom_tests.bat` to call GBDK/RGBDS builders before C-host generators (current fallback kept).
  - Per-component ROM suites (squelettes, tests vides au départ):
    - CPU: `cpu.gb` (flags/arith/EI-HALT); MMU/Memory: `memory.gb`; PPU: `ppu.gb`; Timer: `timer.gb`;
      Interrupts: `interrupt.gb`; Joypad: `joypad.gb`; Serial: `serial.gb`; DMA: `dma.gb`.
    - Create sources under `tests/rom/source/gbdk/<component>/` (C) and `tests/rom/source/rgbds/<component>/` (ASM) with minimal main + série “TEST <id> FAIL (todo)”.
    - Add matching `tests/rom/config/<component>.expect` using structured sub-tests with descriptions and expected tokens (initially failing).
  - Assessment Enhancements:
    - Extend parser in `tests/rom/run_rom_tests.bat` to support `SERIAL_SEQ` (ordered tokens), `SERIAL_COUNT` (occurrences), `TEST.<n>.CYCLES` overrides, and `CONFIG.ONLY` filter.
    - Optional oracle visuel: compare `logs\\rom\\<rom>.ppm` avec `tests\\rom\\expected\\<rom>.ppm` (identité binaire d’abord, tolérance plus tard).
  - Reporting & Scripts:
    - Unify output with unit-test style: append per-ROM details to `logs\\rom_test_results.log` and generate `ROM_RESULTS.md` (date, totals, per-ROM breakdown, excerpts), mirroring `TEST_RESULTS.md`.
    - Update `cameboy.bat test` to call ROM tests as primary path (unit tests deprecated) and include Markdown report path in summary.
    - Update `docs/scripts.md` and `docs/usage.md` with new flows/flags.
  - Migration:
    - Deprecate existing unit tests (skip build/invoke); keep sources for reference during transition.
    - Track progress component-by-component in this file; initial runs expected to fail until components converge.
  - Acceptance (phase 1):
    - Builders detected/executed; ROMs compiled; runner produces logs (`*.log`, `*_serial.txt`, optional `*.ppm`) and `ROM_RESULTS.md` with totals; all configured suites listed (even if failing).

  - Subtasks:
    - [ ] Update `docs/testing.md` with structured `.expect` spec (SERIAL, SERIAL_SEQ, SERIAL_COUNT, TEST.<n>.CYCLES, CONFIG.*, EXPECTED_PPM).
    - [ ] Update `docs/roms.md` with toolchains integration and repo layout for `gbdk/` and `rgbds/` ROMs.
    - [ ] Add `tests/rom/source/build_roms_gbdk.bat` (detect `tools\\gbdk\\bin\\lcc.exe`).
    - [ ] Add `tests/rom/source/build_roms_rgbds.bat` (detect `tools\\rgbds\\bin\\rgbasm/rgblink/rgbfix`).
    - [ ] Update `tests/rom/run_rom_tests.bat` to call GBDK/RGBDS builders before C-host generators.
    - [ ] Extend `run_rom_tests.bat` parser to handle `SERIAL_SEQ`, `SERIAL_COUNT`, `TEST.<n>.CYCLES`, `CONFIG.ONLY`.
    - [ ] Generate `ROM_RESULTS.md` (Markdown summary mirroring `TEST_RESULTS.md`).
    - [ ] Update `cameboy.bat test` to run ROM tests primarily and print path to `ROM_RESULTS.md`.
    - [ ] Create skeleton ROM suites: `tests/rom/source/gbdk/{cpu,memory,ppu,timer,interrupt,joypad,serial,dma}/` (main + TODO tests).
    - [ ] Create skeleton ASM suites under `tests/rom/source/rgbds/` for selected components (optional initially).
    - [ ] Add `tests/rom/config/{cpu,memory,ppu,timer,interrupt,joypad,serial,dma}.expect` with subtests and descriptions.
    - [ ] Add `tests/rom/expected/` and wire optional PPM oracle (strict equality phase 1).
    - [ ] Deprecate unit test execution in `cameboy.bat test` (do not delete sources yet).

How To Validate
---------------
- Build and run unit tests (Windows):
  - `cameboy.bat test`
  - Inspect `logs\test_results.log`
- Run a subset of ROM tests:
  - `cameboy.bat testrom`
  - Inspect `logs\rom\<romname>\*.log` and frame dumps (if any)
- GUI quick check:
  - `cameboy.bat gui tests\rom\visual_grid.gb`

References
----------
- Pan Docs: https://gbdev.io/pandocs/
- Project specs: `docs/specs/` (each file follows the mandated structure)
- Scripts: `docs/scripts.md` (always use `.bat` on Windows)

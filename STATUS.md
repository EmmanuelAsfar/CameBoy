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


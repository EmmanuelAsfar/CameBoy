# Test ROM layout

- `tests/rom/` contains generated `.gb` files.
- `tests/rom/source/` contains the generator sources (e.g., `generate_roms.c`).
- The harness `tests/rom/run_rom_tests.bat` will compile the generator from `tests/rom/source/` if present, otherwise falls back to `tests/rom/generate_roms.c`.

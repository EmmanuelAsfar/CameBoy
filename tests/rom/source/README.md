# Test ROM layout

- `tests/rom/` contains the built `.gb` test ROMs.
- `tests/rom/source/` contains sources for ROMs built via toolchains:
  - `gbdk/<component>/main.c` built with `tools\gbdk\bin\lcc.exe`
  - `rgbds/<component>/main.asm` built with `tools\rgbds\bin\{rgbasm,rgblink,rgbfix}`
- The harness `tests/rom/run_rom_tests.bat` calls `tests/rom/source/build_roms_gbdk.bat` and `build_roms_rgbds.bat` when toolchains are present.
- Legacy per-ROM generators (`gen_*.c`) and the monolithic `generate_roms.c` are no longer part of the default flow.

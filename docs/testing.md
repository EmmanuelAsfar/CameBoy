Retour à l'index: [docs/README.md](./README.md) | [Architecture](./architecture.md) | [Utilisation](./usage.md) | [Scripts](./scripts.md) | [Glossaire](./glossaire.md)

### Stratégie de tests

Ce document décrit les tests unitaires, les tests ROM (série/visuels) et l'organisation des logs.

### Types de tests

- Tests unitaires C: binaire par composant (CPU, MMU, PPU, Timer, Interrupt, Joypad).
- Tests ROM (dossier `tests/rom/`): petites ROMs affichant texte/sorties série ou visuels (PPM/logs).
- Conformité Blargg/Mooneye: ROMs de référence à exécuter manuellement/plus tard dans CI.

### Lancer les tests unitaires (Windows)

```cmd
cameboy.bat test
type logs\test_results.log | more
```

Le script compile chaque test en `build\bin\test_*.exe`, les exécute et cumule les résultats.

Sortie attendue (extrait):

```text
========================================
CameBoy Unit Tests - 06/09/2025 12:34:56.789
========================================
Running test_cpu...
SUCCES test_cpu PASSED
...
Total: 6/6 tests passed
```

Logs associAs:
- `logs\test_build.log`: compilation des tests (OK/FAIL + timestamps)
- `logs\test_results.log`: exAcution agrAgAe

### ExAcuter un test unitaire prAcis

```cmd
build\bin\test_cpu.exe
```

ou recompiler unitairement via Makefile (Linux/macOS possible):

```bash
make test
```

### Tests ROM (sArie/visuels)

Le batch `tests\rom\run_rom_tests.bat` lance un sous-ensemble de ROMs de dAmonstration et stocke:
- `.log`: log daexAcution
- `.serial.txt`: sortie sArie de la ROM (via 0xFF01)
- `.ppm`: capture visuelle (si activAe par la ROM ou laAmulateur)

Exemple de fichiers gAnArAs (dans `logs/rom/`):
- `pass.log`, `pass_serial.txt`, `pass.ppm`
- `visual_grid.log`, `visual_grid.ppm`

### CritAres de conformitA (cible)

- Blargg CPU: toutes les suites `cpu_instrs`, `instr_timing`, `halt_bug`.
- Mooneye: `acceptance` (timings PPU/Timer/Interrupts), `oam_bug`.

Note: certaines zones sont en cours de stabilisation (voir `README_AGENT.md`).

### DAbug et triage

- Compiler en debug: ajouter `-g` (dAjA  prAsent) et lancer sous `gdb` (Linux) ou `lldb`.
- Inspecter `logs\emulator.log` et `logs\rom\<rom>_serial.txt` pour comprendre les sAquences.
- Pour la GUI, stdout/stderr sont captAs en direct dans le panneau Logs, et le Port SArie dans le panneau dAdiA.



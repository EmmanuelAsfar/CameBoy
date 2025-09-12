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

### Tests ROM (série/visuels)

Le batch `tests\rom\run_rom_tests.bat` exécute des ROMs de test et stocke:
- `.log`: log d'exécution
- `.serial.txt`: sortie série de la ROM (via 0xFF01)
- `.ppm`: capture visuelle (si activée)

Exemples de fichiers générés (dans `logs/rom/`):
- `pass.log`, `pass_serial.txt`, `pass.ppm`
- `visual_grid.log`, `visual_grid.ppm`

### Config d’attentes par ROM

Chaque ROM définit ses attentes via un fichier `tests/rom/config/<rom>.expect`:

- Format simple OU structuré.
  - Format simple: une ligne par jeton série attendu. PASS si tous les jetons sont trouvés dans `*_serial.txt`. Lignes vides ou débutant par `#`/`;` ignorées.
  - Format structuré: clés `TEST.<n>.*` pour décrire des sous‑tests (voir section framework étendu ci‑dessous).
- Clés globales dans `.expect`:
  - `CONFIG.CYCLES=<nombre>`: cycles max CPU (anti‑boucle).
  - `CONFIG.SKIP=1`: ignorer la ROM.

Note: le format `.cfg` legacy (CYCLES/SKIP) a été supprimé. Utilisez `CONFIG.*` dans `.expect`.

Exemples fournis:
- `timer_basic.expect` (structuré) avec un sous‑test qui vérifie `PASS` et des commentaires dédiés.
- `pass.expect`, `joypad_default.expect`, `cpu_zc_add.expect` (format simple) avec le jeton `PASS`.
- `ppu_ly_vblank.expect` (format simple) avec `PPU:VBL` (test série VBlank via LY==144).
- `visual_grid.cfg` avec `SKIP=1` (ROM purement visuelle).

### Critères de conformité (cible)

- Blargg CPU: toutes les suites `cpu_instrs`, `instr_timing`, `halt_bug`.
- Mooneye: `acceptance` (timings PPU/Timer/Interrupts), `oam_bug`.

Note: certaines zones sont en cours de stabilisation (voir `README_AGENT.md`).

### DAbug et triage

- Compiler en debug: ajouter `-g` (dAjA  prAsent) et lancer sous `gdb` (Linux) ou `lldb`.
- Inspecter `logs\emulator.log` et `logs\rom\<rom>_serial.txt` pour comprendre les sAquences.
- Pour la GUI, stdout/stderr sont captAs en direct dans le panneau Logs, et le Port SArie dans le panneau dAdiA.



### Tests ROM (série/visuels) — framework étendu

Le batch `tests\rom\run_rom_tests.bat` construit les ROMs (GBDK, RGBDS, générateurs C), lance l'exécutable headless et stocke:
- `.log`: log d'exécution
- `.serial.txt`: sortie série (via 0xFF01)
- `.ppm`: capture visuelle (si demandée)

Exemples (dans `logs/rom/`): `pass.log`, `pass_serial.txt`, `pass.ppm`, `visual_grid.ppm`.

#### Config d'attentes `.expect`

Format simple ou structuré:
- Simple: une ligne par jeton série attendu. PASS si tous les jetons sont trouvés. Lignes vides, `#`/`;` ignorées.
- Structuré: clés `TEST.<n>.*` par sous-test:
  - `TEST.<n>.SERIAL` (ou `SERIAL_EXPECTED`): jeton attendu.
  - `TEST.<n>.SERIAL_SEQ`: liste ordonnée de jetons séparés par `;` (évaluée en ordre de lignes; simplifiée).
  - `TEST.<n>.SERIAL_COUNT`: contrainte d’occurrence (`TOKEN=3`).
  - `TEST.<n>.STATUS`: permet de marquer un cas comme `NOT_IMPLEMENTED` (alias `NI`) — ni PASS ni FAIL, compté séparément.
  - `TEST.<n>.DESC`: description unique du sous-test (utilisée quel que soit le statut: PASS/FAIL/NI).
  - `TEST.<n>.CYCLES`: cycles (réservé, niveau ROM).
  - `TEST.<n>.ENABLED` (0/1).
  - Compatibilité: les anciens champs `COMMENT_PASS` / `COMMENT_FAIL` / `COMMENT_NI` sont lus mais mappés vers `DESC` si `DESC` n’est pas fourni.

Clés globales dans `.expect` (surchargent `.cfg`):
- `CONFIG.CYCLES=<nombre>`
- `CONFIG.SKIP=1`
- `CONFIG.ONLY=id1,id2`
- Option visuelle: `EXPECTED_PPM=tests\rom\expected\<rom>.ppm`

#### Génération des ROMs

- GBDK: `tests\rom\source\build_roms_gbdk.bat` (détecte `tools\gbdk\bin\lcc.exe`).
- RGBDS: `tests\rom\source\build_roms_rgbds.bat` (détecte `tools\rgbds\bin\rgbasm|rgblink|rgbfix`).

Le runner appelle automatiquement ces builders si disponibles. Les anciens générateurs C (monolithique `generate_roms.c` et `gen_*.c`) ne font plus partie du flux par défaut.

#### Rapport Markdown

Un résumé `ROM_RESULTS.md` est généré à la racine, avec totaux, nombre de ROMs « non implémentées », et un extrait de `logs\rom_test_results.log`.

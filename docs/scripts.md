Retour A  laindex: [docs/README.md](./README.md) A [Architecture](./architecture.md) A [Utilisation](./usage.md) A [Tests](./testing.md) A [Glossaire](./glossaire.md)

### Scripts et automatisation

Ce document dAcrit les scripts Windows fournis et leur usage, ainsi que les alternatives Make/SH.

### `cameboy.bat` (racine)

Commandes:
- `build`: compile la version console (`cameboy.exe`) et GUI (`cameboy_gui.exe`).
- `test`: compile et exAcute les tests unitaires; agrAge dans `logs\test_results.log`.
- `run [rom]`: lance laAmulateur console; Acrit `logs\emulator.log`.
- `gui [rom]`: lance laAmulateur GUI; Acrit `logs\emulator_gui.log` et `logs/rom/<rom>/...`.
- `testrom`: exAcute `tests\rom\run_rom_tests.bat`.
- `clean`: supprime `build/` et les `.log` sous `logs/`.

CaractAristiques:
- VArifie `gcc`, tue les exe en cours (`taskkill`) avant link pour Aviter A AccAs refusA A.
- Copie `resources/` vers `build/resources/`.
- ParamAtres de compilation principaux: `-std=c99 -Wall -Wextra -O2 -g`.

### Scripts utilitaires (`user/`)

- `run_unit_tests.bat`: lance rapidement la sAquence de tests unitaires (Aquivalent A  `cameboy.bat test`).
- `run_gui_emulator.bat`: lance la GUI sur une ROM par dAfaut (configurable dans le script).
- `run_gui_default_rom.bat`: variante pour la ROM par dAfaut `tests\rom\visual_grid.gb` si prAsente.
- `run_gui_rom_picker.bat`: lance la GUI en demandant le chemin daune ROM.
- `clean_logs.bat`: purge les logs (`logs/*.log`).

Ces scripts enveloppent `cameboy.bat` et servent de raccourcis.

### Makefile / Shell (optionnel)

- `make`: cible A all A gAnAre les binaires console et GUI.
- `make test`: compile/exAcute les tests unitaires et agrAge les rAsultats.
- `build.sh`: script shell cross-platform minimal (build/test/clean).

Conseil: sous Windows, utiliser `cameboy.bat` en prioritA, qui gAre mieux les particularitAs locales (chemins, ressources, logs).



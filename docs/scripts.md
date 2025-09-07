Retour à l’index: [docs/README.md](./README.md) · [Architecture](./architecture.md) · [Utilisation](./usage.md) · [Tests](./testing.md) · [Glossaire](./glossaire.md)

### Scripts et automatisation

Ce document décrit les scripts Windows fournis et leur usage, ainsi que les alternatives Make/SH.

### `cameboy.bat` (racine)

Commandes:
- `build`: compile la version console (`cameboy.exe`) et GUI (`cameboy_gui.exe`).
- `test`: compile et exécute les tests unitaires; agrège dans `logs\test_results.log`.
- `run [rom]`: lance l’émulateur console; écrit `logs\emulator.log`.
- `gui [rom]`: lance l’émulateur GUI; écrit `logs\emulator_gui.log` et `logs/rom/<rom>/...`.
- `testrom`: exécute `tests\rom\run_rom_tests.bat`.
- `clean`: supprime `build/` et les `.log` sous `logs/`.

Caractéristiques:
- Vérifie `gcc`, tue les exe en cours (`taskkill`) avant link pour éviter « Accès refusé ».
- Copie `resources/` vers `build/resources/`.
- Paramètres de compilation principaux: `-std=c99 -Wall -Wextra -O2 -g`.

### Scripts utilitaires (`user/`)

- `run_unit_tests.bat`: lance rapidement la séquence de tests unitaires (équivalent à `cameboy.bat test`).
- `run_gui_emulator.bat`: lance la GUI sur une ROM par défaut (configurable dans le script).
- `run_gui_default_rom.bat`: variante pour la ROM par défaut `tests\rom\visual_grid.gb` si présente.
- `run_gui_rom_picker.bat`: lance la GUI en demandant le chemin d’une ROM.
- `clean_logs.bat`: purge les logs (`logs/*.log`).

Ces scripts enveloppent `cameboy.bat` et servent de raccourcis.

### Makefile / Shell (optionnel)

- `make`: cible « all » génère les binaires console et GUI.
- `make test`: compile/exécute les tests unitaires et agrège les résultats.
- `build.sh`: script shell cross-platform minimal (build/test/clean).

Conseil: sous Windows, utiliser `cameboy.bat` en priorité, qui gère mieux les particularités locales (chemins, ressources, logs).



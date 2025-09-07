Retour à l’index: [Index docs](./README.md) · [Architecture](./architecture.md) · [Tests](./testing.md) · [Scripts](./scripts.md) · [Glossaire](./glossaire.md)

### Utilisation de CameBoy

Ce guide explique comment compiler, tester et exécuter l'émulateur CameBoy (versions console et GUI) sur Windows, et fournit des repères pour Linux/macOS.

### Prérequis

- Windows: `gcc` (MinGW/TDM-GCC). Vérifier avec `where gcc`.
- Optionnel Linux/macOS: `make`, `gcc`.

### Structure utile

```text
build/            # artefacts (généré)
  bin/            # exécutables
logs/             # logs (généré)
resources/        # assets GUI copiés en build/resources/
src/              # code C
tests/            # tests unitaires et ROM
```

### Compiler (Windows)

Deux options équivalentes:

1) Script principal:

```cmd
cameboy.bat build
```

2) Makefile:

```cmd
make
```

Résultats attendus:
- `build\bin\cameboy.exe` (version console)
- `build\bin\cameboy_gui.exe` (version GUI)

Les logs de compilation sont dans `build\build.log` (script) et `logs\compile.log` (Makefile).

### Exécuter (version console)

```cmd
cameboy.bat run path\to\rom.gb
```

Sans argument ROM, le script tente quelques chemins tests (`tests\rom\...` ou Blargg). La sortie est loggée dans `logs\emulator.log`.

Exemple direct (sans script):

```cmd
build\bin\cameboy.exe path\to\rom.gb
```

### Exécuter (version GUI)

```cmd
cameboy.bat gui path\to\rom.gb
```

Contrôles par défaut:
- Flèches: D-pad
- Z: A
- X: B
- Entrée: Start
- Maj droite: Select
- Échap: Quitter

La GUI affiche:
- Un écran Game Boy (framebuffer du PPU)
- Un panneau Série (flux 0xFF01)
- Un panneau Logs (stdout/stderr redirigés)

Les logs ROM GUI sont sous `logs/rom/<romname>/`.

### Tests rapides

```cmd
cameboy.bat test
type logs\test_results.log | more
```

### Nettoyage

```cmd
cameboy.bat clean
```

### Linux/macOS (optionnel)

```bash
make          # build
make test     # tests
./build.sh    # build via script
```

### Dépannage

- « gcc not found »: installer MinGW/TDM-GCC et ajouter au PATH.
- Accès refusé au link: fermer l’exécutable; le script tente `taskkill` automatique.
- GUI sans image: vérifier `resources/gameboy_bg.bmp` copié en `build/resources/`.



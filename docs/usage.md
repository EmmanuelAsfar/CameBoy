Retour A  laindex: [Index docs](./README.md) A [Architecture](./architecture.md) A [Tests](./testing.md) A [Scripts](./scripts.md) A [Glossaire](./glossaire.md)

### Utilisation de CameBoy

Ce guide explique comment compiler, tester et exAcuter l'Amulateur CameBoy (versions console et GUI) sur Windows, et fournit des repAres pour Linux/macOS.

### PrArequis

- Windows: `gcc` (MinGW/TDM-GCC). VArifier avec `where gcc`.
- Optionnel Linux/macOS: `make`, `gcc`.

### Structure utile

```text
build/            # artefacts (gAnArA)
  bin/            # exAcutables
logs/             # logs (gAnArA)
resources/        # assets GUI copiAs en build/resources/
src/              # code C
tests/            # tests unitaires et ROM
```

### Compiler (Windows)

Deux options Aquivalentes:

1) Script principal:

```cmd
cameboy.bat build
```

2) Makefile:

```cmd
make
```

RAsultats attendus:
- `build\bin\cameboy.exe` (version console)
- `build\bin\cameboy_gui.exe` (version GUI)

Les logs de compilation sont dans `build\build.log` (script) et `logs\compile.log` (Makefile).

### ExAcuter (version console)

```cmd
cameboy.bat run path\to\rom.gb
```

Sans argument ROM, le script tente quelques chemins tests (`tests\rom\...` ou Blargg). La sortie est loggAe dans `logs\emulator.log`.

Exemple direct (sans script):

```cmd
build\bin\cameboy.exe path\to\rom.gb
```

### ExAcuter (version GUI)

```cmd
cameboy.bat gui path\to\rom.gb
```

ContrAles par dAfaut:
- FlAches: D-pad
- Z: A
- X: B
- EntrAe: Start
- Maj droite: Select
- Achap: Quitter

La GUI affiche:
- Un Acran Game Boy (framebuffer du PPU)
- Un panneau SArie (flux 0xFF01)
- Un panneau Logs (stdout/stderr redirigAs)

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

### DApannage

- A gcc not found A: installer MinGW/TDM-GCC et ajouter au PATH.
- AccAs refusA au link: fermer laexAcutable; le script tente `taskkill` automatique.
- GUI sans image: vArifier `resources/gameboy_bg.bmp` copiA en `build/resources/`.



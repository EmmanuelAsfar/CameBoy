# Toolchains pour construire des ROMs Game Boy

Ce guide explique comment installer et configurer les toolchains recommandées pour compiler des ROMs `.gb` à partir des exemples fournis dans `examples/`.

## Outils à installer

- RGBDS (assembleur/linker/fixer)
  - Site: `https://rgbds.gbdev.io/`
  - Téléchargements: `https://github.com/gbdev/rgbds/releases`
  - Binaries: `rgbasm`, `rgblink`, `rgbfix`, `rgbgfx`
- GBDK-2020 (SDK C basé sur SDCC)
  - Site: `https://gbdk-2020.github.io/`
  - Téléchargements: `https://github.com/gbdk-2020/gbdk-2020/releases`
  - Binaire principal: `lcc`

Référence matérielle: [Pan Docs](https://gbdev.io/pandocs)

## Installation (Windows)

Deux approches, au choix:

1) PATH (recommandé)
- Ajoutez les binaires RGBDS (`rgbasm.exe`, `rgblink.exe`, `rgbfix.exe`) à votre PATH
- Ajoutez `.../gbdk/bin` (contenant `lcc.exe`) à votre PATH

2) Sans toucher au PATH (recommandé dans ce repo)
- Placez les outils dans le repo:
  - `tools/rgbds/bin/` contenant `rgbasm.exe`, `rgblink.exe`, `rgbfix.exe`
  - `tools/gbdk/bin/` contenant `lcc.exe`
- Les scripts d'exemples détectent d'abord ces chemins repo-locaux
- Optionnel: variables d'env
  - `RGBDS_HOME` (ex: `C:\tools\rgbds`)
  - `GBDK_HOME` (ex: `C:\tools\gbdk`)
  - Utilisées si les dossiers `tools/` ne sont pas présents

## Vérification rapide

- RGBDS:
  - `rgbasm -V`
  - `rgblink -V`
  - `rgbfix -V`
- GBDK-2020:
  - `lcc -v`

## Construire les exemples

- Builder tous les exemples (si outils installés):
```cmd
cameboy.bat examples
```

- Résultats:
  - `examples/rgbds-hello-serial/build/rgbds_hello.gb`
  - `examples/gbdk-hello-printf/build/gbdk_hello.gb`
  - `examples/rgbds-hello-vblank/build/rgbds_vblank.gb`

- Lancer dans l'émulateur:
```cmd
build\bin\cameboy.exe examples\rgbds-hello-serial\build\rgbds_hello.gb
build\bin\cameboy_gui.exe examples\gbdk-hello-printf\build\gbdk_hello.gb
build\bin\cameboy.exe examples\rgbds-hello-vblank\build\rgbds_vblank.gb
```

## Notes utiles

- `rgbfix` remplit correctement l'en-tête (logo, checksums) pour compatibilité.
- La GUI CameBoy affiche les logs série: écrivez sur `SB`/`SC` pour tracer.
- Pour des ROMs > 32 KiB, choisissez un MBC adapté (cf. [MBCs](https://gbdev.io/pandocs/MBCs.html)).

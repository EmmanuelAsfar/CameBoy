# ROMs Game Boy a" Format, lecture, et construction

Retour: [Index docs](./README.md) A [Architecture](./architecture.md)

## Quaest-ce quaune ROM ? (non-expert)
Une ROM est un fichier binaire (ex: `.gb`) contenant le programme du jeu. LaAmulateur lit ce fichier et laexAcute comme si caAtait une cartouche insArAe dans la console.

## Vue daensemble (expert)
- Fichier binaire mappA en mAmoire A  partir de 0x0000 (ROM0/ROMX)
- En-tAte cartouche A  0x0100a"0x014F (logo, titre, type, tailles, checksums)
- Aventuel contrAleur MBC pour banques ROM/RAM

```
graph LR
  A[ROM file .gb] -->|map| B[0x0000-0x3FFF ROM0]
  A -->|banking| C[0x4000-0x7FFF ROMX]
  A --> D[Header 0x0100-0x014F]
```

## En-tAte de cartouche (header)
Adresse 0x0100a"0x014F. Champs principaux:
- 0x0104a"0x0133: Logo Nintendo (doit matcher)
- 0x0134a"0x0143: Titre (16 bytes)
- 0x0147: Type de cartouche (dAtermine MBC)
- 0x0148: Taille ROM (code -> nb de banques)
- 0x0149: Taille RAM (code -> taille totale)
- 0x014D: Header checksum
- 0x014Ea"0x014F: Global checksum

RAf: [Pan Docs a" The Cartridge Header](https://gbdev.io/pandocs/The_Cartridge_Header.html)

## Lecture par laAmulateur

### Chargement
- Lecture du fichier `.gb` en mAmoire
- Parsing de laen-tAte pour identifier `cart_type`, `rom_size`, `ram_size`
- SAlection du MBC (MBC1/MBC3/MBC5a) si nAcessaire
- Initialisation des banques (ROM0/ROMX, RAM externe)

```
sequenceDiagram
  participant FS as Fichier ROM
  participant MMU
  participant MBC
  FS->>MMU: lire binaire
  MMU->>MMU: parser header (type/taille)
  MMU->>MBC: init selon cart_type
  MMU->>MMU: map ROM0/ROMX
```

### AccAs mAmoire
- 0x0000a"0x3FFF: ROM banc 0
- 0x4000a"0x7FFF: ROM banc sAlectionnA (MBC)
- Acritures dans 0x0000a"0x7FFF utilisAes par MBC pour changer de banc/activer RAM/RTC

RAf: [Pan Docs a" Memory Map](https://gbdev.io/pandocs/Memory_Map.html), [MBCs](https://gbdev.io/pandocs/MBCs.html)

## Construire des ROMs (homebrew)
Deux toolchains populaires:

### 1) RGBDS (assembleur)
- Outils: `rgbasm` (assemble), `rgblink` (link), `rgbfix` (header/checksum)
- Fichiers source: `.asm` + includes `.inc`

Exemple Makefile minimal:
```make
# make
rom.gb: main.o
	rgblink -o $@ $^
	rgbfix -v -p 0 $@

%.o: %.asm
	rgbasm -o $@ $<
```

Exemple `main.asm` minimal:
```asm
SECTION "Entry", ROM0[$0100]
  nop
  jp Start

SECTION "Code", ROM0
Start:
  ; votre code ici
  jr Start
```

- `rgbfix -p 0` fixe le padding et le header pour DMG.
- Docs: `https://rgbds.gbdev.io/`

### 2) GBDK (C)
- SDK en C pour Game Boy
- Compiler du C en ROM `.gb`

Exemple (ligne de commande):
```bash
lcc -o build/hello.gb src/hello.c
```

`hello.c` minimal:
```c
#include <gb/gb.h>
#include <stdio.h>

void main() {
  printf("Hello, GB!\n");
  while(1) wait_vbl_done();
}
```

- Docs: `https://gbdk-2020.github.io/`

## Bonnes pratiques
- Respecter laen-tAte (logo, tailles, checksums) pour compatibilitA maximale
- Choisir MBC adaptA (taille ROM/RAM)
- Aviter daAcrire dans 0x0000a"0x7FFF sauf pour commandes MBC
- Tester sur Amulateurs et si possible sur hardware

## ROMs de test (rApertoire du projet)
- `tests/rom/*.gb`: petites ROMs de debug/visuel/sArie
- `tests/blargg/`, `tests/mooneye/`: suites de conformitA

## RAfArences
- [Pan Docs a" The Cartridge Header](https://gbdev.io/pandocs/The_Cartridge_Header.html)
- [Pan Docs a" MBCs](https://gbdev.io/pandocs/MBCs.html)
- [RGBDS](https://rgbds.gbdev.io/) A [GBDK-2020](https://gbdk-2020.github.io/)

## Conception d'une ROM (guidelines)

Cette rubrique synthAtise les choix et patterns de conception pour des ROMs robustes et compatibles.

### 1) Choix MBC et tailles
- Si ROM a 32 KiB: cartouche ANo MBCA. Sinon choisir MBC1/MBC3/MBC5 selon besoins (banques ROM, RAM, RTC).
- DAfinir `ROM size` (0x0148) et `RAM size` (0x0149) cohArents avec laagencement mAmoire.
- RAf: [Pan Docs a" MBCs](https://gbdev.io/pandocs/MBCs.html)

### 2) Layout mAmoire et banques
- `ROM0` (0x0000a"0x3FFF): code commun (reset, ISR, routines bancables-stubs)
- `ROMX` (0x4000a"0x7FFF): code/data bancable(s)
- Si banques: centraliser les sAlecteurs MBC et une convention daID de banque.

### 3) DAmarrage et initialisation
- Point daentrAe A  0x0100 (aprAs le boot ROM). Ateindre les interruptions, initialiser SP, IO critiques.
- Ateindre le LCD (LCDC.7=0) avant modifications VRAM/OAM/PPU.
- Initialiser palettes, SCX/SCY, WY/WX, STAT/IE/IF selon besoins.
- RAf: [Pan Docs a" Power-Up Sequence](https://gbdev.io/pandocs/Power_Up_Sequence.html) A [LCD Control](https://gbdev.io/pandocs/LCDC.html)

### 4) Interruptions et timing
- VBlank: lieu privilAgiA pour mises A  jour VRAM/OAM et logique frame.
- STAT modes: respecter les fenAtres daaccAs VRAM/OAM (voir STAT timing).
- Timer: utiliser TIMA/TMA/TAC pour logique fixe en Hz dAsirAe.
- RAf: [Interrupts](https://gbdev.io/pandocs/Interrupts.html) A [LCD Status](https://gbdev.io/pandocs/STAT.html)

### 5) Pipeline assets (tuiles/sprites)
- Convertir PNG a' tuiles via `rgbgfx` (RGBDS) ou outils GBDK.
- Organiser les tilesets et maps par banque si nAcessaire, documenter les indices.
- RAf: [Tile Data](https://gbdev.io/pandocs/Tile_Data.html) A [Tile Maps](https://gbdev.io/pandocs/Tile_Maps.html)

### 6) Debug sArial et logs
- Acrire sur SB/SC (0xFF01/0xFF02) avec `SC=0x81` pour tracer des messages vers laAmulateur.
- Pratique pour tests automatisAs et vArification daAtat.
- RAf: [Serial](https://gbdev.io/pandocs/Serial_Data_Transfer.html)

### 7) CompatibilitA et header
- Utiliser `rgbfix` ou laoutil de la toolchain pour le header (logo/titre/checksums).
- VArifier que `cartridge type` matche le MBC rAel.
- RAf: [The Cartridge Header](https://gbdev.io/pandocs/The_Cartridge_Header.html)

### 8) Patterns de boucle jeu
- Boucle principale synchronisAe VBlank (`wait_vbl_done` en GBDK) ou ISR VBlank.
- Double-buffering software des commandes VRAM/OAM appliquAes A  chaque frame.

### 9) Tests et conformitA
- Blargg (CPU) et Mooneye (timing) A  intAgrer tAt.
- Ajouter des micro-ROMs pour chaque fonctionnalitA sensible (DMA, STAT, OAM, joypad, timer).

### 10) Check-list rapide
- Header correct, MBC cohArent, tailles alignAes
- Init SP/IO/PPU, LCD off avant VRAM, ISR configurAes
- AccAs VRAM/OAM uniquement dans les fenAtres autorisAes
- Logs sArial disponibles pour debug

RAf gAnArale: [Pan Docs](https://gbdev.io/pandocs) (synthAse officielle)
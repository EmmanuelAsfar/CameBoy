# ROMs Game Boy – Format, lecture, et construction

Retour: [Index docs](./README.md) · [Architecture](./architecture.md)

## Qu’est-ce qu’une ROM ? (non-expert)
Une ROM est un fichier binaire (ex: `.gb`) contenant le programme du jeu. L’émulateur lit ce fichier et l’exécute comme si c’était une cartouche insérée dans la console.

## Vue d’ensemble (expert)
- Fichier binaire mappé en mémoire à partir de 0x0000 (ROM0/ROMX)
- En-tête cartouche à 0x0100–0x014F (logo, titre, type, tailles, checksums)
- Éventuel contrôleur MBC pour banques ROM/RAM

```mermaid
graph LR
  A[ROM file .gb] -->|map| B[0x0000-0x3FFF ROM0]
  A -->|banking| C[0x4000-0x7FFF ROMX]
  A --> D[Header 0x0100-0x014F]
```

## En-tête de cartouche (header)
Adresse 0x0100–0x014F. Champs principaux:
- 0x0104–0x0133: Logo Nintendo (doit matcher)
- 0x0134–0x0143: Titre (16 bytes)
- 0x0147: Type de cartouche (détermine MBC)
- 0x0148: Taille ROM (code -> nb de banques)
- 0x0149: Taille RAM (code -> taille totale)
- 0x014D: Header checksum
- 0x014E–0x014F: Global checksum

Réf: [Pan Docs – The Cartridge Header](https://gbdev.io/pandocs/The_Cartridge_Header.html)

## Lecture par l’émulateur

### Chargement
- Lecture du fichier `.gb` en mémoire
- Parsing de l’en-tête pour identifier `cart_type`, `rom_size`, `ram_size`
- Sélection du MBC (MBC1/MBC3/MBC5…) si nécessaire
- Initialisation des banques (ROM0/ROMX, RAM externe)

```mermaid
sequenceDiagram
  participant FS as Fichier ROM
  participant MMU
  participant MBC
  FS->>MMU: lire binaire
  MMU->>MMU: parser header (type/taille)
  MMU->>MBC: init selon cart_type
  MMU->>MMU: map ROM0/ROMX
```

### Accès mémoire
- 0x0000–0x3FFF: ROM banc 0
- 0x4000–0x7FFF: ROM banc sélectionné (MBC)
- Écritures dans 0x0000–0x7FFF utilisées par MBC pour changer de banc/activer RAM/RTC

Réf: [Pan Docs – Memory Map](https://gbdev.io/pandocs/Memory_Map.html), [MBCs](https://gbdev.io/pandocs/MBCs.html)

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
- Respecter l’en-tête (logo, tailles, checksums) pour compatibilité maximale
- Choisir MBC adapté (taille ROM/RAM)
- Éviter d’écrire dans 0x0000–0x7FFF sauf pour commandes MBC
- Tester sur émulateurs et si possible sur hardware

## ROMs de test (répertoire du projet)
- `tests/rom/*.gb`: petites ROMs de debug/visuel/série
- `tests/blargg/`, `tests/mooneye/`: suites de conformité

## Références
- [Pan Docs – The Cartridge Header](https://gbdev.io/pandocs/The_Cartridge_Header.html)
- [Pan Docs – MBCs](https://gbdev.io/pandocs/MBCs.html)
- [RGBDS](https://rgbds.gbdev.io/) · [GBDK-2020](https://gbdk-2020.github.io/)

## Conception d'une ROM (guidelines)

Cette rubrique synthétise les choix et patterns de conception pour des ROMs robustes et compatibles.

### 1) Choix MBC et tailles
- Si ROM ≤ 32 KiB: cartouche «No MBC». Sinon choisir MBC1/MBC3/MBC5 selon besoins (banques ROM, RAM, RTC).
- Définir `ROM size` (0x0148) et `RAM size` (0x0149) cohérents avec l’agencement mémoire.
- Réf: [Pan Docs – MBCs](https://gbdev.io/pandocs/MBCs.html)

### 2) Layout mémoire et banques
- `ROM0` (0x0000–0x3FFF): code commun (reset, ISR, routines bancables-stubs)
- `ROMX` (0x4000–0x7FFF): code/data bancable(s)
- Si banques: centraliser les sélecteurs MBC et une convention d’ID de banque.

### 3) Démarrage et initialisation
- Point d’entrée à 0x0100 (après le boot ROM). Éteindre les interruptions, initialiser SP, IO critiques.
- Éteindre le LCD (LCDC.7=0) avant modifications VRAM/OAM/PPU.
- Initialiser palettes, SCX/SCY, WY/WX, STAT/IE/IF selon besoins.
- Réf: [Pan Docs – Power-Up Sequence](https://gbdev.io/pandocs/Power_Up_Sequence.html) · [LCD Control](https://gbdev.io/pandocs/LCDC.html)

### 4) Interruptions et timing
- VBlank: lieu privilégié pour mises à jour VRAM/OAM et logique frame.
- STAT modes: respecter les fenêtres d’accès VRAM/OAM (voir STAT timing).
- Timer: utiliser TIMA/TMA/TAC pour logique fixe en Hz désirée.
- Réf: [Interrupts](https://gbdev.io/pandocs/Interrupts.html) · [LCD Status](https://gbdev.io/pandocs/STAT.html)

### 5) Pipeline assets (tuiles/sprites)
- Convertir PNG → tuiles via `rgbgfx` (RGBDS) ou outils GBDK.
- Organiser les tilesets et maps par banque si nécessaire, documenter les indices.
- Réf: [Tile Data](https://gbdev.io/pandocs/Tile_Data.html) · [Tile Maps](https://gbdev.io/pandocs/Tile_Maps.html)

### 6) Debug sérial et logs
- Écrire sur SB/SC (0xFF01/0xFF02) avec `SC=0x81` pour tracer des messages vers l’émulateur.
- Pratique pour tests automatisés et vérification d’état.
- Réf: [Serial](https://gbdev.io/pandocs/Serial_Data_Transfer.html)

### 7) Compatibilité et header
- Utiliser `rgbfix` ou l’outil de la toolchain pour le header (logo/titre/checksums).
- Vérifier que `cartridge type` matche le MBC réel.
- Réf: [The Cartridge Header](https://gbdev.io/pandocs/The_Cartridge_Header.html)

### 8) Patterns de boucle jeu
- Boucle principale synchronisée VBlank (`wait_vbl_done` en GBDK) ou ISR VBlank.
- Double-buffering software des commandes VRAM/OAM appliquées à chaque frame.

### 9) Tests et conformité
- Blargg (CPU) et Mooneye (timing) à intégrer tôt.
- Ajouter des micro-ROMs pour chaque fonctionnalité sensible (DMA, STAT, OAM, joypad, timer).

### 10) Check-list rapide
- Header correct, MBC cohérent, tailles alignées
- Init SP/IO/PPU, LCD off avant VRAM, ISR configurées
- Accès VRAM/OAM uniquement dans les fenêtres autorisées
- Logs sérial disponibles pour debug

Réf générale: [Pan Docs](https://gbdev.io/pandocs) (synthèse officielle)
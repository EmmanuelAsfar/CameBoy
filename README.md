# CameBoy - Game Boy Emulator

Un émulateur Game Boy écrit en C, développé avec Cursor et ChatGPT.

## Structure du projet

```
src/
├── cpu.h/.c          # CPU LR35902 (fetch-decode-execute)
├── mmu.h/.c          # Bus mémoire et mapping
├── mbc.h/.c          # Memory Bank Controllers
├── ppu.h/.c          # Picture Processing Unit
├── timer.h/.c        # Timers et DIV
├── joypad.h/.c       # Contrôleur
├── dma.h/.c          # OAM DMA
├── cart.h/.c         # Gestion des cartouches
└── emulator.c        # Boucle principale

docs/
├── pandocs/          # Documentation Pan Docs
└── opcodes.json      # Tables d'opcodes

tests/
├── blargg/           # Tests CPU Blargg
└── mooneye/          # Tests Mooneye

Makefile
```

## Ressources utilisées

- **Pan Docs** : Spécifications officielles Game Boy
- **Blargg Tests** : Tests de conformité CPU
- **Mooneye Tests** : Tests de timing et interrupts
- **Pastraiser** : Tables d'opcodes LR35902

## Compilation

```bash
make
```

## Tests

```bash
make test
```

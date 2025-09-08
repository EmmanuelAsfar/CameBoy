# Game Boy Color (CGB) a" SpAcificitAs et cadrage

Retour: [Index specs](./README.md) A [Architecture](../architecture.md)

## Pourquoi une page dAdiAe ?
CGB introduit des diffArences majeures par rapport A  DMG (couleur, vitesse, mAmoire). MAme si la cible actuelle est DMG, cadrer le scope CGB aide A  planifier laextension.

## DiffArences clAs vs DMG

- Couleurs: palettes BG/OBJ en 15-bit (RGB555), 8 palettes BG, 8 palettes OBJ
- VRAM: 2 banques (bank 0/1) a" tile data/tile maps Atendus
- Vitesse: double-speed mode (CPU x2) a" impact sur timers/APU/PPU
- OAM/PPU: prioritAs et attributs Atendus (CGB attr map)
- DMA: H-Blank DMA (HDMA) pour transferts pendant HBlank
- MAmoire: WRAM/HRAM Atendus (banques WRAM)

## Registres CGB
- Palettes couleur: `0xFF68a"0xFF6B` (BG/OBJ index/data)
- VRAM bank: `0xFF4F`
- WRAM bank: `0xFF70`
- Key1 (double-speed): `0xFF4D`

## Impacts daimplAmentation

- PPU: pipeline doit gArer attributs CGB (palette index, VRAM bank pour tile data)
- Palettes: gestion des donnAes 15-bit et auto-increment des registres palette
- DMA: implAmenter GDMA/HDMA (gAnAral/Horizontal-Blank DMA)
- Timers: double-speed affecte les diviseurs (DIV/TIMA)
- MMU: banques supplAmentaires (VRAM/WRAM), mapping CGB

```
graph LR
  MMU -->|bank select| VRAM0
  MMU -->|bank select| VRAM1
  PPU -->|attr map| Palettes(BG/OBJ 15-bit)
  CPU -->|Key1| Clock[Double-speed]
```

## Tests
- Blargg/Mooneye: tests CGB spAcifiques (palettes, HDMA, speed)
- DAmos homebrew CGB

## StratAgie de migration
1) Stabiliser DMG (CPU/PPU/Timers/IRQ/Joypad)
2) Ajouter VRAM banks + palettes CGB (affichage mono palettes a' couleur)
3) ImplAmenter HDMA + Key1 (double-speed) avec timers ajustAs

## RAfArences
- [CGB Registers](https://gbdev.io/pandocs/CGB_Registers.html)
- [VRAM and Palettes (CGB)](https://gbdev.io/pandocs/Rendering.html)
- [HDMA](https://gbdev.io/pandocs/OAM_DMA_Transfer.html)


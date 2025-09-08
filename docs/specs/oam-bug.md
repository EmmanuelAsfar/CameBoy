# Bug de corruption OAM a" SpAcifications et implications

Retour: [Index specs](./README.md) A [PPU](./ppu.md)

## De quoi saagit-il ?
Le A OAM Corruption Bug A est un comportement matAriel de la DMG oA, dans certaines conditions (Acriture/lecture OAM pendant des phases critiques), les donnAes sprites (OAM) peuvent Atre corrompues.

### Pourquoi Aa existe ?
- Contraintes matArielles du bus: accAs concurrents PPU/CPU
- FenAtres temporelles serrAes pendant OAM Search/Pixel Transfer
- Acritures non arbitAes a' duplication/mirroring partiel de bytes

## Quand survient-il ?
- Acritures/lectures OAM pendant Mode 2 (OAM Search) ou Mode 3 (Pixel Transfer)
- DMA OAM en cours + accAs CPU
- SAquences spAcifiques dainstructions impactant les timings

## SymptAmes
- Sprites incorrects (positions/tiles/attributs A bizarres A)
- Patterns de duplication de bytes (ex: 0xAB a' 0xAA/0xBB)

## StratAgies daAmulation

1) Approche simple (sAcurisAe)
- Interdire strictement lecture/Acriture OAM en Mode 2/3 et pendant DMA a' retourner 0xFF, ignorer writes
- Pros: simple, stable
- Cons: ne reproduit pas toutes corruptions A rAelles A

2) Approche rAaliste (avancAe)
- ModAliser des corruptions probabilistes/dAterministes selon timing/micro-Atats
- ImplAmenter mirroring partiel des nibbles/bytes selon cycles
- Pros: haute fidAlitA pour dAmos/tests exploitant le bug
- Cons: complexitA forte, dApendances timing CPU/PPU prAcises

## Recommandation
- DMG v1: commencer par laapproche simple (bloquage) pour la stabilitA et la conformitA gAnArale
- Aventuellement ajouter un A mode prAcision A activable (corruptions simulAes) pour des ROMs spAcifiques

## Hooks dans la MMU
```c
u8 mmu_read_oam(MMU* mmu, u16 addr) {
  if (ppu_is_mode_2_or_3(&mmu->ppu) || mmu->dma.active) return 0xFF;
  return mmu->oam[addr - 0xFE00];
}

void mmu_write_oam(MMU* mmu, u16 addr, u8 val) {
  if (ppu_is_mode_2_or_3(&mmu->ppu) || mmu->dma.active) return;
  mmu->oam[addr - 0xFE00] = val;
}
```

## Tests
- Mooneye `oam_bug/` (dAclinaisons) a" vArifient timings et effets
- DAmos exploitant le bug (scAnes)

## RAfArence
- [OAM Corruption Bug](https://gbdev.io/pandocs/OAM_Corruption_Bug.html)

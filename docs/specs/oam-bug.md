# Bug de corruption OAM - Spécifications et implications

Retour: [Index specs](./README.md) | [PPU](./ppu.md)

## De quoi s'agit-il ?
Le « OAM Corruption Bug » est un comportement matériel de la DMG où, dans certaines conditions (écriture/lecture OAM pendant des phases critiques), les données sprites (OAM) peuvent être corrompues.

### Pourquoi ça existe ?
- Contraintes matérielles du bus: accès concurrents PPU/CPU
- Fenêtres temporelles serrées pendant OAM Search/Pixel Transfer
- Écritures non arbitrées à duplication/mirroring partiel de bytes

## Quand survient-il ?
- Écritures/lectures OAM pendant Mode 2 (OAM Search) ou Mode 3 (Pixel Transfer)
- DMA OAM en cours + accès CPU
- Séquences spécifiques d'instructions impactant les timings

## Symptômes
- Sprites incorrects (positions/tiles/attributs « bizarres »)
- Patterns de duplication de bytes (ex: 0xAB → 0xAA/0xBB)

## Stratégies d'émulation

1) Approche simple (sécurisée)
- Interdire strictement lecture/écriture OAM en Mode 2/3 et pendant DMA → retourner 0xFF, ignorer writes
- Pros: simple, stable
- Cons: ne reproduit pas toutes corruptions « réelles »

2) Approche réaliste (avancée)
- Modéliser des corruptions probabilistes/déterministes selon timing/micro-états
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

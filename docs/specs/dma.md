# DMA/OAM a" SpAcifications d'implAmentation

Retour: [Index specs](./README.md) A [Architecture](../architecture.md) A [Utilisation](../usage.md)

## Vue d'ensemble

Le DMA (Direct Memory Access) de la Game Boy permet de copier rapidement des donnAes depuis la RAM vers la mAmoire des sprites (OAM). C'est un mAcanisme essentiel pour l'animation des sprites.

### Pourquoi le DMA ?

Le DMA OAM est nAcessaire car :
- **Performance** : Copie 160 octets en 160 cycles (vs 160 instructions)
- **Synchronisation** : Avite les conflits d'accAs pendant le rendu
- **SimplicitA** : Une seule instruction pour copier tous les sprites
- **CompatibilitA** : Beaucoup de jeux l'utilisent

## Registre DMA (0xFF46)

### DAclenchement du DMA
```c
#define DMA_REG 0xFF46

void dma_start(MMU* mmu, u8 source_high) {
    // L'adresse source est (source_high << 8)
    u16 source_addr = source_high << 8;
    
    // VArifier que l'adresse source est valide
    if (source_addr < 0x8000 || source_addr >= 0xFE00) {
        return;  // Adresse invalide
    }
    
    // DAmarrer le DMA
    mmu->dma.active = true;
    mmu->dma.source_addr = source_addr;
    mmu->dma.dest_addr = 0xFE00;  // OAM
    mmu->dma.bytes_remaining = 160;  // 40 sprites A- 4 octets
    mmu->dma.cycles_remaining = 160;  // 1 cycle par octet
}
```

**Pourquoi 160 octets ?** C'est la taille exacte de l'OAM (40 sprites A- 4 octets par sprite).

**Pourquoi 160 cycles ?** Le DMA copie 1 octet par cycle, donc 160 cycles pour 160 octets.

## Processus de copie

### SAquence de copie
`````mermaid`r`nsequenceDiagram
    participant CPU
    participant DMA
    participant OAM
    participant RAM
    
    CPU->>DMA: Acrire source_high dans 0xFF46
    DMA->>DMA: Calculer source_addr = source_high << 8
    DMA->>DMA: Initialiser compteurs (160 octets, 160 cycles)
    
    loop 160 fois
        DMA->>RAM: Lire 1 octet A  source_addr
        DMA->>OAM: Acrire 1 octet A  dest_addr
        DMA->>DMA: source_addr++, dest_addr++, bytes_remaining--
    end
    
    DMA->>DMA: DMA terminA
```

### Mise A  jour du DMA
```c
void dma_tick(MMU* mmu) {
    if (!mmu->dma.active) return;
    
    // VArifier si on peut copier un octet
    if (mmu->dma.cycles_remaining > 0) {
        mmu->dma.cycles_remaining--;
        return;
    }
    
    // Copier un octet
    u8 data = mmu_read8(mmu, mmu->dma.source_addr);
    mmu_write8(mmu, mmu->dma.dest_addr, data);
    
    // Passer A  l'octet suivant
    mmu->dma.source_addr++;
    mmu->dma.dest_addr++;
    mmu->dma.bytes_remaining--;
    
    // VArifier si le DMA est terminA
    if (mmu->dma.bytes_remaining == 0) {
        mmu->dma.active = false;
    } else {
        // RAinitialiser le compteur de cycles
        mmu->dma.cycles_remaining = 1;
    }
}
```

**Pourquoi 1 cycle par octet ?** C'est la vitesse exacte du DMA matAriel de la Game Boy.

## Restrictions d'accAs

### Blocage de l'OAM
```c
bool dma_can_access_oam(MMU* mmu) {
    // Pendant le DMA, l'OAM est inaccessible
    if (mmu->dma.active) {
        return false;
    }
    
    // VArifier aussi les restrictions PPU
    return ppu_can_access_oam(&mmu->ppu);
}
```

**Pourquoi bloquer l'OAM ?** Pendant le DMA, l'OAM est en cours de modification. Les accAs simultanAs causeraient des corruptions.

### Blocage partiel du bus
```c
bool dma_can_access_memory(MMU* mmu, u16 addr) {
    if (!mmu->dma.active) return true;
    
    // Pendant le DMA, seules certaines zones sont accessibles
    if (addr >= 0x8000 && addr < 0xFE00) {
        return true;  // VRAM, WRAM, etc.
    }
    
    return false;  // OAM et autres zones bloquAes
}
```

**Pourquoi ces restrictions ?** Le DMA utilise le bus mAmoire. Certaines zones sont inaccessibles pendant la copie.

## Gestion des accAs

### Lecture OAM pendant DMA
```c
u8 mmu_read_oam_during_dma(MMU* mmu, u16 addr) {
    if (mmu->dma.active) {
        // Pendant le DMA, retourner 0xFF
        return 0xFF;
    }
    
    return mmu->oam[addr - 0xFE00];
}
```

**Pourquoi 0xFF ?** C'est le comportement matAriel. L'OAM retourne 0xFF pendant le DMA.

### Acriture OAM pendant DMA
```c
void mmu_write_oam_during_dma(MMU* mmu, u16 addr, u8 value) {
    if (mmu->dma.active) {
        // Pendant le DMA, ignorer l'Acriture
        return;
    }
    
    mmu->oam[addr - 0xFE00] = value;
}
```

**Pourquoi ignorer ?** Pendant le DMA, l'OAM est en cours de modification. Les Acritures sont ignorAes.

## Synchronisation avec le PPU

### VArification des modes PPU
```c
void dma_check_ppu_mode(MMU* mmu) {
    if (!mmu->dma.active) return;
    
    // Le DMA peut s'exAcuter pendant certains modes PPU
    if (mmu->ppu.mode == PPU_MODE_HBLANK || mmu->ppu.mode == PPU_MODE_VBLANK) {
        // DMA autorisA
        return;
    }
    
    // Dans d'autres modes, suspendre le DMA
    mmu->dma.suspended = true;
}
```

**Pourquoi suspendre ?** Pendant certains modes PPU, l'accAs A  l'OAM est restreint.

### Reprise du DMA
```c
void dma_resume_if_possible(MMU* mmu) {
    if (!mmu->dma.suspended) return;
    
    // VArifier si on peut reprendre
    if (mmu->ppu.mode == PPU_MODE_HBLANK || mmu->ppu.mode == PPU_MODE_VBLANK) {
        mmu->dma.suspended = false;
    }
}
```

**Pourquoi reprendre ?** Le DMA doit continuer dAs que possible pour maintenir la synchronisation.

## Gestion des erreurs

### VArification des adresses
```c
bool dma_validate_source(u16 source_addr) {
    // L'adresse source doit Atre dans une zone accessible
    if (source_addr < 0x8000) return false;  // ROM
    if (source_addr >= 0xFE00) return false;  // OAM et au-delA 
    
    return true;
}
```

**Pourquoi valider ?** Avite de copier depuis des zones inaccessibles ou invalides.

### Gestion des timeouts
```c
void dma_check_timeout(MMU* mmu) {
    if (!mmu->dma.active) return;
    
    // VArifier si le DMA prend trop de temps
    if (mmu->dma.cycles_remaining > DMA_TIMEOUT) {
        // Abandonner le DMA
        mmu->dma.active = false;
        mmu->dma.suspended = false;
    }
}
```

**Pourquoi un timeout ?** Avite que l'Amulateur se bloque si le DMA Achoue.

## Optimisations

### Copie en bloc
```c
void dma_copy_block(MMU* mmu) {
    if (!mmu->dma.active) return;
    
    // Copier tous les octets restants en une fois
    u16 bytes_to_copy = mmu->dma.bytes_remaining;
    
    for (u16 i = 0; i < bytes_to_copy; i++) {
        u8 data = mmu_read8(mmu, mmu->dma.source_addr + i);
        mmu_write8(mmu, mmu->dma.dest_addr + i, data);
    }
    
    // Marquer le DMA comme terminA
    mmu->dma.active = false;
    mmu->dma.bytes_remaining = 0;
}
```

**Pourquoi cette optimisation ?** Dans un Amulateur, on peut copier en bloc au lieu de cycle par cycle.

### VArification des conditions
```c
bool dma_can_start(MMU* mmu) {
    // VArifier que le DMA n'est pas dAjA  actif
    if (mmu->dma.active) return false;
    
    // VArifier que le PPU est dans un mode compatible
    if (mmu->ppu.mode != PPU_MODE_HBLANK && mmu->ppu.mode != PPU_MODE_VBLANK) {
        return false;
    }
    
    return true;
}
```

**Pourquoi vArifier ?** Avite de dAmarrer un DMA dans des conditions incompatibles.

## Initialisation

```c
void dma_init(MMU* mmu) {
    memset(&mmu->dma, 0, sizeof(DMA));
    
    // Valeurs par dAfaut
    mmu->dma.active = false;
    mmu->dma.suspended = false;
    mmu->dma.source_addr = 0;
    mmu->dma.dest_addr = 0xFE00;
    mmu->dma.bytes_remaining = 0;
    mmu->dma.cycles_remaining = 0;
}
```

**Pourquoi ces valeurs ?** Le DMA est inactif par dAfaut.

## Tests de conformitA

### Test de copie
```c
void test_dma_copy() {
    MMU mmu;
    dma_init(&mmu);
    
    // PrAparer les donnAes source
    for (int i = 0; i < 160; i++) {
        mmu_write8(&mmu, 0xC000 + i, i);
    }
    
    // DAmarrer le DMA
    dma_start(&mmu, 0xC0);
    
    // Simuler la copie
    while (mmu.dma.active) {
        dma_tick(&mmu);
    }
    
    // VArifier que les donnAes ont AtA copiAes
    for (int i = 0; i < 160; i++) {
        u8 expected = i;
        u8 actual = mmu_read8(&mmu, 0xFE00 + i);
        assert(actual == expected);
    }
}
```

**Pourquoi ces tests ?** Ils vArifient que le DMA copie correctement les donnAes.

## RAfArences Pan Docs

- [OAM DMA Transfer](https://gbdev.io/pandocs/OAM_DMA_Transfer.html)

### DMA CGB (GDMA/HDMA)

- Registres `FF51`..`FF55` (source, destination, longueur/mode).
- GDMA (General): copie immediate d'un bloc, CPU suspendu pendant la copie.
- HDMA (H-Blank): copie 16 octets par HBlank sans bloquer le CPU; `FF55.bit7=1` pendant le transfert.
- Contraintes: VRAM destination (`0x8000-0x9FFF`) avec alignements specifiques.

Dans CameBoy, le support HDMA sera integre cote MMU/PPU: ecriture des registres, etat de transfert, execution sur HBlank.

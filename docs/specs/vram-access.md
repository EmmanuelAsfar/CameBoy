# Accès VRAM/OAM – Spécifications d'implémentation

Retour: [Index specs](./README.md) · [Architecture](../architecture.md) · [Utilisation](../usage.md)

## Vue d'ensemble

L'accès à la VRAM et à l'OAM est restreint pendant certaines phases du rendu pour éviter les corruptions. Ces restrictions sont cruciales pour la compatibilité des jeux.

### Pourquoi des restrictions ?

Le PPU lit la VRAM et l'OAM pendant le rendu :
- **VRAM** : Données des tuiles et des cartes
- **OAM** : Données des sprites
- **Conflits** : Accès simultanés causent des corruptions
- **Timing** : Le rendu doit être synchronisé

## Zones concernées

### VRAM (0x8000-0x9FFF)
```c
#define VRAM_START 0x8000
#define VRAM_END   0x9FFF

// Sous-zones de la VRAM
#define TILE_DATA_0_START 0x8000  // Données des tuiles (0x8000-0x87FF)
#define TILE_DATA_1_START 0x8800  // Données des tuiles (0x8800-0x8FFF)
#define TILE_MAP_0_START  0x9800  // Carte d'arrière-plan 0 (0x9800-0x9BFF)
#define TILE_MAP_1_START  0x9C00  // Carte d'arrière-plan 1 (0x9C00-0x9FFF)
```

**Pourquoi ces zones ?** Chaque zone a un rôle spécifique dans le rendu :
- **TILE_DATA** : Données des tuiles (8x8 pixels)
- **TILE_MAP** : Cartes d'arrière-plan (quelles tuiles afficher)

### OAM (0xFE00-0xFE9F)
```c
#define OAM_START 0xFE00
#define OAM_END   0xFE9F

// Structure d'un sprite (4 octets)
typedef struct {
    u8 y;        // Position Y
    u8 x;        // Position X
    u8 tile;     // Numéro de tuile
    u8 flags;    // Attributs (priorité, palette, etc.)
} Sprite;
```

**Pourquoi cette structure ?** C'est le format standard des sprites Game Boy, défini par le matériel.

## Modes PPU et restrictions

### Restrictions par mode
```c
typedef enum {
    PPU_MODE_HBLANK = 0,      // HBlank : accès libre
    PPU_MODE_VBLANK = 1,      // VBlank : accès libre
    PPU_MODE_OAM_SEARCH = 2,  // OAM Search : OAM bloqué
    PPU_MODE_PIXEL_TRANSFER = 3  // Pixel Transfer : VRAM et OAM bloqués
} PPUMode;

bool can_access_vram(PPUMode mode) {
    return mode == PPU_MODE_HBLANK || mode == PPU_MODE_VBLANK;
}

bool can_access_oam(PPUMode mode) {
    return mode == PPU_MODE_HBLANK || mode == PPU_MODE_VBLANK;
}
```

**Pourquoi ces restrictions ?** Chaque mode correspond à une phase du rendu :
- **HBlank/VBlank** : Pas de rendu actif, accès libre
- **OAM Search** : Lecture de l'OAM, écriture bloquée
- **Pixel Transfer** : Lecture de la VRAM, écriture bloquée

### Diagramme des restrictions
```mermaid
gantt
    title Restrictions d'accès VRAM/OAM
    dateFormat X
    axisFormat %s
    
    section Mode 0 (HBlank)
    VRAM libre    :0, 204
    OAM libre     :0, 204
    
    section Mode 2 (OAM Search)
    VRAM libre    :204, 284
    OAM bloqué    :204, 284
    
    section Mode 3 (Pixel Transfer)
    VRAM bloqué   :284, 456
    OAM bloqué    :284, 456
```

## Gestion des accès

### Vérification des accès
```c
bool mmu_can_access_vram(MMU* mmu, u16 addr) {
    // Vérifier que l'adresse est dans la VRAM
    if (addr < VRAM_START || addr > VRAM_END) {
        return true;  // Pas de VRAM
    }
    
    // Vérifier le mode PPU
    if (!can_access_vram(mmu->ppu.mode)) {
        return false;  // Accès bloqué
    }
    
    // Vérifier le DMA
    if (mmu->dma.active) {
        return false;  // DMA en cours
    }
    
    return true;
}

bool mmu_can_access_oam(MMU* mmu, u16 addr) {
    // Vérifier que l'adresse est dans l'OAM
    if (addr < OAM_START || addr > OAM_END) {
        return true;  // Pas d'OAM
    }
    
    // Vérifier le mode PPU
    if (!can_access_oam(mmu->ppu.mode)) {
        return false;  // Accès bloqué
    }
    
    // Vérifier le DMA
    if (mmu->dma.active) {
        return false;  // DMA en cours
    }
    
    return true;
}
```

**Pourquoi ces vérifications ?** Elles empêchent les accès non autorisés qui causeraient des corruptions.

### Lecture avec restrictions
```c
u8 mmu_read_vram(MMU* mmu, u16 addr) {
    if (!mmu_can_access_vram(mmu, addr)) {
        // Pendant les restrictions, retourner 0xFF
        return 0xFF;
    }
    
    return mmu->vram[addr - VRAM_START];
}

u8 mmu_read_oam(MMU* mmu, u16 addr) {
    if (!mmu_can_access_oam(mmu, addr)) {
        // Pendant les restrictions, retourner 0xFF
        return 0xFF;
    }
    
    return mmu->oam[addr - OAM_START];
}
```

**Pourquoi 0xFF ?** C'est le comportement matériel. Les zones bloquées retournent 0xFF.

### Écriture avec restrictions
```c
void mmu_write_vram(MMU* mmu, u16 addr, u8 value) {
    if (!mmu_can_access_vram(mmu, addr)) {
        // Pendant les restrictions, ignorer l'écriture
        return;
    }
    
    mmu->vram[addr - VRAM_START] = value;
}

void mmu_write_oam(MMU* mmu, u16 addr, u8 value) {
    if (!mmu_can_access_oam(mmu, addr)) {
        // Pendant les restrictions, ignorer l'écriture
        return;
    }
    
    mmu->oam[addr - OAM_START] = value;
}
```

**Pourquoi ignorer ?** Pendant les restrictions, les écritures sont ignorées pour éviter les corruptions.

## Gestion des conflits

### Détection des conflits
```c
void mmu_check_access_conflicts(MMU* mmu, u16 addr, bool is_write) {
    if (addr >= VRAM_START && addr <= VRAM_END) {
        if (!mmu_can_access_vram(mmu, addr)) {
            if (is_write) {
                // Écriture bloquée dans la VRAM
                mmu->vram_write_blocked = true;
            } else {
                // Lecture bloquée dans la VRAM
                mmu->vram_read_blocked = true;
            }
        }
    }
    
    if (addr >= OAM_START && addr <= OAM_END) {
        if (!mmu_can_access_oam(mmu, addr)) {
            if (is_write) {
                // Écriture bloquée dans l'OAM
                mmu->oam_write_blocked = true;
            } else {
                // Lecture bloquée dans l'OAM
                mmu->oam_read_blocked = true;
            }
        }
    }
}
```

**Pourquoi détecter ?** Pour informer l'utilisateur des accès bloqués et aider au débogage.

### Gestion des erreurs
```c
void mmu_handle_access_error(MMU* mmu, u16 addr, bool is_write) {
    if (mmu->vram_write_blocked || mmu->oam_write_blocked) {
        // Log de l'erreur
        printf("ERREUR: Écriture bloquée à 0x%04X (mode PPU: %d)\n", 
               addr, mmu->ppu.mode);
    }
    
    if (mmu->vram_read_blocked || mmu->oam_read_blocked) {
        // Log de l'erreur
        printf("ERREUR: Lecture bloquée à 0x%04X (mode PPU: %d)\n", 
               addr, mmu->ppu.mode);
    }
}
```

**Pourquoi gérer les erreurs ?** Pour aider au débogage et à la compréhension des restrictions.

## Optimisations

### Cache des restrictions
```c
typedef struct {
    bool vram_read_allowed;
    bool vram_write_allowed;
    bool oam_read_allowed;
    bool oam_write_allowed;
    PPUMode last_mode;
} AccessCache;

void mmu_update_access_cache(MMU* mmu) {
    if (mmu->access_cache.last_mode != mmu->ppu.mode) {
        mmu->access_cache.vram_read_allowed = can_access_vram(mmu->ppu.mode);
        mmu->access_cache.vram_write_allowed = can_access_vram(mmu->ppu.mode);
        mmu->access_cache.oam_read_allowed = can_access_oam(mmu->ppu.mode);
        mmu->access_cache.oam_write_allowed = can_access_oam(mmu->ppu.mode);
        mmu->access_cache.last_mode = mmu->ppu.mode;
    }
}
```

**Pourquoi un cache ?** Évite de recalculer les restrictions à chaque accès.

### Vérification rapide
```c
bool mmu_can_access_vram_fast(MMU* mmu, u16 addr) {
    if (addr < VRAM_START || addr > VRAM_END) {
        return true;
    }
    
    mmu_update_access_cache(mmu);
    return mmu->access_cache.vram_read_allowed;
}
```

**Pourquoi cette optimisation ?** Améliore les performances en évitant les calculs répétitifs.

## Tests de conformité

### Test des restrictions
```c
void test_vram_access_restrictions() {
    MMU mmu;
    mmu_init(&mmu);
    
    // Mode OAM Search (VRAM libre, OAM bloqué)
    mmu.ppu.mode = PPU_MODE_OAM_SEARCH;
    
    // VRAM doit être accessible
    assert(mmu_can_access_vram(&mmu, 0x8000));
    
    // OAM doit être bloqué
    assert(!mmu_can_access_oam(&mmu, 0xFE00));
    
    // Mode Pixel Transfer (VRAM et OAM bloqués)
    mmu.ppu.mode = PPU_MODE_PIXEL_TRANSFER;
    
    // VRAM doit être bloqué
    assert(!mmu_can_access_vram(&mmu, 0x8000));
    
    // OAM doit être bloqué
    assert(!mmu_can_access_oam(&mmu, 0xFE00));
}
```

**Pourquoi ces tests ?** Ils vérifient que les restrictions sont appliquées correctement.

## Références Pan Docs

- [Accessing VRAM and OAM](https://gbdev.io/pandocs/Accessing_VRAM_and_OAM.html)
- [OAM Corruption Bug](https://gbdev.io/pandocs/OAM_Corruption_Bug.html)

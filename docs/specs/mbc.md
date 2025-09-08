# MBC (Memory Bank Controllers) – Spécifications d'implémentation

Retour: [Index specs](./README.md) · [Architecture](../architecture.md) · [Utilisation](../usage.md)

## Vue d'ensemble

Les MBC (Memory Bank Controllers) sont des circuits intégrés dans les cartouches Game Boy qui permettent de gérer des ROMs et des RAMs plus grandes que ce que la Game Boy peut adresser directement.

### Pourquoi des MBC ?

La Game Boy a des limitations d'adressage :
- **ROM** : 32KB maximum (0x0000-0x7FFF)
- **RAM** : 8KB maximum (0xA000-0xBFFF)
- **Jeux plus gros** : Besoin de "bank switching" pour accéder à plus de données

## Types de MBC

### MBC1 (le plus courant)
```c
typedef struct {
    u8 rom_bank;      // Banc ROM actuel (1-127)
    u8 ram_bank;      // Banc RAM actuel (0-3)
    u8 ram_enabled;   // RAM activée
    u8 banking_mode;  // Mode de banking (0=ROM, 1=RAM)
} MBC1;
```

**Pourquoi MBC1 ?** C'est le MBC le plus simple et le plus courant. Il permet de gérer jusqu'à 2MB de ROM et 32KB de RAM.

### MBC3 (avec RTC)
```c
typedef struct {
    u8 rom_bank;      // Banc ROM actuel (1-127)
    u8 ram_bank;      // Banc RAM actuel (0-3)
    u8 ram_enabled;   // RAM activée
    u8 rtc_enabled;   // RTC activé
    u8 rtc_reg;       // Registre RTC actuel
    RTC rtc;          // Horloge temps réel
} MBC3;
```

**Pourquoi MBC3 ?** Il ajoute une horloge temps réel (RTC) pour les jeux qui ont besoin de l'heure (ex: Pokémon).

### MBC5 (le plus avancé)
```c
typedef struct {
    u16 rom_bank;     // Banc ROM actuel (1-511)
    u8 ram_bank;      // Banc RAM actuel (0-15)
    u8 ram_enabled;   // RAM activée
    u8 rumble;        // Moteur de vibration
} MBC5;
```

**Pourquoi MBC5 ?** Il supporte des ROMs jusqu'à 8MB et des RAMs jusqu'à 128KB, plus des fonctionnalités avancées.

## Détection du type de MBC

### En-tête de cartouche
```c
typedef struct {
    u8 entry_point[4];     // Point d'entrée
    u8 logo[48];           // Logo Nintendo
    char title[16];        // Titre du jeu
    u8 cgb_flag;           // Support CGB
    u8 new_licensee[2];    // Nouveau licencié
    u8 sgb_flag;           // Support SGB
    u8 cart_type;          // Type de cartouche (MBC)
    u8 rom_size;           // Taille de la ROM
    u8 ram_size;           // Taille de la RAM
    u8 destination;        // Destination (JPN/World)
    u8 old_licensee;       // Ancien licencié
    u8 version;            // Version du jeu
    u8 checksum;           // Checksum
    u16 global_checksum;   // Checksum global
} CartridgeHeader;
```

**Pourquoi cette structure ?** C'est le format standard des en-têtes de cartouche Game Boy, défini par Nintendo.

### Détection du MBC
```c
MBCType detect_mbc_type(u8 cart_type) {
    switch (cart_type) {
        case 0x00: return MBC_NONE;
        case 0x01: case 0x02: case 0x03: return MBC1;
        case 0x05: case 0x06: return MBC2;
        case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13: return MBC3;
        case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: return MBC5;
        default: return MBC_UNKNOWN;
    }
}
```

**Pourquoi cette détection ?** Chaque type de MBC a un code spécifique dans l'en-tête de la cartouche.

## Gestion des bancs ROM

### MBC1 - Banking ROM
```c
void mbc1_write_rom_bank(MMU* mmu, u16 addr, u8 value) {
    if (addr >= 0x2000 && addr < 0x4000) {
        // Sélection du banc ROM (bits 0-4)
        u8 bank = value & 0x1F;
        if (bank == 0) bank = 1;  // Banc 0 invalide
        mmu->mbc1.rom_bank = bank;
    } else if (addr >= 0x4000 && addr < 0x6000) {
        // Sélection du banc ROM haut (bits 5-6)
        u8 high_bank = (value & 0x03) << 5;
        mmu->mbc1.rom_bank = (mmu->mbc1.rom_bank & 0x1F) | high_bank;
    }
}
```

**Pourquoi cette logique ?** MBC1 utilise 7 bits pour sélectionner le banc ROM (1-127), répartis sur deux registres.

### MBC5 - Banking ROM
```c
void mbc5_write_rom_bank(MMU* mmu, u16 addr, u8 value) {
    if (addr >= 0x2000 && addr < 0x3000) {
        // Banc ROM bas (bits 0-7)
        mmu->mbc5.rom_bank = (mmu->mbc5.rom_bank & 0x100) | value;
    } else if (addr >= 0x3000 && addr < 0x4000) {
        // Banc ROM haut (bit 8)
        mmu->mbc5.rom_bank = (mmu->mbc5.rom_bank & 0xFF) | ((value & 0x01) << 8);
    }
}
```

**Pourquoi 9 bits ?** MBC5 supporte jusqu'à 512 bancs ROM (2^9), permettant des ROMs jusqu'à 8MB.

## Gestion des bancs RAM

### MBC1 - Banking RAM
```c
void mbc1_write_ram_bank(MMU* mmu, u16 addr, u8 value) {
    if (addr >= 0x4000 && addr < 0x6000) {
        // Sélection du banc RAM (bits 0-1)
        mmu->mbc1.ram_bank = value & 0x03;
    } else if (addr >= 0x6000 && addr < 0x8000) {
        // Changement de mode de banking
        mmu->mbc1.banking_mode = value & 0x01;
    }
}
```

**Pourquoi deux modes ?** MBC1 peut utiliser les bits de banc RAM soit pour la RAM, soit pour la ROM haute.

### MBC3 - Banking RAM
```c
void mbc3_write_ram_bank(MMU* mmu, u16 addr, u8 value) {
    if (addr >= 0x4000 && addr < 0x6000) {
        // Sélection du banc RAM ou RTC
        if (value >= 0x00 && value <= 0x03) {
            mmu->mbc3.ram_bank = value;
            mmu->mbc3.rtc_enabled = false;
        } else if (value >= 0x08 && value <= 0x0C) {
            mmu->mbc3.rtc_reg = value - 0x08;
            mmu->mbc3.rtc_enabled = true;
        }
    }
}
```

**Pourquoi RTC ?** MBC3 intègre une horloge temps réel pour les jeux qui en ont besoin.

## Activation/désactivation de la RAM

### Contrôle de la RAM
```c
void mbc_write_ram_enable(MMU* mmu, u16 addr, u8 value) {
    if (addr >= 0x0000 && addr < 0x2000) {
        // Activation/désactivation de la RAM
        u8 enable = (value & 0x0A) == 0x0A;
        mmu->mbc.ram_enabled = enable;
    }
}
```

**Pourquoi 0x0A ?** C'est la valeur magique pour activer la RAM. D'autres valeurs la désactivent.

### Protection contre les écritures accidentelles
```c
bool mbc_can_write_ram(MMU* mmu) {
    return mmu->mbc.ram_enabled;
}
```

**Pourquoi cette protection ?** Évite de corrompre la RAM en cas d'écriture accidentelle.

## Accès aux données

### Lecture ROM
```c
u8 mbc_read_rom(MMU* mmu, u16 addr) {
    if (addr < 0x4000) {
        // Banc 0 (toujours le même)
        return mmu->rom[addr];
    } else {
        // Banc sélectionné
        u16 bank_addr = (mmu->mbc.rom_bank * 0x4000) + (addr - 0x4000);
        return mmu->rom[bank_addr];
    }
}
```

**Pourquoi cette logique ?** La zone 0x0000-0x3FFF contient toujours le banc 0, la zone 0x4000-0x7FFF contient le banc sélectionné.

### Lecture/Écriture RAM
```c
u8 mbc_read_ram(MMU* mmu, u16 addr) {
    if (!mbc_can_write_ram(mmu)) {
        return 0xFF;  // RAM désactivée
    }
    
    u16 ram_addr = (mmu->mbc.ram_bank * 0x2000) + (addr - 0xA000);
    return mmu->ram[ram_addr];
}

void mbc_write_ram(MMU* mmu, u16 addr, u8 value) {
    if (!mbc_can_write_ram(mmu)) {
        return;  // RAM désactivée
    }
    
    u16 ram_addr = (mmu->mbc.ram_bank * 0x2000) + (addr - 0xA000);
    mmu->ram[ram_addr] = value;
}
```

**Pourquoi cette logique ?** La RAM est mappée à partir de 0xA000, avec des bancs de 8KB.

## RTC (Real Time Clock)

### Structure RTC
```c
typedef struct {
    u8 seconds;    // Secondes (0-59)
    u8 minutes;    // Minutes (0-59)
    u8 hours;      // Heures (0-23)
    u8 days_low;   // Jours bas (0-255)
    u8 days_high;  // Jours haut (bit 0-6, bit 7 = overflow)
    u8 halt;       // Arrêt (bit 7)
    u8 latch;      // Verrouillage (bit 0)
} RTC;
```

**Pourquoi cette structure ?** C'est le format standard du RTC MBC3, compatible avec les jeux existants.

### Mise à jour du RTC
```c
void rtc_update(RTC* rtc, u32 cycles) {
    if (rtc->halt) return;  // RTC arrêté
    
    rtc->cycles += cycles;
    
    // Mettre à jour les secondes
    if (rtc->cycles >= 4194304) {  // 1 seconde
        rtc->cycles -= 4194304;
        rtc->seconds++;
        
        if (rtc->seconds >= 60) {
            rtc->seconds = 0;
            rtc->minutes++;
            
            if (rtc->minutes >= 60) {
                rtc->minutes = 0;
                rtc->hours++;
                
                if (rtc->hours >= 24) {
                    rtc->hours = 0;
                    rtc->days_low++;
                    
                    if (rtc->days_low == 0) {
                        rtc->days_high++;
                        if (rtc->days_high >= 128) {
                            rtc->days_high = 0;  // Overflow
                        }
                    }
                }
            }
        }
    }
}
```

**Pourquoi cette logique ?** Le RTC compte le temps réel, avec des conversions entre cycles CPU et unités de temps.

## Initialisation

```c
void mbc_init(MMU* mmu, MBCType type) {
    memset(&mmu->mbc, 0, sizeof(MBC));
    mmu->mbc.type = type;
    
    // Valeurs par défaut
    mmu->mbc.rom_bank = 1;  // Banc 0 invalide
    mmu->mbc.ram_bank = 0;
    mmu->mbc.ram_enabled = false;
    mmu->mbc.banking_mode = 0;
}
```

**Pourquoi ces valeurs ?** Ce sont les valeurs par défaut des MBC au démarrage.

## Tests de conformité

### Test de banking ROM
```c
void test_mbc1_rom_banking() {
    MMU mmu;
    mbc_init(&mmu, MBC1);
    
    // Sélectionner le banc 5
    mbc1_write_rom_bank(&mmu, 0x2000, 5);
    
    // Lire depuis la zone haute
    u8 value = mbc_read_rom(&mmu, 0x4000);
    
    // Vérifier que c'est bien le banc 5
    assert(value == mmu.rom[0x4000 * 5]);
}
```

**Pourquoi ces tests ?** Ils vérifient que le banking fonctionne correctement.

## Références Pan Docs

- [The Cartridge Header](https://gbdev.io/pandocs/The_Cartridge_Header.html)
- [MBCs](https://gbdev.io/pandocs/MBCs.html)

### Autres MBC et RTC (aper�u)

- MBC2, MBC6, MBC7, MMM01, M161, HuC1/HuC3: variantes de banking � documenter progressivement.
- MBC3 RTC: registres temps r�el (sec/min/h/low day/high+flags), latch, halt, day carry; persistance.
- Sauvegardes: d�finir un format de fichiers SRAM/RTC simple par cartouche.

P�dagogie: commencer par MBC1/MBC3/MBC5, ajouter le RTC MBC3 (latching, halt) quand n�cessaire.

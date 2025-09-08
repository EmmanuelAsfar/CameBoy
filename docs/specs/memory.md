# MAmoire (Memory Map) a" SpAcifications d'implAmentation

Retour: [Index specs](./README.md) A [Architecture](../architecture.md) A [Utilisation](../usage.md)

## 1) Principe de fonctionnement (Game Boy)

La Game Boy utilise un adressage 16-bit (0x0000a"0xFFFF) dAcoupA en zones. Certaines zones sont restreintes selon laAtat vidAo (PPU) et des opArations spAciales (DMA OAM).

### Carte mAmoire (DMG)

```
flowchart TD
    A[0x0000-0x3FFF<br/>ROM0<br/>Banc 0 fixe]:::rom
    B[0x4000-0x7FFF<br/>ROMX<br/>Bancs commutables]:::rom
    C[0x8000-0x9FFF<br/>VRAM<br/>DonnAes vidAo]:::vram
    D[0xA000-0xBFFF<br/>ERAM<br/>RAM externe]:::ram
    E[0xC000-0xDFFF<br/>WRAM<br/>RAM de travail]:::ram
    F[0xE000-0xFDFF<br/>Echo<br/>Miroir WRAM]:::echo
    G[0xFE00-0xFE9F<br/>OAM<br/>Sprites]:::oam
    H[0xFEA0-0xFEFF<br/>Unusable<br/>Zone interdite]:::bad
    I[0xFF00-0xFF7F<br/>IO<br/>Registres pAriphAriques]:::io
    J[0xFF80-0xFFFE<br/>HRAM<br/>RAM haute vitesse]:::ram
    K[0xFFFF<br/>IE<br/>Masque interruptions]:::io

    classDef rom fill:#ffd,stroke:#aa0,stroke-width:2px
    classDef vram fill:#dfd,stroke:#0a0,stroke-width:2px
    classDef ram fill:#ddf,stroke:#00a,stroke-width:2px
    classDef echo fill:#eef,stroke:#88f,stroke-width:2px,stroke-dasharray:4 2
    classDef oam fill:#fdd,stroke:#a00,stroke-width:2px
    classDef io fill:#def,stroke:#06a,stroke-width:2px
    classDef bad fill:#eee,stroke:#999,stroke-width:2px,stroke-dasharray:2 2
```

- Echo RAM (0xE000a"0xFDFF) miroite WRAM (0xC000a"0xDDFF)
- Zone 0xFEA0a"0xFEFF inutilisable (lectures=0xFF, Acritures ignorAes)

### Restrictions daaccAs VRAM/OAM (timing PPU)

- Mode 3 (Pixel Transfer): VRAM bloquAe
- Modes 2 (OAM Search) et 3: OAM bloquAe
- Pendant un DMA OAM: OAM bloquAe

```
gantt
  title Restrictions d'accAs VRAM/OAM (par ligne)
  dateFormat X
  axisFormat %s
  section Modes visibles
  OAM Search (2): done, 0, 80
  Pixel Transfer (3): done, 80, 172
  HBlank (0): 204, 204
  section VBlank
  VBlank (1): 456, 456
```

### OAM DMA (0xFF46)

- Acrire `val` A  `0xFF46` copie 160 octets depuis `(val << 8)` vers OAM (0xFE00a"0xFE9F)
- Vitesse matArielle: 1 octet/cycle (nous offrons une version synchrone simple)

### Pourquoi ces rAgles ? (non-expert)
- Le PPU a besoin daun accAs prioritaire A  VRAM/OAM pendant le dessin de laimage; bloquer les accAs CPU Avite les corruptions
- Le DMA OAM accAlAre la mise A  jour des sprites (copie daun bloc)

RAfs Pan Docs: Memory Map, Accessing VRAM and OAM, OAM DMA Transfer

---

## 2) Logique daimplAmentation (CameBoy)

### Structure et liaisons
- `MMU` stocke des pointeurs sur les rAgions (`rom`, `vram`, `eram`, `wram`, `oam`, `io`, `hram`) et sur les composants (`timer`, `apu`, `joypad`, `ppu`)
- Ajout `mmu_set_ppu(mmu, ppu)` pour connaAtre le `PPU->mode`
- Atat DMA: `mmu->dma { active, source_addr, index }`

### AccAs mAmoire (rAgles clAs)
- `mmu_read8/write8`:
  - ROM/ERAM: routAs via `mbc_read/mbc_write` (MBC1 minimal)
  - VRAM: lecture/Acriture bloquAes en Mode 3 (retour 0xFF / ignore write)
  - OAM: lecture/Acriture bloquAes en Modes 2/3 et pendant DMA
  - Echo RAM: miroir WRAM
  - IO:
    - Joypad `0xFF00` routA via `joypad_read/write`
    - Timers `0xFF04a"0xFF07` routAs vers `timer_read/write`
    - Audio `0xFF10a"0xFF3F` routAs vers `apu_read/write`
    - OAM DMA `0xFF46`: dAclenche la copie OAM (synchrone)
  - HRAM/IE: accAs directs

### OAM DMA (implAmentation actuelle)
- Version simple synchrone: boucle 160 octets, `mmu->oam[i] = mmu_read8(mmu, src+i)`
- Drapeau `dma.active` remis A  `false` immAdiatement aprAs la copie
- Option future: version timAe (1 octet par tick) si nAcessaire pour des ROMs Mooneye

### VRAM/OAM restrictions
- Fonctions utilitaires dans `mmu.c`:
  - `mmu_vram_access_allowed(mmu)`: refuse en Mode 3
  - `mmu_oam_access_allowed(mmu)`: refuse en Mode 2/3 ou DMA actif

### MBC
- MBC1 minimal suffisant pour Blargg de base (ROM banking, RAM enable/bank, mode)
- MBC3/MBC5 A  venir

### Points assumAs/paramAtrAs
- `LY` reset A  `0x90` (compat Blargg); un mode strict Pan Docs (0x00) pourra Atre ajoutA si besoin
- SArie (SB/SC): transfert immAdiat pour logs/GUI; temporisation sArie non implAmentAe (piste future)

---

## 3) StratAgie de test (unitaires)

Tests dans `tests/unit/test_mmu.c` (liste et objectifs):

- `test_mmu_init`
  - Alloue 64KB, vArifie les pointeurs des rAgions
- `test_mmu_reset`
  - VArifie les valeurs IO de power-up (P1, LCDC, STAT, LY, etc.)
- `test_mmu_memory_mapping`
  - Lecture des bornes de chaque zone et daIE
- `test_mmu_cart_parsing`
  - Parse un header ROM mAmoire et vArifie type/tailles
- `test_mmu_read_write_8bit`
  - Acritures/lectures basiques WRAM/VRAM/OAM/HRAM/IE
- `test_mmu_read_write_16bit`
  - Little-endian: vArifie LSB/MSB, HRAM 16-bit
- `test_mmu_echo_ram`
  - Miroir WRAMaEcho: Acritures rApercutAes
- `test_mmu_vram_oam_restrictions` (nouveau)
  - Lie `PPU` A  `MMU`, force Mode 3 (VRAM bloquAe) et Mode 2 (OAM bloquAe); vArifie 0xFF/ignorAs; vArifie accAs libres en HBlank
- `test_mmu_dma_oam_copy` (nouveau)
  - PrApare source WRAM `0xC000`, Acrit `0xFF46=0xC0`, vArifie que 160 octets ont AtA recopiAs vers OAM

```
sequenceDiagram
  participant T as Test
  participant MMU
  participant PPU
  T->>PPU: set mode = PIXEL_TRANSFER
  T->>MMU: write 0x8000
  MMU-->>T: read 0x8000 = 0xFF (bloquA)
  T->>PPU: set mode = OAM_SEARCH
  T->>MMU: write 0xFE00
  MMU-->>T: read 0xFE00 = 0xFF (bloquA)
  T->>PPU: set mode = HBLANK
  T->>MMU: write 0x8000/0xFE00
  MMU-->>T: reads = valeurs Acrites
```

### Couverture et limites
- Couvre mapping, power-up, echo, restrictions VRAM/OAM, DMA OAM
- SArie cadencAe et MBC3/5 non couverts (pistes futures)

---

## RAfArences Pan Docs
- [Memory Map](https://gbdev.io/pandocs/Memory_Map.html)
- [I/O Ports](https://gbdev.io/pandocs/I_O_Ports.html)
- [Accessing VRAM and OAM](https://gbdev.io/pandocs/Accessing_VRAM_and_OAM.html)
- [OAM DMA Transfer](https://gbdev.io/pandocs/OAM_DMA_Transfer.html)

# SAquence de dAmarrage a" SpAcifications d'implAmentation

Retour: [Index specs](./README.md) A [Architecture](../architecture.md) A [Utilisation](../usage.md)

## Vue d'ensemble

La sAquence de dAmarrage (power-up) de la Game Boy initialise tous les composants avec des valeurs spAcifiques. Cette initialisation est cruciale pour la compatibilitA des jeux.

### Pourquoi une sAquence spAcifique ?

La Game Boy a un comportement de dAmarrage prAcis :
- **Valeurs initiales** : Chaque registre a une valeur spAcifique
- **Timing** : L'ordre d'initialisation est important
- **CompatibilitA** : Les jeux s'appuient sur ces valeurs
- **StabilitA** : Avite les Atats indAterminAs

## SAquence de dAmarrage

### Atapes du dAmarrage
```
sequenceDiagram
    participant Power as Alimentation
    participant CPU as CPU
    participant PPU as PPU
    participant Timer as Timer
    participant MMU as MMU
    
    Power->>CPU: Reset
    CPU->>CPU: PC = 0x0000
    CPU->>CPU: IME = false
    CPU->>PPU: Initialisation
    CPU->>Timer: Initialisation
    CPU->>MMU: Initialisation
    CPU->>CPU: ExAcution du Boot ROM
    CPU->>CPU: PC = 0x0100 (ROM du jeu)
```

**Pourquoi cette sAquence ?** L'ordre d'initialisation est important pour Aviter les conflits entre composants.

### Boot ROM
```c
// Le Boot ROM est exAcutA en premier (0x0000-0x00FF)
void boot_rom_execute(CPU* cpu, MMU* mmu) {
    // Le Boot ROM vArifie l'intAgritA de la cartouche
    // et affiche le logo Nintendo
    
    // A la fin, il saute A  0x0100 (dAbut de la ROM du jeu)
    cpu->pc = 0x0100;
}
```

**Pourquoi un Boot ROM ?** Il vArifie l'intAgritA de la cartouche et affiche le logo Nintendo avant de lancer le jeu.

## Initialisation du CPU

### Registres CPU
```c
void cpu_power_up(CPU* cpu) {
    // Valeurs exactes de power-up
    cpu->a = 0x01;
    cpu->f = 0xB0;  // Flags Z=1, N=0, H=1, C=0
    cpu->b = 0x00;
    cpu->c = 0x13;
    cpu->d = 0x00;
    cpu->e = 0xD8;
    cpu->h = 0x01;
    cpu->l = 0x4D;
    cpu->sp = 0xFFFE;
    cpu->pc = 0x0000;  // DAmarre au Boot ROM
    
    // Atat des interruptions
    cpu->ime = false;  // Interruptions dAsactivAes
    cpu->halted = false;
    cpu->halt_bug = false;
}
```

**Pourquoi ces valeurs ?** Ce sont les valeurs exactes de la Game Boy au dAmarrage, importantes pour la compatibilitA.

### Flags CPU
```c
// Valeurs des flags au dAmarrage
#define POWER_UP_FLAGS 0xB0  // 10110000

// DAtail des flags
#define FLAG_Z 0x80  // 1 (zAro)
#define FLAG_N 0x40  // 0 (pas de soustraction)
#define FLAG_H 0x20  // 1 (demi-retient)
#define FLAG_C 0x10  // 0 (pas de retient)
```

**Pourquoi ces flags ?** Ils reflAtent l'Atat du CPU aprAs l'initialisation matArielle.

## Initialisation du PPU

### Registres PPU
```c
void ppu_power_up(PPU* ppu) {
    // Valeurs exactes de power-up
    ppu->lcdc = 0x91;  // LCD activA, BG activA
    ppu->stat = 0x85;  // Mode VBlank
    ppu->scy = 0x00;   // Scroll Y
    ppu->scx = 0x00;   // Scroll X
    ppu->ly = 0x00;    // Ligne courante
    ppu->lyc = 0x00;   // LYC
    ppu->bgp = 0xFC;   // Palette BG
    ppu->obp0 = 0xFF;  // Palette OBJ 0
    ppu->obp1 = 0xFF;  // Palette OBJ 1
    ppu->wy = 0x00;    // Window Y
    ppu->wx = 0x00;    // Window X
    
    // Atat interne
    ppu->mode = PPU_MODE_HBLANK;
    ppu->mode_cycles = 0;
    ppu->line_cycles = 0;
}
```

**Pourquoi ces valeurs ?** Elles correspondent A  l'Atat du PPU aprAs l'initialisation matArielle.

### Mode PPU initial
```c
// Le PPU dAmarre en mode HBlank
ppu->mode = PPU_MODE_HBLANK;
ppu->ly = 0x00;  // Ligne 0
```

**Pourquoi HBlank ?** C'est le mode le plus sAr pour l'initialisation, sans conflits d'accAs.

## Initialisation des timers

### Registres Timer
```c
void timer_power_up(Timer* timer) {
    // Valeurs exactes de power-up
    timer->div = 0xAB;    // DIV commence A  0xAB
    timer->tima = 0x00;   // TIMA commence A  0x00
    timer->tma = 0x00;    // TMA commence A  0x00
    timer->tac = 0xF8;    // TAC commence A  0xF8 (timer dAsactivA)
    
    // Atat interne
    timer->div_cycles = 0;
    timer->tima_cycles = 0;
    timer->overflow = false;
}
```

**Pourquoi ces valeurs ?** Elles reflAtent l'Atat des timers aprAs l'initialisation matArielle.

### DIV initial
```c
// DIV commence A  0xAB, pas 0x00
timer->div = 0xAB;
```

**Pourquoi 0xAB ?** C'est la valeur exacte de DIV au dAmarrage, importante pour la synchronisation.

## Initialisation de la MMU

### Registres IO
```c
void mmu_power_up(MMU* mmu) {
    // Valeurs exactes de power-up
    mmu->io[0xFF00 - 0xFF00] = 0xCF;  // P1 (joypad)
    mmu->io[0xFF01 - 0xFF00] = 0x00;  // SB (sArie)
    mmu->io[0xFF02 - 0xFF00] = 0x7E;  // SC (sArie)
    mmu->io[0xFF04 - 0xFF00] = 0xAB;  // DIV
    mmu->io[0xFF05 - 0xFF00] = 0x00;  // TIMA
    mmu->io[0xFF06 - 0xFF00] = 0x00;  // TMA
    mmu->io[0xFF07 - 0xFF00] = 0xF8;  // TAC
    mmu->io[0xFF0F - 0xFF00] = 0xE1;  // IF (bits 5-7 toujours 1)
    mmu->io[0xFF40 - 0xFF00] = 0x91;  // LCDC
    mmu->io[0xFF41 - 0xFF00] = 0x85;  // STAT
    mmu->io[0xFF42 - 0xFF00] = 0x00;  // SCY
    mmu->io[0xFF43 - 0xFF00] = 0x00;  // SCX
    mmu->io[0xFF44 - 0xFF00] = 0x00;  // LY
    mmu->io[0xFF45 - 0xFF00] = 0x00;  // LYC
    mmu->io[0xFF46 - 0xFF00] = 0x00;  // DMA
    mmu->io[0xFF47 - 0xFF00] = 0xFC;  // BGP
    mmu->io[0xFF48 - 0xFF00] = 0xFF;  // OBP0
    mmu->io[0xFF49 - 0xFF00] = 0xFF;  // OBP1
    mmu->io[0xFF4A - 0xFF00] = 0x00;  // WY
    mmu->io[0xFF4B - 0xFF00] = 0x00;  // WX
    mmu->io[0xFFFF - 0xFF00] = 0x00;  // IE (toutes interruptions dAsactivAes)
}
```

**Pourquoi ces valeurs ?** Ce sont les valeurs exactes des registres IO au dAmarrage.

### MAmoire
```c
void mmu_power_up_memory(MMU* mmu) {
    // WRAM initialisA A  0xFF (pas 0x00)
    memset(mmu->wram, 0xFF, sizeof(mmu->wram));
    
    // VRAM initialisA A  0x00
    memset(mmu->vram, 0x00, sizeof(mmu->vram));
    
    // OAM initialisA A  0x00
    memset(mmu->oam, 0x00, sizeof(mmu->oam));
    
    // HRAM initialisA A  0x00
    memset(mmu->hram, 0x00, sizeof(mmu->hram));
}
```

**Pourquoi 0xFF pour WRAM ?** C'est le comportement matAriel. La WRAM est initialisAe A  0xFF, pas 0x00.

## Initialisation du joypad

### Atat du joypad
```c
void joypad_power_up(Joypad* joypad) {
    // Tous les boutons sont relAchAs au dAmarrage
    joypad->right = false;
    joypad->left = false;
    joypad->up = false;
    joypad->down = false;
    joypad->a = false;
    joypad->b = false;
    joypad->start = false;
    joypad->select = false;
    
    // Interruptions dAsactivAes
    joypad->irq_enabled = false;
    joypad->irq_callback = NULL;
    joypad->irq_user_data = NULL;
}
```

**Pourquoi tous relAchAs ?** C'est l'Atat naturel des boutons au dAmarrage.

## SAquence complAte

### Fonction d'initialisation
```c
void emulator_power_up(Emulator* emu) {
    // Initialiser tous les composants
    cpu_power_up(&emu->cpu);
    ppu_power_up(&emu->ppu);
    timer_power_up(&emu->timer);
    mmu_power_up(&emu->mmu);
    joypad_power_up(&emu->joypad);
    
    // Charger le Boot ROM
    mmu_load_boot_rom(&emu->mmu);
    
    // DAmarrer l'exAcution
    emu->running = true;
    emu->cpu.pc = 0x0000;  // DAmarre au Boot ROM
}
```

**Pourquoi cette sAquence ?** L'ordre d'initialisation est important pour Aviter les conflits.

### Boot ROM
```c
void mmu_load_boot_rom(MMU* mmu) {
    // Le Boot ROM est chargA dans la zone 0x0000-0x00FF
    // Il vArifie l'intAgritA de la cartouche et affiche le logo Nintendo
    
    // A la fin, il saute A  0x0100 (dAbut de la ROM du jeu)
    // et dAsactive l'accAs au Boot ROM
}
```

**Pourquoi un Boot ROM ?** Il fournit une sAquence de dAmarrage standardisAe et vArifie l'intAgritA de la cartouche.

## Tests de conformitA

### Test des valeurs de power-up
```c
void test_power_up_values() {
    Emulator emu;
    emulator_power_up(&emu);
    
    // VArifier les valeurs CPU
    assert(emu.cpu.a == 0x01);
    assert(emu.cpu.f == 0xB0);
    assert(emu.cpu.sp == 0xFFFE);
    assert(emu.cpu.pc == 0x0000);
    
    // VArifier les valeurs PPU
    assert(emu.ppu.lcdc == 0x91);
    assert(emu.ppu.stat == 0x85);
    assert(emu.ppu.ly == 0x00);
    
    // VArifier les valeurs Timer
    assert(emu.timer.div == 0xAB);
    assert(emu.timer.tima == 0x00);
    assert(emu.timer.tac == 0xF8);
}
```

**Pourquoi ces tests ?** Ils vArifient que l'initialisation respecte les valeurs exactes de la Game Boy.

## RAfArences Pan Docs

- [Power-Up Sequence](https://gbdev.io/pandocs/Power_Up_Sequence.html)

# Interruptions - Spécifications d'implémentation

Retour: [Index specs](./README.md) | [Architecture](../architecture.md) | [Tests](../testing.md)

## Vue d'ensemble

Le système d'interruptions de la Game Boy permet aux composants matériels de signaler des événements au CPU. C'est un mécanisme essentiel pour la synchronisation et la réactivité.

### Pourquoi des interruptions ?

Les interruptions permettent :
- **Synchronisation** : Coordonner les composants (PPU, timers, joypad)
- **Réactivité** : Réagir immédiatement aux événements
- **Efficacité** : Éviter de poller constamment les périphériques
- **Multitâche** : Simuler l'exécution simultanée de plusieurs tâches

## Types d'interruptions

### Les 5 interruptions de la Game Boy
```c
#define IRQ_VBLANK  0x01  // Fin de frame vidéo
#define IRQ_LCD     0x02  // Événement LCD (HBlank, VBlank, OAM, LYC)
#define IRQ_TIMER   0x04  // Overflow du timer
#define IRQ_SERIAL  0x08  // Transfert série terminé
#define IRQ_JOYPAD  0x10  // Appui sur une touche
```

**Pourquoi ces interruptions ?** Chaque interruption correspond à un événement critique :
- **VBlank** : Fin de frame, temps pour mettre à jour l'écran
- **LCD** : Événements de rendu (lignes, modes)
- **Timer** : Événements temporels
- **Serial** : Communication avec d'autres Game Boy
- **Joypad** : Entrées utilisateur

### Priorité des interruptions
```c
u8 get_highest_priority_irq(u8 if_reg, u8 ie_reg) {
    u8 pending = if_reg & ie_reg;
    
    // Vérifier dans l'ordre de priorité
    if (pending & IRQ_VBLANK) return IRQ_VBLANK;
    if (pending & IRQ_LCD) return IRQ_LCD;
    if (pending & IRQ_TIMER) return IRQ_TIMER;
    if (pending & IRQ_SERIAL) return IRQ_SERIAL;
    if (pending & IRQ_JOYPAD) return IRQ_JOYPAD;
    
    return 0;  // Aucune interruption
}
```

**Pourquoi cette priorité ?** VBlank est critique pour l'affichage, LCD pour le rendu, Timer pour la synchronisation, etc.

## Registres d'interruption

### IF (0xFF0F) - Interrupt Flag
```c
#define IF_REG 0xFF0F

// Chaque bit indique si une interruption est en attente
typedef struct {
    u8 vblank : 1;  // Bit 0
    u8 lcd    : 1;  // Bit 1
    u8 timer  : 1;  // Bit 2
    u8 serial : 1;  // Bit 3
    u8 joypad : 1;  // Bit 4
    u8 unused : 3;  // Bits 5-7 (toujours 1)
} InterruptFlags;
```

**Pourquoi ces bits ?** Chaque bit correspond à une source d'interruption. Les bits 5-7 sont toujours à 1 (comportement matériel).

### IE (0xFFFF) - Interrupt Enable
```c
#define IE_REG 0xFFFF

// Chaque bit active/désactive une interruption
typedef struct {
    u8 vblank : 1;  // Bit 0
    u8 lcd    : 1;  // Bit 1
    u8 timer  : 1;  // Bit 2
    u8 serial : 1;  // Bit 3
    u8 joypad : 1;  // Bit 4
    u8 unused : 3;  // Bits 5-7 (toujours 0)
} InterruptEnable;
```

**Pourquoi un registre séparé ?** Permet d'activer/désactiver les interruptions individuellement sans affecter les flags.

## Gestion des interruptions

### VArification des interruptions
```c
bool check_interrupts(CPU* cpu, MMU* mmu) {
    if (!cpu->ime) return false;  // Interruptions dAsactivAes
    
    u8 if_reg = mmu_read8(mmu, IF_REG);
    u8 ie_reg = mmu_read8(mmu, IE_REG);
    u8 irq = get_highest_priority_irq(if_reg, ie_reg);
    
    if (!irq) return false;
    
    // Traiter l'interruption
    handle_interrupt(cpu, mmu, irq);
    return true;
}
```

**Pourquoi vArifier IME ?** Le CPU peut dAsactiver temporairement les interruptions pour des opArations critiques.

### Traitement d'une interruption
```c
void handle_interrupt(CPU* cpu, MMU* mmu, u8 irq) {
    // DAsactiver les interruptions
    cpu->ime = false;
    
    // Effacer le flag d'interruption
    u8 if_reg = mmu_read8(mmu, IF_REG);
    if_reg &= ~irq;
    mmu_write8(mmu, IF_REG, if_reg);
    
    // Sauvegarder PC sur la pile
    cpu->sp -= 2;
    mmu_write16(mmu, cpu->sp, cpu->pc);
    
    // Sauter à la routine d'interruption
    switch (irq) {
        case IRQ_VBLANK: cpu->pc = 0x40; break;
        case IRQ_LCD:    cpu->pc = 0x48; break;
        case IRQ_TIMER:  cpu->pc = 0x50; break;
        case IRQ_SERIAL: cpu->pc = 0x58; break;
        case IRQ_JOYPAD: cpu->pc = 0x60; break;
    }
}
```

**Pourquoi sauvegarder PC ?** Pour pouvoir revenir à l'instruction interrompue après le traitement.

### Adresses des routines d'interruption
```c
#define IRQ_VBLANK_ADDR 0x40
#define IRQ_LCD_ADDR    0x48
#define IRQ_TIMER_ADDR  0x50
#define IRQ_SERIAL_ADDR 0x58
#define IRQ_JOYPAD_ADDR 0x60
```

**Pourquoi ces adresses ?** Ce sont les adresses fixes dAfinies par la Game Boy. Chaque interruption a sa propre routine.

## Gestion des flags d'interruption

### DAclencher une interruption
```c
void request_interrupt(MMU* mmu, u8 irq) {
    u8 if_reg = mmu_read8(mmu, IF_REG);
    if_reg |= irq;
    mmu_write8(mmu, IF_REG, if_reg);
}
```

**Pourquoi un flag ?** Permet de signaler qu'un AvAnement s'est produit, mAme si les interruptions sont temporairement dAsactivAes.

### Effacer une interruption
```c
void clear_interrupt(MMU* mmu, u8 irq) {
    u8 if_reg = mmu_read8(mmu, IF_REG);
    if_reg &= ~irq;
    mmu_write8(mmu, IF_REG, if_reg);
}
```

**Pourquoi effacer ?** Avite de traiter la mAme interruption plusieurs fois.

## Interruptions spAcifiques

### VBlank (0x40)
```c
void ppu_vblank_interrupt(PPU* ppu, MMU* mmu) {
    if (ppu->ly >= 144) {  // VBlank commence à la ligne 144
        request_interrupt(mmu, IRQ_VBLANK);
    }
}
```

**Pourquoi VBlank ?** C'est le moment où l'écran n'est pas en cours de rendu, idéal pour mettre à jour les graphismes.

### LCD (0x48)
```c
void ppu_lcd_interrupt(PPU* ppu, MMU* mmu) {
    u8 stat = mmu_read8(mmu, 0xFF41);
    
    // VArifier les diffArents types d'interruption LCD
    if (ppu->mode == PPU_MODE_HBLANK && (stat & 0x08)) {
        request_interrupt(mmu, IRQ_LCD);
    }
    if (ppu->mode == PPU_MODE_VBLANK && (stat & 0x10)) {
        request_interrupt(mmu, IRQ_LCD);
    }
    if (ppu->mode == PPU_MODE_OAM_SEARCH && (stat & 0x20)) {
        request_interrupt(mmu, IRQ_LCD);
    }
    if (ppu->ly == mmu_read8(mmu, 0xFF45) && (stat & 0x40)) {
        request_interrupt(mmu, IRQ_LCD);
    }
}
```

**Pourquoi LCD ?** Permet de synchroniser avec les phases de rendu pour des effets spAciaux.

### Timer (0x50)
```c
void timer_interrupt(Timer* timer, MMU* mmu) {
    if (timer->overflow) {
        request_interrupt(mmu, IRQ_TIMER);
        timer->overflow = false;
    }
}
```

**Pourquoi Timer ?** Pour les AvAnements temporels (animations, dAlais, rythme de jeu).

### Serial (0x58)
```c
void serial_interrupt(MMU* mmu) {
    // Déclencher quand le transfert série est terminé
    request_interrupt(mmu, IRQ_SERIAL);
}
```

**Pourquoi Serial ?** Pour la communication entre Game Boy (jeux multijoueurs).

### Joypad (0x60)
```c
void joypad_interrupt(Joypad* joypad, MMU* mmu) {
    if (joypad->irq_enabled && joypad->button_pressed) {
        request_interrupt(mmu, IRQ_JOYPAD);
    }
}
```

**Pourquoi Joypad ?** Pour rAagir immAdiatement aux entrAes utilisateur.

## Gestion des interruptions en cours

### Interruptions imbriquAes
```c
void handle_interrupt(CPU* cpu, MMU* mmu, u8 irq) {
    // DAsactiver les interruptions
    cpu->ime = false;
    
    // ... traiter l'interruption ...
    
    // RAactiver les interruptions (aprAs l'instruction suivante)
    cpu->ime_pending = true;
}
```

**Pourquoi dAsactiver ?** Avite les interruptions imbriquAes qui pourraient corrompre l'Atat du systAme.

### DAlai d'activation (EI)
```c
void op_ei(CPU* cpu, MMU* mmu) {
    // EI active les interruptions aprAs l'instruction suivante
    cpu->ime_pending = true;
}

void cpu_step(CPU* cpu, MMU* mmu) {
    // VArifier si on doit activer les interruptions
    if (cpu->ime_pending) {
        cpu->ime = true;
        cpu->ime_pending = false;
    }
    
    // ... exAcuter l'instruction ...
}
```

**Pourquoi un délai ?** Sécurité. Si les interruptions étaient activées immédiatement, l'instruction en cours pourrait être interrompue de manière inattendue.

## Initialisation

```c
void interrupt_init(InterruptManager* im) {
    memset(im, 0, sizeof(InterruptManager));
    
    // Valeurs de power-up
    im->if_reg = 0xE1;  // Bits 5-7 toujours à 1
    im->ie_reg = 0x00;  // Toutes les interruptions dAsactivAes
}
```

**Pourquoi ces valeurs ?** Ce sont les valeurs exactes de la Game Boy au démarrage, importantes pour la compatibilité.

## Tests de conformité

### Test de priorité
```c
void test_interrupt_priority() {
    CPU cpu;
    MMU mmu;
    
    // Activer plusieurs interruptions
    mmu_write8(&mmu, IF_REG, IRQ_TIMER | IRQ_VBLANK);
    mmu_write8(&mmu, IE_REG, IRQ_TIMER | IRQ_VBLANK);
    
    // VBlank doit avoir la priorité
    u8 irq = get_highest_priority_irq(mmu_read8(&mmu, IF_REG), mmu_read8(&mmu, IE_REG));
    assert(irq == IRQ_VBLANK);
}
```

**Pourquoi ces tests ?** Ils vArifient que le comportement des interruptions est conforme aux spAcifications.

## RAfArences Pan Docs

- [Interrupts](https://gbdev.io/pandocs/Interrupts.html)
- [Interrupt Sources](https://gbdev.io/pandocs/Interrupt_Sources.html)
- [HALT](https://gbdev.io/pandocs/HALT.html)

#include "interrupt.h"
#include <stdio.h>

// Initialisation du gestionnaire d'interruptions
void interrupt_init(InterruptManager* im) {
    im->ie = 0x00;
    im->if_reg = 0xE1; // Valeur par défaut selon Pan Docs
    im->pending_interrupts = 0x00;
}

// Reset du gestionnaire d'interruptions
void interrupt_reset(InterruptManager* im) {
    im->ie = 0x00;
    im->if_reg = 0xE1;
    im->pending_interrupts = 0x00;
}

// Écriture dans le registre IE
void interrupt_write_ie(InterruptManager* im, u8 value) {
    im->ie = value;
}

// Lecture du registre IE
u8 interrupt_read_ie(InterruptManager* im) {
    return im->ie;
}

// Écriture dans le registre IF
void interrupt_write_if(InterruptManager* im, u8 value) {
    im->if_reg = value;
}

// Lecture du registre IF
u8 interrupt_read_if(InterruptManager* im) {
    return im->if_reg;
}

// Demander une interruption
void interrupt_request(InterruptManager* im, u8 interrupt) {
    im->if_reg |= interrupt;
    im->pending_interrupts |= interrupt;
}

// Effacer une interruption
void interrupt_clear(InterruptManager* im, u8 interrupt) {
    im->if_reg &= ~interrupt;
    im->pending_interrupts &= ~interrupt;
}

// Obtenir les interruptions en attente
u8 interrupt_get_pending(InterruptManager* im) {
    return im->pending_interrupts;
}

// Vérifier s'il y a des interruptions en attente
bool interrupt_has_pending(InterruptManager* im) {
    return (im->if_reg & im->ie) != 0;
}

// Obtenir l'interruption de priorité la plus haute
u8 interrupt_get_highest_priority(InterruptManager* im) {
    u8 active_interrupts = im->if_reg & im->ie;
    
    // Vérifier par ordre de priorité (bit 0 = priorité la plus haute)
    for (int i = 0; i < 5; i++) {
        u8 interrupt = 1 << i;
        if (active_interrupts & interrupt) {
            return interrupt;
        }
    }
    
    return 0; // Aucune interruption active
}

// Obtenir le nom d'une interruption (pour debug)
const char* interrupt_get_name(u8 interrupt) {
    switch (interrupt) {
        case VBLANK_INT:   return "VBlank";
        case LCD_STAT_INT: return "LCD STAT";
        case TIMER_INT:    return "Timer";
        case SERIAL_INT:   return "Serial";
        case JOYPAD_INT:   return "Joypad";
        default:           return "Unknown";
    }
}

// Obtenir l'adresse du gestionnaire d'interruption
u16 interrupt_get_handler_address(u8 interrupt) {
    switch (interrupt) {
        case VBLANK_INT:   return INT_VBLANK_ADDR;
        case LCD_STAT_INT: return INT_LCD_STAT_ADDR;
        case TIMER_INT:    return INT_TIMER_ADDR;
        case SERIAL_INT:   return INT_SERIAL_ADDR;
        case JOYPAD_INT:   return INT_JOYPAD_ADDR;
        default:           return 0x0000;
    }
}

// Routine de service d'interruption
void interrupt_service_routine(CPU* cpu, MMU* mmu, u8 interrupt) {
    // 1. Désactiver les interruptions (IME = 0)
    cpu->ime = false;
    
    // 2. Effacer le flag d'interruption
    u8 if_reg = mmu_read8(mmu, IF_REG);
    mmu_write8(mmu, IF_REG, if_reg & ~interrupt);
    
    // 3. Attendre 2 cycles (2 M-cycles)
    // (géré par l'appelant)
    
    // 4. Empiler PC sur la pile (2 M-cycles)
    cpu->sp -= 2;
    mmu_write16(mmu, cpu->sp, cpu->pc);
    
    // 5. Définir PC à l'adresse du gestionnaire (1 M-cycle)
    u16 handler_addr = interrupt_get_handler_address(interrupt);
    cpu->pc = handler_addr;
    
    // 6. Total : 5 M-cycles pour une interruption
}

// Traitement principal des interruptions
u8 interrupt_handle(InterruptManager* im, CPU* cpu, MMU* mmu) {
    // Vérifier si les interruptions sont activées
    if (!cpu->ime) {
        return 0; // Aucune interruption traitée
    }
    
    // Obtenir l'interruption de priorité la plus haute
    u8 interrupt = interrupt_get_highest_priority(im);
    if (interrupt == 0) {
        return 0; // Aucune interruption active
    }
    
    // Traiter l'interruption
    interrupt_service_routine(cpu, mmu, interrupt);
    
    // Effacer l'interruption des interruptions en attente
    im->pending_interrupts &= ~interrupt;
    
    return interrupt;
}

// Fonction utilitaire pour ajouter des interruptions depuis d'autres composants
void interrupt_request_from_component(InterruptManager* im, u8 interrupt) {
    interrupt_request(im, interrupt);
}

// Fonction utilitaire pour vérifier si une interruption spécifique est active
bool interrupt_is_active(InterruptManager* im, u8 interrupt) {
    return (im->if_reg & interrupt) != 0;
}

// Fonction utilitaire pour obtenir le statut de toutes les interruptions
void interrupt_get_status(InterruptManager* im, u8* ie, u8* if_reg, u8* pending) {
    *ie = im->ie;
    *if_reg = im->if_reg;
    *pending = im->pending_interrupts;
}

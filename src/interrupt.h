#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "common.h"
#include "cpu.h"
#include "mmu.h"

// Types d'interruptions (selon Pan Docs)
#define VBLANK_INT   0x01   // INT $40 - VBlank interrupt (priorité la plus haute)
#define LCD_STAT_INT 0x02   // INT $48 - STAT interrupt  
#define TIMER_INT    0x04   // INT $50 - Timer interrupt
#define SERIAL_INT   0x08   // INT $58 - Serial interrupt
#define JOYPAD_INT   0x10   // INT $60 - Joypad interrupt (priorité la plus basse)

// Adresses des gestionnaires d'interruption
#define INT_VBLANK_ADDR   0x0040
#define INT_LCD_STAT_ADDR 0x0048
#define INT_TIMER_ADDR    0x0050
#define INT_SERIAL_ADDR   0x0058
#define INT_JOYPAD_ADDR   0x0060

// Structure de gestion des interruptions
typedef struct {
    u8 ie;  // Interrupt Enable register (0xFFFF)
    u8 if_reg;  // Interrupt Flag register (0xFF0F)
    u8 pending_interrupts;  // Interruptions en attente
} InterruptManager;

// Fonctions de gestion des interruptions
void interrupt_init(InterruptManager* im);
void interrupt_reset(InterruptManager* im);
void interrupt_write_ie(InterruptManager* im, u8 value);
u8 interrupt_read_ie(InterruptManager* im);
void interrupt_write_if(InterruptManager* im, u8 value);
u8 interrupt_read_if(InterruptManager* im);

// Gestion des interruptions
void interrupt_request(InterruptManager* im, u8 interrupt);
void interrupt_clear(InterruptManager* im, u8 interrupt);
u8 interrupt_get_pending(InterruptManager* im);
bool interrupt_has_pending(InterruptManager* im);

// Traitement des interruptions
u8 interrupt_handle(InterruptManager* im, CPU* cpu, MMU* mmu);
void interrupt_service_routine(CPU* cpu, MMU* mmu, u8 interrupt);

// Utilitaires
u8 interrupt_get_highest_priority(InterruptManager* im);
const char* interrupt_get_name(u8 interrupt);
u16 interrupt_get_handler_address(u8 interrupt);

#endif // INTERRUPT_H

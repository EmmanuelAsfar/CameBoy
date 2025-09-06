#ifndef TIMER_H
#define TIMER_H

#include "common.h"

// Structure des timers
typedef struct {
    u8 div;   // Divider register (0xFF04)
    u8 tima;  // Timer counter (0xFF05)
    u8 tma;   // Timer modulo (0xFF06)
    u8 tac;   // Timer control (0xFF07)
    
    u32 div_cycles;   // Compteur pour DIV
    u32 tima_cycles;  // Compteur pour TIMA
    u32 tima_period;  // Période actuelle de TIMA
    bool interrupt_pending;  // Interruption timer en attente
} Timer;

// Fonctions timer
void timer_init(Timer* timer);
void timer_reset(Timer* timer);
void timer_tick(Timer* timer, u8 cycles);
void timer_write(Timer* timer, u16 address, u8 value);
u8 timer_read(Timer* timer, u16 address);
u8 timer_get_interrupts(Timer* timer);  // Récupère les interruptions timer

// Calcul de la période TIMA
u32 timer_get_tima_period(u8 tac);

#endif // TIMER_H

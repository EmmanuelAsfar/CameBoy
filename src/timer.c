#include "timer.h"

// Initialisation du timer
void timer_init(Timer* timer) {
    memset(timer, 0, sizeof(Timer));
    timer_reset(timer);
}

// Reset du timer
void timer_reset(Timer* timer) {
    timer->div = 0;
    timer->tima = 0;
    timer->tma = 0;
    timer->tac = 0;
    timer->div_cycles = 0;
    timer->tima_cycles = 0;
    timer->tima_period = 0;
    timer->interrupt_pending = false;
}

// Tick du timer
void timer_tick(Timer* timer, u8 cycles) {
    // DIV timer (incrémente toutes les 256 cycles)
    timer->div_cycles += cycles;
    if (timer->div_cycles >= 256) {
        timer->div_cycles -= 256;
        timer->div++;
    }
    
    // TIMA timer
    if (timer->tac & 0x04) {  // Timer enabled
        timer->tima_cycles += cycles;
        if (timer->tima_cycles >= timer->tima_period) {
            timer->tima_cycles -= timer->tima_period;
            timer->tima++;
            
            if (timer->tima == 0) {
                // Overflow - recharger avec TMA et déclencher interrupt
                timer->tima = timer->tma;
                timer->interrupt_pending = true;
            }
        }
    }
}

// Écriture dans les registres timer
void timer_write(Timer* timer, u16 address, u8 value) {
    switch (address) {
        case DIV_REG:
            // DIV se remet à 0 quand on écrit dedans
            timer->div = 0;
            timer->div_cycles = 0;
            break;
            
        case TIMA_REG:
            timer->tima = value;
            break;
            
        case TMA_REG:
            timer->tma = value;
            break;
            
        case TAC_REG:
            timer->tac = value;
            timer->tima_period = timer_get_tima_period(value);
            break;
    }
}

// Lecture des registres timer
u8 timer_read(Timer* timer, u16 address) {
    switch (address) {
        case DIV_REG:  return timer->div;
        case TIMA_REG: return timer->tima;
        case TMA_REG:  return timer->tma;
        case TAC_REG:  return timer->tac;
        default:       return 0xFF;
    }
}

// Récupère les interruptions timer
u8 timer_get_interrupts(Timer* timer) {
    if (timer->interrupt_pending) {
        timer->interrupt_pending = false;
        return 0x04;  // TIMER_INT
    }
    return 0;
}

// Calcul de la période TIMA
u32 timer_get_tima_period(u8 tac) {
    if (!(tac & 0x04)) return 0;  // Timer disabled
    
    u8 clock_select = tac & 0x03;
    switch (clock_select) {
        case 0: return 1024;  // 4096 Hz
        case 1: return 16;    // 262144 Hz
        case 2: return 64;    // 65536 Hz
        case 3: return 256;   // 16384 Hz
        default: return 0;
    }
}

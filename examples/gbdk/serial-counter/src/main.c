// GBDK: compteur série 0..10 via SB/SC (polling)
#include <gb/gb.h>
#include <gb/hardware.h>

static void serial_putc(unsigned char c) {
    SB_REG = c;               // Donnée
    SC_REG = 0x81;            // Start + horloge interne
    while (SC_REG & 0x80) {   // Attendre fin transfert
        // busy
    }
}

static void serial_puts(const char* s) {
    while (*s) serial_putc((unsigned char)*s++);
}

void main(void) {
    disable_interrupts();
    DISPLAY_OFF;

    for (unsigned char i = 0; i <= 10; i++) {
        serial_puts("TOTO: ");
        serial_putc('0' + (i % 10));
        serial_putc('\n');

        // petite pause (~quelques frames)
        for (unsigned char d = 0; d < 30; d++) wait_vbl_done();
    }

    // boucle passive
    while (1) wait_vbl_done();
}


// GBDK joypad -> serial: envoie l'état des boutons via SB/SC
#include <gb/gb.h>
#include <gb/hardware.h>
#include <stdio.h>

static void serial_write(uint8_t v) {
    SB_REG = v;
    SC_REG = 0x81; // start, internal clock
    while (SC_REG & 0x80) {
        // wait
    }
}

static void serial_puts(const char *s) {
    while (*s) serial_write((uint8_t)*s++);
}

static void serial_putc(char c) { serial_write((uint8_t)c); }

void main(void) {
    uint8_t last = 0xFF;
    serial_puts("Joypad Serial (GBDK)\n");

    while (1) {
        uint8_t jp = joypad(); // J_* bits set when pressed

        if (jp != last) {
            last = jp;
            serial_puts("Joypad: ");

            // Directions: Down, Up, Left, Right
            serial_putc((jp & J_DOWN) ? 'D' : '-');
            serial_putc((jp & J_UP) ? 'U' : '-');
            serial_putc((jp & J_LEFT) ? 'L' : '-');
            serial_putc((jp & J_RIGHT) ? 'R' : '-');
            serial_putc(' ');

            // Actions: Start, Select, B, A
            serial_putc((jp & J_START) ? 'S' : '-');
            serial_putc((jp & J_SELECT) ? 's' : '-');
            serial_putc((jp & J_B) ? 'B' : '-');
            serial_putc((jp & J_A) ? 'A' : '-');
            serial_putc('\n');
        }

        // petit délai / vsync
        wait_vbl_done();
    }
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static void write_header(uint8_t *rom, const char *title) {
    memset(rom, 0x00, 0x150);
    rom[0x0100] = 0xC3; rom[0x0101] = 0x50; rom[0x0102] = 0x01; // JP 0150h
    static const uint8_t nintendo_logo[48] = {
        0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
        0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,
        0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
        0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,
        0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
        0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E
    };
    memcpy(&rom[0x0104], nintendo_logo, 48);
    char buf[16]; memset(buf, ' ', sizeof(buf));
    size_t len = strlen(title); if (len > 16) len = 16; memcpy(buf, title, len);
    memcpy(&rom[0x0134], buf, 16);
    rom[0x0147] = 0x00; rom[0x0148] = 0x00; rom[0x0149] = 0x00;
    uint8_t x = 0; for (int i = 0x0134; i <= 0x014C; i++) x = x - rom[i] - 1; rom[0x014D] = x;
}

int main(void) {
    uint8_t *rom = calloc(1, 32768);
    if (!rom) { fprintf(stderr, "Alloc ROM toto echouee\n"); return 1; }
    write_header(rom, "TOTO COUNTER   ");

    // Messages 0..10
    const char *messages[11] = {
        "TOTO: 0\n","TOTO: 1\n","TOTO: 2\n","TOTO: 3\n","TOTO: 4\n",
        "TOTO: 5\n","TOTO: 6\n","TOTO: 7\n","TOTO: 8\n","TOTO: 9\n","TOTO: 10\n"
    };
    // Ecrire en ROM a partir de 0x0200 par pas de 16
    uint16_t msg_addr = 0x0200;
    for (int i = 0; i < 11; i++) memcpy(&rom[msg_addr + i*16], messages[i], strlen(messages[i]));

    uint16_t p = 0x0150; uint8_t *c = &rom[p];

    // Compteur dans registre C = 0
    *c++ = 0x0E; *c++ = 0x00;                 // LD C,0

    // Main loop start
    uint16_t main_loop = (uint16_t)(p + (c - &rom[p]));

    // HL = 0x0200 + (C * 16)
    *c++ = 0x79;                               // LD A,C
    *c++ = 0x6F;                               // LD L,A
    *c++ = 0x26; *c++ = 0x00;                  // LD H,0
    *c++ = 0x29; *c++ = 0x29; *c++ = 0x29; *c++ = 0x29; // HL *= 16
    *c++ = 0x11; *c++ = 0x00; *c++ = 0x02;     // LD DE,0200h
    *c++ = 0x19;                               // ADD HL,DE

    // Send up to 8 chars or until 0x00
    *c++ = 0x0E; *c++ = 8;                     // LD C,8
    uint16_t send_loop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x7E;                               // LD A,(HL)
    *c++ = 0xFE; *c++ = 0x00;                  // CP 0
    *c++ = 0x28; *c++ = 0x08;                  // JR Z, +8 (skip send)
    *c++ = 0xE0; *c++ = 0x01;                  // LDH (FF01),A
    *c++ = 0x3E; *c++ = 0x81;                  // LD A,81h
    *c++ = 0xE0; *c++ = 0x02;                  // LDH (FF02),A
    *c++ = 0x23;                               // INC HL
    *c++ = 0x0D;                               // DEC C
    int8_t rel = (int8_t)(send_loop - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel; // JR NZ,send_loop

    // counter++ (C)
    *c++ = 0x0C;                               // INC C

    // if counter >= 11 -> HALT else continue
    *c++ = 0x3E; *c++ = 11;                     // LD A,11
    *c++ = 0xB9;                                // CP C
    *c++ = 0x30; *c++ = 0x02;                   // JR NC, +2 (to HALT)
    *c++ = 0x18; *c++ = 0x02;                   // JR +2 (skip HALT)
    *c++ = 0x76;                                 // HALT

    // delay
    *c++ = 0x06; *c++ = 0xFF;                    // LD B,FFh
    uint16_t pause = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x05;                                 // DEC B
    rel = (int8_t)(pause - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel; // JR NZ,pause

    // loop: JP main_loop (absolute) pour eviter limite +-128 octets de JR
    *c++ = 0xC3; *c++ = (uint8_t)(main_loop & 0xFF); *c++ = (uint8_t)(main_loop >> 8);

    FILE *f = fopen("../toto_counter.gb", "wb");
    if (!f) { fprintf(stderr, "Creer toto_counter.gb echoue\n"); free(rom); return 1; }
    fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    return 0;
}

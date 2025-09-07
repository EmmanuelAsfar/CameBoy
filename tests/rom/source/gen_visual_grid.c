#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static void write_header(uint8_t *rom, const char *title) {
    memset(rom, 0x00, 0x150);
    rom[0x0100] = 0xC3; rom[0x0101] = 0x50; rom[0x0102] = 0x01;
    const uint8_t nintendo_logo[48] = {
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
    if (!rom) { fprintf(stderr, "Alloc ROM visual échouée\n"); return 1; }
    write_header(rom, "VISUAL GRID  ");

    const char *msg = "START : OK\n";
    memcpy(&rom[0x0200], msg, strlen(msg));

    uint16_t p = 0x0150; uint8_t *c = &rom[p];
    // Send serial message
    *c++ = 0x21; *c++ = 0x00; *c++ = 0x02; // LD HL,0200h
    *c++ = 0x06; *c++ = (uint8_t)strlen(msg); // LD B,len
    uint16_t loop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x7E; *c++ = 0xE0; *c++ = 0x01; *c++ = 0x3E; *c++ = 0x81; *c++ = 0xE0; *c++ = 0x02; // send byte
    *c++ = 0x23; *c++ = 0x05; // INC HL, DEC B
    int8_t rel = (int8_t)(loop - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel; // JR NZ

    // LCD off during VRAM writes, set palette
    *c++ = 0x3E; *c++ = 0x00; *c++ = 0xE0; *c++ = 0x40; // LCDC off
    *c++ = 0x3E; *c++ = 0xE4; *c++ = 0xE0; *c++ = 0x47; // BGP

    // Tiles 0 and 1: checker rows differ
    *c++ = 0x21; *c++ = 0x00; *c++ = 0x80; // HL=8000
    // tile0
    *c++ = 0x0E; *c++ = 0x08; // C=8 rows
    *c++ = 0x3E; *c++ = 0xAA; uint16_t t0 = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x77; *c++ = 0x23; *c++ = 0xAF; *c++ = 0x77; *c++ = 0x23; *c++ = 0x0F; *c++ = 0x0D; // RRCA (0xAA<->0x55)
    rel = (int8_t)(t0 - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel;
    // tile1
    *c++ = 0x0E; *c++ = 0x08; *c++ = 0x3E; *c++ = 0x55; uint16_t t1 = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x77; *c++ = 0x23; *c++ = 0xAF; *c++ = 0x77; *c++ = 0x23; *c++ = 0x0F; *c++ = 0x0D;
    rel = (int8_t)(t1 - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel;

    // Fill BG map alternate 0/1 for 32x18
    *c++ = 0x11; *c++ = 0x00; *c++ = 0x98; // DE=9800
    *c++ = 0x06; *c++ = 18; uint16_t rlp = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x0E; *c++ = 32; *c++ = 0x3E; *c++ = 0x00; uint16_t clp = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x12; *c++ = 0x13; *c++ = 0xEE; *c++ = 0x01; *c++ = 0x0D; // write,xor1,dec
    rel = (int8_t)(clp - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel;
    *c++ = 0xEE; *c++ = 0x01; *c++ = 0x05; // toggle start, dec rows
    rel = (int8_t)(rlp - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel;

    // LCD on
    *c++ = 0x3E; *c++ = 0x91; *c++ = 0xE0; *c++ = 0x40;
    *c++ = 0x18; *c++ = 0xFE; // loop forever

    FILE *f = fopen("../visual_grid.gb", "wb");
    if (!f) { fprintf(stderr, "Créer visual_grid.gb échoué\n"); free(rom); return 1; }
    fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    return 0;
}



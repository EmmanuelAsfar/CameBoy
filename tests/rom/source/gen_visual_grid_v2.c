// Générateur ROM: visual_grid_v2 – damier de carreaux 8x8 (plein/vide)
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
    rom[0x0147] = 0x00; // ROM Only
    rom[0x0148] = 0x00; // 32 KB
    rom[0x0149] = 0x00; // no RAM
    uint8_t x = 0; for (int i = 0x0134; i <= 0x014C; i++) x = x - rom[i] - 1; rom[0x014D] = x;
}

int main(void) {
    uint8_t *rom = calloc(1, 32768);
    if (!rom) { fprintf(stderr, "Alloc ROM visual_grid_v2 échouée\n"); return 1; }
    write_header(rom, "VIS GRID V2     ");

    // Préparer tiles: 0=vide (00/00), 1=plein (FF/FF) => couleur index 3
    uint8_t tiles[32];
    memset(tiles, 0x00, 16);           // tile 0 vide
    for (int i = 0; i < 8; i++) {      // tile 1 plein (8 lignes)
        tiles[16 + i*2 + 0] = 0xFF;    // plan bas
        tiles[16 + i*2 + 1] = 0xFF;    // plan haut
    }

    // Construire la BG map 32x18: alternance par tuile (damier 8x8)
    uint8_t map[32*18];
    for (int y = 0; y < 18; y++) {
        for (int x = 0; x < 32; x++) {
            map[y*32 + x] = (uint8_t)(((x + y) & 1) ? 1 : 0);
        }
    }

    // Code à 0x0150: LCD off, palette, copie tiles et map, LCD on, boucle
    uint16_t p = 0x0150; uint8_t *c = &rom[p];
    *c++ = 0x3E; *c++ = 0x00; *c++ = 0xE0; *c++ = 0x40; // LCDC off
    *c++ = 0x3E; *c++ = 0xE4; *c++ = 0xE0; *c++ = 0x47; // BGP

    // Copier tiles (32 octets) -> 8000h
    uint16_t tiles_addr = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x21; *c++ = (uint8_t)(tiles_addr & 0xFF); *c++ = (uint8_t)(tiles_addr >> 8); // HL=tiles
    *c++ = 0x11; *c++ = 0x00; *c++ = 0x80; // DE=8000
    *c++ = 0x01; *c++ = 32; *c++ = 0x00;   // BC=32
    uint16_t loop1 = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x78; *c++ = 0xB1; *c++ = 0x28; *c++ = 0x08; // LD A,B | OR C | JR Z,+8
    *c++ = 0x7E; *c++ = 0x12; *c++ = 0x23; *c++ = 0x13; // LD A,(HL) | LD (DE),A | INC HL | INC DE
    *c++ = 0x0B;                                        // DEC BC
    int8_t rel1 = (int8_t)(loop1 - (uint16_t)(p + (c - &rom[p]) + 2)); *c++ = 0x18; *c++ = (uint8_t)rel1; // JR loop1

    // Copier map (576 octets) -> 9800h
    uint16_t map_addr = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x21; *c++ = (uint8_t)(map_addr & 0xFF); *c++ = (uint8_t)(map_addr >> 8); // HL=map
    *c++ = 0x11; *c++ = 0x00; *c++ = 0x98; // DE=9800
    *c++ = 0x01; *c++ = (uint8_t)(576 & 0xFF); *c++ = (uint8_t)(576 >> 8); // BC=576
    uint16_t loop2 = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x78; *c++ = 0xB1; *c++ = 0x28; *c++ = 0x08;
    *c++ = 0x7E; *c++ = 0x12; *c++ = 0x23; *c++ = 0x13;
    *c++ = 0x0B;
    int8_t rel2 = (int8_t)(loop2 - (uint16_t)(p + (c - &rom[p]) + 2)); *c++ = 0x18; *c++ = (uint8_t)rel2;

    // LCD ON et boucle
    *c++ = 0x3E; *c++ = 0x91; *c++ = 0xE0; *c++ = 0x40; // LCDC on
    *c++ = 0x18; *c++ = 0xFE; // JR -0

    // Placer les données en ROM
    memcpy(&rom[tiles_addr], tiles, sizeof(tiles));
    memcpy(&rom[map_addr], map, sizeof(map));

    FILE *f = fopen("../visual_grid_v2.gb", "wb");
    if (!f) { fprintf(stderr, "Créer visual_grid_v2.gb échoué\n"); free(rom); return 1; }
    fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    return 0;
}


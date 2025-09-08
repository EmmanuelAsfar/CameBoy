// Générateur de ROM: cercle au centre (rayon 30px)
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
    rom[0x0148] = 0x00; // 32KB
    rom[0x0149] = 0x00; // no RAM
    uint8_t x = 0; for (int i = 0x0134; i <= 0x014C; i++) x = x - rom[i] - 1; rom[0x014D] = x; // header checksum
}

typedef struct { uint8_t bytes[16]; int used; } Tile;

int main(void) {
    uint8_t *rom = calloc(1, 32768);
    if (!rom) { fprintf(stderr, "Alloc ROM circle échouée\n"); return 1; }
    write_header(rom, "CIRCLE R30       ");

    // Data for tiles and map we'll embed in ROM after code
    Tile tiles[256]; memset(tiles, 0, sizeof(tiles));
    uint8_t map[32*18]; memset(map, 0, sizeof(map));
    uint8_t tile_index_map[20][18]; memset(tile_index_map, 0, sizeof(tile_index_map));
    int next_idx = 1; // 0 = blank

    // Helpers in C99
    int (*get_or_alloc)(int,int) = NULL;
    void (*set_pixel)(int,int) = NULL;

    int get_or_alloc_impl(int tx, int ty) {
        if (tx < 0 || tx >= 20 || ty < 0 || ty >= 18) return 0;
        int idx = tile_index_map[tx][ty];
        if (idx == 0) {
            if (next_idx >= 256) return 0; // safety
            idx = next_idx++;
            tile_index_map[tx][ty] = (uint8_t)idx;
            tiles[idx].used = 1;
        }
        return idx;
    }
    void set_pixel_impl(int px, int py) {
        if (px < 0 || px >= 160 || py < 0 || py >= 144) return;
        int tx = px / 8, ty = py / 8; // tile coords
        int idx = get_or_alloc_impl(tx, ty);
        if (idx <= 0) return;
        int lx = px % 8, ly = py % 8;
        uint8_t mask = (uint8_t)(0x80u >> lx);
        tiles[idx].bytes[ly*2 + 0] |= mask; // plane 0
        tiles[idx].bytes[ly*2 + 1] |= mask; // plane 1 (color index 3)
    }
    get_or_alloc = get_or_alloc_impl;
    set_pixel = set_pixel_impl;

    // Plot circle outline with Midpoint algorithm
    const int cx = 80, cy = 72, r = 30;
    int x = r, y = 0; int d = 1 - r;
    while (y <= x) {
        set_pixel(cx + x, cy + y);
        set_pixel(cx + y, cy + x);
        set_pixel(cx - y, cy + x);
        set_pixel(cx - x, cy + y);
        set_pixel(cx - x, cy - y);
        set_pixel(cx - y, cy - x);
        set_pixel(cx + y, cy - x);
        set_pixel(cx + x, cy - y);
        y++;
        if (d <= 0) {
            d += 2*y + 1;
        } else {
            x--; d += 2*(y - x) + 1;
        }
    }

    // Build map: 32x18, use allocated indices in left 20 columns
    for (int ty = 0; ty < 18; ty++) {
        for (int tx = 0; tx < 32; tx++) {
            uint8_t idx = 0;
            if (tx < 20) idx = tile_index_map[tx][ty];
            map[ty*32 + tx] = idx;
        }
    }

    // Prepare code at 0x0150
    uint16_t p = 0x0150; uint8_t *c = &rom[p];

    // LCD off, set BGP (E4)
    *c++ = 0x3E; *c++ = 0x00; *c++ = 0xE0; *c++ = 0x40; // LCDC off
    *c++ = 0x3E; *c++ = 0xE4; *c++ = 0xE0; *c++ = 0x47; // BGP

    // Copy tiles from ROM to VRAM (8000h)
    uint16_t tiles_addr = (uint16_t)(p + (c - &rom[p]));
    // We'll place tiles here after code; For now, emit LD HL,tiles_addr
    *c++ = 0x21; *c++ = (uint8_t)(tiles_addr & 0xFF); *c++ = (uint8_t)(tiles_addr >> 8); // LD HL, tiles_addr
    *c++ = 0x11; *c++ = 0x00; *c++ = 0x80; // LD DE,8000h
    uint16_t tiles_size = (uint16_t)((next_idx) * 16); // include tile 0 (blank) for simplicity
    *c++ = 0x01; *c++ = (uint8_t)(tiles_size & 0xFF); *c++ = (uint8_t)(tiles_size >> 8); // LD BC, size
    // Copy loop
    uint16_t copy_loop1 = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x78;               // LD A,B
    *c++ = 0xB1;               // OR C
    *c++ = 0x28; *c++ = 0x08;  // JR Z, +8 (to after loop)
    *c++ = 0x7E;               // LD A,(HL)
    *c++ = 0x12;               // LD (DE),A
    *c++ = 0x23;               // INC HL
    *c++ = 0x13;               // INC DE
    *c++ = 0x0B;               // DEC BC
    int8_t rel1 = (int8_t)(copy_loop1 - (uint16_t)(p + (c - &rom[p]) + 2));
    *c++ = 0x18; *c++ = (uint8_t)rel1; // JR copy_loop1
    // continue here

    // Copy map to 9800h
    uint16_t map_addr = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x21; *c++ = (uint8_t)(map_addr & 0xFF); *c++ = (uint8_t)(map_addr >> 8); // LD HL,map_addr
    *c++ = 0x11; *c++ = 0x00; *c++ = 0x98; // LD DE,9800h
    *c++ = 0x01; *c++ = (uint8_t)(576 & 0xFF); *c++ = (uint8_t)(576 >> 8); // LD BC, 32*18
    uint16_t copy_loop2 = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x78;               // LD A,B
    *c++ = 0xB1;               // OR C
    *c++ = 0x28; *c++ = 0x08;  // JR Z, +8
    *c++ = 0x7E;               // LD A,(HL)
    *c++ = 0x12;               // LD (DE),A
    *c++ = 0x23;               // INC HL
    *c++ = 0x13;               // INC DE
    *c++ = 0x0B;               // DEC BC
    int8_t rel2 = (int8_t)(copy_loop2 - (uint16_t)(p + (c - &rom[p]) + 2));
    *c++ = 0x18; *c++ = (uint8_t)rel2; // JR copy_loop2

    // LCD on and loop forever
    *c++ = 0x3E; *c++ = 0x91; *c++ = 0xE0; *c++ = 0x40; // LCDC on
    *c++ = 0x18; *c++ = 0xFE; // JR -0 (inf)

    // Place tiles data at tiles_addr
    uint8_t *tiles_ptr = &rom[tiles_addr];
    // tile 0 = blank
    memset(tiles_ptr, 0x00, 16);
    // tiles 1..next_idx-1
    for (int i = 1; i < next_idx; i++) {
        memcpy(tiles_ptr + i*16, tiles[i].bytes, 16);
    }

    // Place map data at map_addr
    memcpy(&rom[map_addr], map, sizeof(map));

    // Write ROM file
    FILE *f = fopen("../circle_r30.gb", "wb");
    if (!f) { fprintf(stderr, "Créer circle_r30.gb échoué\n"); free(rom); return 1; }
    fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    return 0;
}

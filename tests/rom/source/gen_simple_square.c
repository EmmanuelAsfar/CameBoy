#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static void write_header(uint8_t *rom, const char *title) {
    memset(rom, 0x00, 0x150);
    rom[0x0100] = 0xC3; // JP nn
    rom[0x0101] = 0x50; // low
    rom[0x0102] = 0x01; // high

    const uint8_t nintendo_logo[48] = {
        0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
        0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,
        0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
        0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,
        0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
        0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E
    };
    memcpy(&rom[0x0104], nintendo_logo, 48);

    char buf[16];
    memset(buf, ' ', sizeof(buf));
    size_t len = strlen(title);
    if (len > 16) len = 16;
    memcpy(buf, title, len);
    memcpy(&rom[0x0134], buf, 16);

    rom[0x0147] = 0x00; // ROM only
    rom[0x0148] = 0x00; // 32KB
    rom[0x0149] = 0x00; // no RAM

    uint8_t x = 0;
    for (int i = 0x0134; i <= 0x014C; i++) x = x - rom[i] - 1;
    rom[0x014D] = x;
}

int main(void) {
    uint8_t *rom = calloc(1, 32768);
    if (!rom) {
        fprintf(stderr, "Alloc ROM simple échouée\n");
        return 1;
    }

    write_header(rom, "SIMPLE SQUARE");

    // Message série "START : OK\n" à 0x0200
    const char *msg = "START : OK\n";
    memcpy(&rom[0x0200], msg, strlen(msg));

    // Programme à 0x0150
    uint16_t p = 0x0150; uint8_t *c = &rom[p];

    // Envoyer le message série
    *c++ = 0x21; *c++ = 0x00; *c++ = 0x02; // LD HL,0200h
    *c++ = 0x06; *c++ = (uint8_t)strlen(msg); // LD B,len
    uint16_t loop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x7E;             // LD A,(HL)
    *c++ = 0xE0; *c++ = 0x01; // LDH (FF01),A
    *c++ = 0x3E; *c++ = 0x81; // LD A,81h
    *c++ = 0xE0; *c++ = 0x02; // LDH (FF02),A
    *c++ = 0x23;             // INC HL
    *c++ = 0x05;             // DEC B
    int8_t rel = (int8_t)(loop - (((p + (c - &rom[p])) + 2)));
    *c++ = 0x20; *c++ = (uint8_t)rel; // JR NZ,loop

    // Eteindre LCD pendant écriture VRAM, configurer palette
    *c++ = 0x3E; *c++ = 0x00; // LD A,00h (LCD off)
    *c++ = 0xE0; *c++ = 0x40; // LDH (LCDC),A
    *c++ = 0x3E; *c++ = 0xE4; // LD A,E4h
    *c++ = 0xE0; *c++ = 0x47; // LDH (BGP),A

    // Définir le tile 1 = tout noir (8 lignes, deux plans 0xFF)
    *c++ = 0x21; *c++ = 0x10; *c++ = 0x80; // LD HL,8010h (tile 1)
    *c++ = 0x0E; *c++ = 0x10;             // LD C,16
    *c++ = 0x3E; *c++ = 0xFF;             // LD A,FFh
    uint16_t tloop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x77;                           // LD (HL),A
    *c++ = 0x23;                           // INC HL
    *c++ = 0x0D;                           // DEC C
    rel = (int8_t)(tloop - (((p + (c - &rom[p])) + 2)));
    *c++ = 0x20; *c++ = (uint8_t)rel;      // JR NZ,tloop

    // Effacer le BG (tout tile 0)
    *c++ = 0x11; *c++ = 0x00; *c++ = 0x98; // LD DE,9800h
    *c++ = 0x06; *c++ = 18;                // LD B,18
    uint16_t rloop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x0E; *c++ = 32;                // LD C,32
    *c++ = 0xAF;                           // XOR A (0)
    uint16_t cloop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x12;                           // LD (DE),A
    *c++ = 0x13;                           // INC DE
    *c++ = 0x0D;                           // DEC C
    rel = (int8_t)(cloop - (((p + (c - &rom[p])) + 2)));
    *c++ = 0x20; *c++ = (uint8_t)rel;      // JR NZ,cloop
    *c++ = 0x05;                           // DEC B
    rel = (int8_t)(rloop - (((p + (c - &rom[p])) + 2)));
    *c++ = 0x20; *c++ = (uint8_t)rel;      // JR NZ,rloop

    // Dessiner un carré 10x10 au centre (x=75,y=67) avec tile 1
    *c++ = 0x11; *c++ = 0x4B; *c++ = 0x98; // LD DE,9800+67*32+75
    *c++ = 0x06; *c++ = 10;                // LD B,10 (hauteur)
    uint16_t yloop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x0E; *c++ = 10;                // LD C,10 (largeur)
    *c++ = 0x3E; *c++ = 0x01;              // LD A,1
    uint16_t xloop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x12;                           // LD (DE),A
    *c++ = 0x13;                           // INC DE
    *c++ = 0x0D;                           // DEC C
    rel = (int8_t)(xloop - (((p + (c - &rom[p])) + 2)));
    *c++ = 0x20; *c++ = (uint8_t)rel;      // JR NZ,xloop
    // ligne suivante: DE += (32-10)=22
    *c++ = 0x21; *c++ = 22; *c++ = 0x00;   // LD HL,22
    *c++ = 0x19;                           // ADD HL,DE
    *c++ = 0xEB;                           // EX DE,HL
    *c++ = 0x05;                           // DEC B
    rel = (int8_t)(yloop - (((p + (c - &rom[p])) + 2)));
    *c++ = 0x20; *c++ = (uint8_t)rel;      // JR NZ,yloop

    // Allumer LCD
    *c++ = 0x3E; *c++ = 0x91; // LD A,91h (LCD on, BG on)
    *c++ = 0xE0; *c++ = 0x40; // LDH (LCDC),A

    // Boucle infinie
    *c++ = 0x18; *c++ = 0xFE;              // JR -2

    FILE *f = fopen("../simple_square.gb", "wb");
    if (!f) { fprintf(stderr, "Créer simple_square.gb échoué\n"); free(rom); return 1; }
    fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    return 0;
}



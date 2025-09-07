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
    if (!rom) { fprintf(stderr, "Alloc ROM joypad échouée\n"); return 1; }
    write_header(rom, "JOY PAD DEF ");

    const char *start = "START : OK | ROM: JOYPAD_DEFAULT reads P1 and prints PASS if == CF\n";
    const char *pass = "PASS\n";
    const char *fail = "FAIL\n";
    memcpy(&rom[0x0200], start, strlen(start));
    memcpy(&rom[0x0240], pass, strlen(pass));
    memcpy(&rom[0x0280], fail, strlen(fail));

    uint16_t p = 0x0150; uint8_t *c = &rom[p];
    // Send START
    *c++ = 0x21; *c++ = 0x00; *c++ = 0x02; // HL=0200
    *c++ = 0x06; *c++ = (uint8_t)strlen(start);
    uint16_t loop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x7E; *c++ = 0xE0; *c++ = 0x01; *c++ = 0x3E; *c++ = 0x81; *c++ = 0xE0; *c++ = 0x02;
    *c++ = 0x23; *c++ = 0x05; int8_t rel = (int8_t)(loop - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel;

    // Read P1 and compare with 0xCF -> Z=1 if equal
    *c++ = 0xF0; *c++ = 0x00; // LDH A,(FF00)
    *c++ = 0xFE; *c++ = 0xCF; // CP 0xCF
    // If NZ -> FAIL
    *c++ = 0xC2; uint8_t *addr = c; c += 2; // JP NZ,nn (placeholder)
    // PASS path
    uint16_t pass_path = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x21; *c++ = 0x40; *c++ = 0x02; // HL=0240
    *c++ = 0x06; *c++ = (uint8_t)strlen(pass);
    loop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x7E; *c++ = 0xE0; *c++ = 0x01; *c++ = 0x3E; *c++ = 0x81; *c++ = 0xE0; *c++ = 0x02;
    *c++ = 0x23; *c++ = 0x05; rel = (int8_t)(loop - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel;
    *c++ = 0x76; // HALT
    // FAIL path
    uint16_t fail_path = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x21; *c++ = 0x80; *c++ = 0x02; // HL=0280
    *c++ = 0x06; *c++ = (uint8_t)strlen(fail);
    loop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x7E; *c++ = 0xE0; *c++ = 0x01; *c++ = 0x3E; *c++ = 0x81; *c++ = 0xE0; *c++ = 0x02;
    *c++ = 0x23; *c++ = 0x05; rel = (int8_t)(loop - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel;
    *c++ = 0x76; // HALT
    // Fill JP NZ addr
    rom[(size_t)(addr - rom) + 0] = (uint8_t)(fail_path & 0xFF);
    rom[(size_t)(addr - rom) + 1] = (uint8_t)((fail_path >> 8) & 0xFF);

    FILE *f = fopen("../joypad_default.gb", "wb");
    if (!f) { fprintf(stderr, "Créer joypad_default.gb échoué\n"); free(rom); return 1; }
    fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    return 0;
}



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
    if (!rom) { fprintf(stderr, "Alloc ROM key échouée\n"); return 1; }
    write_header(rom, "KEY SERIAL   ");

    // Table des libellés des touches (terminées par \n)
    const char *label_right = "RIGHT\n";
    const char *label_left  = "LEFT\n";
    const char *label_up    = "UP\n";
    const char *label_down  = "DOWN\n";
    const char *label_a     = "A\n";
    const char *label_b     = "B\n";
    const char *label_start = "START\n";
    const char *label_select= "SELECT\n";

    memcpy(&rom[0x0200], label_right, strlen(label_right));
    memcpy(&rom[0x0210], label_left,  strlen(label_left));
    memcpy(&rom[0x0220], label_up,    strlen(label_up));
    memcpy(&rom[0x0230], label_down,  strlen(label_down));
    memcpy(&rom[0x0240], label_a,     strlen(label_a));
    memcpy(&rom[0x0250], label_b,     strlen(label_b));
    memcpy(&rom[0x0260], label_start, strlen(label_start));
    memcpy(&rom[0x0270], label_select,strlen(label_select));

    uint16_t p = 0x0150; uint8_t *c = &rom[p];
    // Positions à patcher pour les JR send (octet de déplacement)
    int jr_pos[16]; int jr_count = 0;

    // Boucle principale: lire P1 et envoyer le label correspondant
    uint16_t loop = (uint16_t)(p + (c - &rom[p]));
    // Sélectionner d'abord la ligne boutons (P15)
    *c++ = 0x3E; *c++ = 0x20; // LD A,20h (select buttons)
    *c++ = 0xE0; *c++ = 0x00; // LDH (FF00),A
    *c++ = 0xF0; *c++ = 0x00; // LDH A,(FF00) P1
    // Priorité: A,B,Start,Select,Right,Left,Up,Down
    // A (bit0==0)
    *c++ = 0xE6; *c++ = 0x01; *c++ = 0x20; *c++ = 0x07; // AND 01h; JR NZ,skipA
    *c++ = 0x21; *c++ = 0x40; *c++ = 0x02; // HL=0240 "A\n"
    *c++ = 0x18; jr_pos[jr_count++] = (int)(c - rom); *c++ = 0x00; // JR send (patch plus tard)
    // skipA:
    // B (bit1==0)
    *c++ = 0xF0; *c++ = 0x00; *c++ = 0xE6; *c++ = 0x02; *c++ = 0x20; *c++ = 0x07;
    *c++ = 0x21; *c++ = 0x50; *c++ = 0x02; // HL=0250 "B\n"
    *c++ = 0x18; jr_pos[jr_count++] = (int)(c - rom); *c++ = 0x00;
    // Start (bit3==0)
    *c++ = 0xF0; *c++ = 0x00; *c++ = 0xE6; *c++ = 0x08; *c++ = 0x20; *c++ = 0x07;
    *c++ = 0x21; *c++ = 0x60; *c++ = 0x02; // HL=0260 "START\n"
    *c++ = 0x18; jr_pos[jr_count++] = (int)(c - rom); *c++ = 0x00;
    // Select (bit2==0)
    *c++ = 0xF0; *c++ = 0x00; *c++ = 0xE6; *c++ = 0x04; *c++ = 0x20; *c++ = 0x07;
    *c++ = 0x21; *c++ = 0x70; *c++ = 0x02; // HL=0270 "SELECT\n"
    *c++ = 0x18; jr_pos[jr_count++] = (int)(c - rom); *c++ = 0x00;
    // Directions: sélectionner la ligne P14 puis lire
    *c++ = 0x3E; *c++ = 0x10; // LD A,10h (select directions)
    *c++ = 0xE0; *c++ = 0x00; // LDH (FF00),A
    *c++ = 0xF0; *c++ = 0x00; // LDH A,(FF00)
    // Right (bit0==0)
    *c++ = 0xE6; *c++ = 0x01; *c++ = 0x20; *c++ = 0x07;
    *c++ = 0x21; *c++ = 0x00; *c++ = 0x02; // HL=0200 "RIGHT\n"
    *c++ = 0x18; jr_pos[jr_count++] = (int)(c - rom); *c++ = 0x00;
    // Left (bit1==0)
    *c++ = 0xF0; *c++ = 0x00; *c++ = 0xE6; *c++ = 0x02; *c++ = 0x20; *c++ = 0x07;
    *c++ = 0x21; *c++ = 0x10; *c++ = 0x02; // HL=0210 "LEFT\n"
    *c++ = 0x18; jr_pos[jr_count++] = (int)(c - rom); *c++ = 0x00;
    // Up (bit2==0)
    *c++ = 0xF0; *c++ = 0x00; *c++ = 0xE6; *c++ = 0x04; *c++ = 0x20; *c++ = 0x07;
    *c++ = 0x21; *c++ = 0x20; *c++ = 0x02; // HL=0220 "UP\n"
    *c++ = 0x18; jr_pos[jr_count++] = (int)(c - rom); *c++ = 0x00; // fallthrough via JR send
    // Down (bit3==0)
    *c++ = 0xF0; *c++ = 0x00; *c++ = 0xE6; *c++ = 0x08; *c++ = 0x20; *c++ = 0x07;
    *c++ = 0x21; *c++ = 0x30; *c++ = 0x02; // HL=0230 "DOWN\n"
    *c++ = 0x18; jr_pos[jr_count++] = (int)(c - rom); *c++ = 0x00;

    // send: envoyer la chaîne pointée par HL
    // Longueur max 8
    *c++ = 0x0E; *c++ = 8; // C=8
    uint16_t send = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x7E; // LD A,(HL)
    *c++ = 0xFE; *c++ = 0x00; *c++ = 0x28; *c++ = 0x08; // CP 0 ; JR Z,skip
    *c++ = 0xE0; *c++ = 0x01; *c++ = 0x3E; *c++ = 0x81; *c++ = 0xE0; *c++ = 0x02; // send
    *c++ = 0x23; *c++ = 0x0D; // INC HL; DEC C
    int8_t rel = (int8_t)(send - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel; // JR NZ
    // skip:
    // Petite attente anti-spam
    *c++ = 0x06; *c++ = 0xFF; uint16_t wait = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x05; rel = (int8_t)(wait - (((p + (c - &rom[p])) + 2))); *c++ = 0x20; *c++ = (uint8_t)rel;
    // Patcher tous les JR send
    for (int i = 0; i < jr_count; i++) {
        uint16_t pos = (uint16_t)jr_pos[i];
        uint16_t next_pc = (uint16_t)(p + pos + 1);
        int8_t disp = (int8_t)(send - next_pc);
        rom[pos] = (uint8_t)disp;
    }

    // Loop forever
    rel = (int8_t)(loop - (((p + (c - &rom[p])) + 2))); *c++ = 0x18; *c++ = (uint8_t)rel;

    FILE *f = fopen("../key_serial.gb", "wb");
    if (!f) { fprintf(stderr, "Créer key_serial.gb échoué\n"); free(rom); return 1; }
    fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    return 0;
}



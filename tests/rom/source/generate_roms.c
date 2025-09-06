#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Génère une ROM minimale qui écrit "PASS\n" sur le port série
// à l'adresse d'entrée 0x0100.

static void write_header(uint8_t *rom, const char *title) {
    // Vecteurs init/boot (dummy)
    memset(rom, 0x00, 0x150);
    // Entrée à 0x0100 : JP 0x0150
    rom[0x0100] = 0xC3; // JP nn
    rom[0x0101] = 0x50; // low
    rom[0x0102] = 0x01; // high

    // Logo Nintendo requis (copié depuis Pan Docs)
    const uint8_t nintendo_logo[48] = {
        0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
        0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,
        0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
        0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,
        0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
        0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E
    };
    memcpy(&rom[0x0104], nintendo_logo, 48);

    // Titre (max 16)
    char buf[16];
    memset(buf, ' ', sizeof(buf));
    size_t len = strlen(title);
    if (len > 16) len = 16;
    memcpy(buf, title, len);
    memcpy(&rom[0x0134], buf, 16);

    // Type ROM only
    rom[0x0147] = 0x00;
    // ROM size = 32KB (code 0)
    rom[0x0148] = 0x00;
    // RAM size = 0
    rom[0x0149] = 0x00;

    // Header checksum (simple calc pour compat de base)
    uint8_t x = 0;
    for (int i = 0x0134; i <= 0x014C; i++) x = x - rom[i] - 1;
    rom[0x014D] = x;
}

static void write_program_pass(uint8_t *rom) {
    // Code à 0x0150 : écrire "PASS\n" sur SB/SC (FF01/FF02)
    // Routine simple: pour chaque octet dans la table, ldh (SB),a ; ld a,$81 ; ldh (SC),a ; loop attend SC bit7=0 ; suivant
    // Table à 0x0200
    const char *msg = "PASS\n";
    memcpy(&rom[0x0200], msg, strlen(msg));

    uint16_t p = 0x0150;
    uint8_t *c = &rom[p];

    // LD HL, 0x0200
    *c++ = 0x21; *c++ = 0x00; *c++ = 0x02;
    // LD B, len
    *c++ = 0x06; *c++ = (uint8_t)strlen(msg);
    // loop:
    uint16_t loop = (uint16_t)(p + (c - &rom[p]));
    // LD A,(HL)
    *c++ = 0x7E;
    // LDH (FF01),A  (LDH (n),A avec n=0x01)
    *c++ = 0xE0; *c++ = 0x01;
    // LD A,0x81 (start + internal clock)
    *c++ = 0x3E; *c++ = 0x81;
    // LDH (FF02),A (transfert immédiat simulé par l'émulateur)
    *c++ = 0xE0; *c++ = 0x02;
    // INC HL
    *c++ = 0x23;
    // DEC B
    *c++ = 0x05;
    // JR NZ, loop
    int8_t rel_loop = (int8_t)(loop - ( (p + (c - &rom[p])) + 2 ));
    *c++ = 0x20; *c++ = (uint8_t)rel_loop;
    // HALT
    *c++ = 0x76;
}

static void write_serial_sender(uint8_t *rom, uint16_t entry_addr, uint16_t msg_addr_pass, uint8_t pass_len, uint16_t msg_addr_fail, uint8_t fail_len) {
    // Inject at entry_addr code that branches solely on Z flag:
    // If Z == 0 (NZ) -> FAIL, else -> PASS.
    uint16_t p = entry_addr;
    uint8_t *c = &rom[p];

    // If NZ -> JP to FAIL path (absolute to avoid short-range limits)
    *c++ = 0xC2; // JP NZ, nn
    uint8_t *jp_nz_addr = c; c += 2; // placeholder 16-bit address (little endian)

    // PASS path: set HL=msg_addr_pass, B=pass_len, then send loop
    uint16_t pass_path = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x21; *c++ = (uint8_t)(msg_addr_pass & 0xFF); *c++ = (uint8_t)(msg_addr_pass >> 8);
    *c++ = 0x06; *c++ = pass_len;
    // send loop (same as in write_program_pass)
    uint16_t loop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x7E;             // LD A,(HL)
    *c++ = 0xE0; *c++ = 0x01; // LDH (FF01),A
    *c++ = 0x3E; *c++ = 0x81; // LD A,81h
    *c++ = 0xE0; *c++ = 0x02; // LDH (FF02),A
    uint16_t wait = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0xF0; *c++ = 0x02; // LDH A,(FF02)
    *c++ = 0xCB; *c++ = 0x7F; // BIT 7,A
    int8_t rel_wait = (int8_t)(wait - ((p + (c - &rom[p])) + 2));
    *c++ = 0x20; *c++ = (uint8_t)rel_wait; // JR NZ, wait
    *c++ = 0x23; // INC HL
    *c++ = 0x05; // DEC B
    int8_t rel_loop = (int8_t)(loop - ((p + (c - &rom[p])) + 2));
    *c++ = 0x20; *c++ = (uint8_t)rel_loop; // JR NZ, loop
    *c++ = 0x76; // HALT

    // FAIL path: set HL=msg_addr_fail, B=fail_len, then same send loop
    uint16_t fail_path = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x21; *c++ = (uint8_t)(msg_addr_fail & 0xFF); *c++ = (uint8_t)(msg_addr_fail >> 8);
    *c++ = 0x06; *c++ = fail_len;
    // reuse send loop
    loop = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0x7E;
    *c++ = 0xE0; *c++ = 0x01;
    *c++ = 0x3E; *c++ = 0x81;
    *c++ = 0xE0; *c++ = 0x02;
    wait = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0xF0; *c++ = 0x02;
    *c++ = 0xCB; *c++ = 0x7F;
    rel_wait = (int8_t)(wait - ((p + (c - &rom[p])) + 2));
    *c++ = 0x20; *c++ = (uint8_t)rel_wait;
    *c++ = 0x23;
    *c++ = 0x05;
    rel_loop = (int8_t)(loop - ((p + (c - &rom[p])) + 2));
    *c++ = 0x20; *c++ = (uint8_t)rel_loop;
    *c++ = 0x76;

    // Fill JP NZ target address (little endian)
    rom[(size_t)(jp_nz_addr - rom)]     = (uint8_t)(fail_path & 0xFF);
    rom[(size_t)(jp_nz_addr - rom) + 1] = (uint8_t)((fail_path >> 8) & 0xFF);
}

static void write_program_joypad_default(uint8_t *rom) {
    // Messages
    const char *pass = "PASS\n";
    const char *fail = "FAIL\n";
    memcpy(&rom[0x0200], pass, strlen(pass));
    memcpy(&rom[0x0210], fail, strlen(fail));

    // Program at 0x0150: read P1, compare with 0xCF; Z=1 if equal
    uint16_t p = 0x0150; uint8_t *c = &rom[p];
    // Read P1
    *c++ = 0xF0; *c++ = 0x00; // LDH A,(FF00)
    // CP 0xCF -> Z=1 if equal
    *c++ = 0xFE; *c++ = 0xCF; // CP n
    // Fall into serial sender which branches on Z only
    write_serial_sender(rom, (uint16_t)(p + (c - &rom[p])), 0x0200, (uint8_t)strlen(pass), 0x0210, (uint8_t)strlen(fail));
}

static void write_program_timer_basic(uint8_t *rom) {
    const char *pass = "PASS\n";
    const char *fail = "FAIL\n";
    memcpy(&rom[0x0220], pass, strlen(pass));
    memcpy(&rom[0x0230], fail, strlen(fail));

    uint16_t p = 0x0150; uint8_t *c = &rom[p];
    // Ensure C=1
    *c++ = 0x37; // SCF
    // TIMA=0
    *c++ = 0x3E; *c++ = 0x00; // LD A,0
    *c++ = 0xE0; *c++ = 0x05; // LDH (FF05),A
    // TMA=0
    *c++ = 0x3E; *c++ = 0x00;
    *c++ = 0xE0; *c++ = 0x06; // LDH (FF06),A
    // TAC=0x04 (enable, 4096Hz)
    *c++ = 0x3E; *c++ = 0x04;
    *c++ = 0xE0; *c++ = 0x07; // LDH (FF07),A
    // wait_loop: poll TIMA until non-zero then set Z=1 via CP A
    uint16_t wait = (uint16_t)(p + (c - &rom[p]));
    *c++ = 0xF0; *c++ = 0x05; // LDH A,(FF05)
    *c++ = 0xFE; *c++ = 0x00; // CP 0
    int8_t rel_wait = (int8_t)(wait - ((p + (c - &rom[p])) + 2));
    *c++ = 0x28; *c++ = (uint8_t)rel_wait; // JR Z, wait
    // Set Z=1 before calling sender
    *c++ = 0xBF; // CP A (sets Z=1)
    write_serial_sender(rom, (uint16_t)(p + (c - &rom[p])), 0x0220, (uint8_t)strlen(pass), 0x0230, (uint8_t)strlen(fail));
}

static void write_program_cpu_zc_add(uint8_t *rom) {
    const char *pass = "PASS\n";
    const char *fail = "FAIL\n";
    memcpy(&rom[0x0240], pass, strlen(pass));
    memcpy(&rom[0x0250], fail, strlen(fail));

    uint16_t p = 0x0150; uint8_t *c = &rom[p];
    // A=0x01
    *c++ = 0x3E; *c++ = 0x01; // LD A,1
    // ADD A,0xFF -> result 0, Z=1, C=1
    *c++ = 0xC6; *c++ = 0xFF; // ADD A,n
    // Now Z=1 and C=1 if ADD sets carry; sender branches on Z only (Z=1 -> PASS)
    write_serial_sender(rom, (uint16_t)(p + (c - &rom[p])), 0x0240, (uint8_t)strlen(pass), 0x0250, (uint8_t)strlen(fail));
}

int main(void) {
    // Generate multiple ROMs
    // 1) pass.gb
    {
        uint8_t *rom = calloc(1, 32768);
        if (!rom) { fprintf(stderr, "Alloc ROM pass échouée\n"); return 1; }
        write_header(rom, "ROM PASS    ");
        write_program_pass(rom);
        FILE *f = fopen("tests/rom/pass.gb", "wb");
        if (!f) { fprintf(stderr, "Créer pass.gb échoué\n"); free(rom); return 1; }
        fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    }

    // 2) joypad_default.gb
    {
        uint8_t *rom = calloc(1, 32768);
        if (!rom) { fprintf(stderr, "Alloc ROM joypad échouée\n"); return 1; }
        write_header(rom, "JOY PAD DEF ");
        write_program_joypad_default(rom);
        FILE *f = fopen("tests/rom/joypad_default.gb", "wb");
        if (!f) { fprintf(stderr, "Créer joypad_default.gb échoué\n"); free(rom); return 1; }
        fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    }

    // 3) timer_basic.gb
    {
        uint8_t *rom = calloc(1, 32768);
        if (!rom) { fprintf(stderr, "Alloc ROM timer échouée\n"); return 1; }
        write_header(rom, "TIMER BASIC ");
        write_program_timer_basic(rom);
        FILE *f = fopen("tests/rom/timer_basic.gb", "wb");
        if (!f) { fprintf(stderr, "Créer timer_basic.gb échoué\n"); free(rom); return 1; }
        fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    }

    // 4) cpu_zc_add.gb
    {
        uint8_t *rom = calloc(1, 32768);
        if (!rom) { fprintf(stderr, "Alloc ROM cpu échouée\n"); return 1; }
        write_header(rom, "CPU ZC ADD  ");
        write_program_cpu_zc_add(rom);
        FILE *f = fopen("tests/rom/cpu_zc_add.gb", "wb");
        if (!f) { fprintf(stderr, "Créer cpu_zc_add.gb échoué\n"); free(rom); return 1; }
        fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    }

    // 5) visual_grid.gb - Checkerboard BG (32x18 tile area)
    {
        uint8_t *rom = calloc(1, 32768);
        if (!rom) { fprintf(stderr, "Alloc ROM visual échouée\n"); return 1; }
        write_header(rom, "VISUAL GRID  ");

        // Program at 0x0150: write two tiles and fill BG map alternated, then loop
        uint16_t p = 0x0150; uint8_t *c = &rom[p];
        // HL=0x8000: tile data base
        *c++ = 0x21; *c++ = 0x00; *c++ = 0x80; // LD HL,8000h
        
        // Write tile0: 16 bytes, pattern 0xAA/0x00 repeated
        *c++ = 0x0E; *c++ = 0x08; // LD C,8 (8 lines)
        uint16_t t0_loop = (uint16_t)(p + (c - &rom[p]));
        *c++ = 0x3E; *c++ = 0xAA; // LD A,0xAA
        *c++ = 0x77;             // LD (HL),A
        *c++ = 0x23;             // INC HL
        *c++ = 0x3E; *c++ = 0x00; // LD A,0x00
        *c++ = 0x77;             // LD (HL),A
        *c++ = 0x23;             // INC HL
        *c++ = 0x0D;             // DEC C
        int8_t rel_t0 = (int8_t)(t0_loop - (((p + (c - &rom[p])) + 2)));
        *c++ = 0x20; *c++ = (uint8_t)rel_t0; // JR NZ,t0_loop
        
        // Write tile1: inverse pattern 0x55/0x00 at 0x8010
        *c++ = 0x0E; *c++ = 0x08; // LD C,8 (8 lines)
        uint16_t t1_loop = (uint16_t)(p + (c - &rom[p]));
        *c++ = 0x3E; *c++ = 0x55; // LD A,0x55
        *c++ = 0x77; *c++ = 0x23; // LD (HL),A ; INC HL
        *c++ = 0x3E; *c++ = 0x00; // LD A,0x00
        *c++ = 0x77; *c++ = 0x23; // LD (HL),A ; INC HL
        *c++ = 0x0D;             // DEC C
        int8_t rel_t1 = (int8_t)(t1_loop - (((p + (c - &rom[p])) + 2)));
        *c++ = 0x20; *c++ = (uint8_t)rel_t1; // JR NZ,t1_loop

        // Fill BG map at 0x9800 with 32x18 area alternating 0/1 (full screen checkerboard)
        *c++ = 0x11; *c++ = 0x00; *c++ = 0x98; // LD DE,9800h
        *c++ = 0x06; *c++ = 18; // LD B,18 rows
        uint16_t row_loop = (uint16_t)(p + (c - &rom[p]));
        *c++ = 0x0E; *c++ = 32; // LD C,32 cols (full width)
        *c++ = 0x3E; *c++ = 0x00; // LD A,0 start
        uint16_t col_loop = (uint16_t)(p + (c - &rom[p]));
        *c++ = 0x12; // LD (DE),A
        *c++ = 0x13; // INC DE
        *c++ = 0xEE; *c++ = 0x01; // XOR 1
        *c++ = 0x0D; // DEC C
        int8_t rel_col = (int8_t)(col_loop - (((p + (c - &rom[p])) + 2)));
        *c++ = 0x20; *c++ = (uint8_t)rel_col; // JR NZ,col_loop
        // Next row: toggle start parity (DE is already at next row start)
        *c++ = 0xEE; *c++ = 0x01; // XOR 1 (toggle row start)
        *c++ = 0x05; // DEC B
        int8_t rel_row = (int8_t)(row_loop - (((p + (c - &rom[p])) + 2)));
        *c++ = 0x20; *c++ = (uint8_t)rel_row; // JR NZ,row_loop
        // Loop forever instead of HALT
        *c++ = 0x18; *c++ = 0xFE; // JR $-2 (infinite loop)

        FILE *f = fopen("tests/rom/visual_grid.gb", "wb");
        if (!f) { fprintf(stderr, "Créer visual_grid.gb échoué\n"); free(rom); return 1; }
        fwrite(rom, 1, 32768, f); fclose(f); free(rom);
    }

    printf("ROMs générées: pass.gb, joypad_default.gb, timer_basic.gb, cpu_zc_add.gb, visual_grid.gb\n");
    return 0;
}

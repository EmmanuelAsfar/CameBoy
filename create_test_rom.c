#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Structure d'un header Game Boy
typedef struct {
    uint8_t nop;           // 0x00
    uint8_t jp[3];         // 0x01-0x03
    uint8_t nintendo[4];   // 0x04-0x07
    uint8_t title[11];     // 0x08-0x12
    uint8_t manu[4];       // 0x13-0x16
    uint8_t cgb;           // 0x17
    uint8_t licensee[2];   // 0x18-0x19
    uint8_t sgb;           // 0x1A
    uint8_t cart_type;     // 0x1B
    uint8_t rom_size;      // 0x1C
    uint8_t ram_size;      // 0x1D
    uint8_t dest;          // 0x1E
    uint8_t old_licensee;  // 0x1F
    uint8_t version;       // 0x20
    uint8_t checksum;      // 0x21
    uint16_t global_checksum; // 0x22-0x23
} __attribute__((packed)) gb_header_t;

int main() {
    FILE* f = fopen("test_pattern.gb", "wb");
    if (!f) {
        printf("Erreur: Impossible de créer test_pattern.gb\n");
        return 1;
    }
    
    // Créer un header Game Boy valide
    gb_header_t header = {0};
    header.nop = 0x00;
    header.jp[0] = 0xC3;    // JP
    header.jp[1] = 0x50;    // Adresse basse
    header.jp[2] = 0x01;    // Adresse haute
    header.nintendo[0] = 0xCE;
    header.nintendo[1] = 0xED;
    header.nintendo[2] = 0x66;
    header.nintendo[3] = 0x66;
    
    // Title
    strcpy((char*)header.title, "TEST PATTERN");
    
    header.cart_type = 0x00;  // ROM only
    header.rom_size = 0x00;   // 32KB
    header.dest = 0x00;       // Japan
    header.version = 0x00;
    
    // Écrire le header
    fwrite(&header, sizeof(header), 1, f);
    
    // Code principal de la ROM (sans labels)
    uint8_t rom_code[] = {
        // Initialisation
        0x3E, 0x00,           // LD A, $00
        0xE0, 0x40,           // LD (LCDC), A    ; Désactiver LCD
        
        // Attendre VBlank
        0x3E, 0x90,           // LD A, $90
        0xE0, 0x44,           // LD (LY), A      ; LY = 144 (VBlank)
        
        // Initialiser les registres
        0x3E, 0x00,           // LD A, $00
        0xE0, 0x42,           // LD (SCY), A     ; Scroll Y = 0
        0xE0, 0x43,           // LD (SCX), A     ; Scroll X = 0
        
        // Palette
        0x3E, 0xE4,           // LD A, $E4       ; Palette: blanc, gris clair, gris foncé, noir
        0xE0, 0x47,           // LD (BGP), A
        
        // Créer des tiles de test dans VRAM
        0x21, 0x80, 0x80,     // LD HL, $8000    ; VRAM start
        
        // Tile 0: Tout blanc
        0x06, 0x10,           // LD B, $10       ; 16 bytes
        0x3E, 0x00,           // LD A, $00
        0x77,                 // LD (HL), A
        0x23,                 // INC HL
        0x05,                 // DEC B
        0x20, 0xFC,           // JR NZ, -4
        
        // Tile 1: Rayures
        0x06, 0x08,           // LD B, $08
        0x3E, 0xAA,           // LD A, $AA
        0x77,                 // LD (HL), A
        0x23,                 // INC HL
        0x3E, 0x55,           // LD A, $55
        0x77,                 // LD (HL), A
        0x23,                 // INC HL
        0x05,                 // DEC B
        0x20, 0xF4,           // JR NZ, -12
        
        // Tile 2: Damier
        0x06, 0x08,           // LD B, $08
        0x3E, 0xFF,           // LD A, $FF
        0x77,                 // LD (HL), A
        0x23,                 // INC HL
        0x3E, 0x00,           // LD A, $00
        0x77,                 // LD (HL), A
        0x23,                 // INC HL
        0x05,                 // DEC B
        0x20, 0xF4,           // JR NZ, -12
        
        // Tile 3: Tout noir
        0x06, 0x10,           // LD B, $10
        0x3E, 0xFF,           // LD A, $FF
        0x77,                 // LD (HL), A
        0x23,                 // INC HL
        0x05,                 // DEC B
        0x20, 0xFC,           // JR NZ, -4
        
        // Remplir la tile map
        0x21, 0x98, 0x80,     // LD HL, $9800    ; Tile map
        0x06, 0x12,           // LD B, $12       ; 18 lignes
        0x0E, 0x14,           // LD C, $14       ; 20 colonnes
        0x78,                 // LD A, B
        0x81,                 // ADD A, C
        0xE6, 0x03,           // AND $03         ; Modulo 4
        0x77,                 // LD (HL), A
        0x23,                 // INC HL
        0x0D,                 // DEC C
        0x20, 0xF8,           // JR NZ, -8
        0x05,                 // DEC B
        0x20, 0xF0,           // JR NZ, -16
        
        // Activer le LCD
        0x3E, 0x91,           // LD A, $91       ; LCD on, BG on
        0xE0, 0x40,           // LD (LCDC), A
        
        // Boucle infinie
        0x76,                 // HALT
        0x00,                 // NOP
        0x18, 0xFD,           // JR -3
    };
    
    // Écrire le code
    fwrite(rom_code, sizeof(rom_code), 1, f);
    
    // Remplir le reste de la ROM avec des zéros
    uint8_t zero = 0;
    for (int i = 0; i < 32768 - sizeof(header) - sizeof(rom_code); i++) {
        fwrite(&zero, 1, 1, f);
    }
    
    fclose(f);
    printf("ROM de test créée: test_pattern.gb\n");
    return 0;
}

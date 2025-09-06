#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main() {
    FILE* f = fopen("simple_animation.gb", "wb");
    if (!f) {
        printf("Erreur: Impossible de créer simple_animation.gb\n");
        return 1;
    }
    
    // Header Game Boy simple
    uint8_t header[0x150] = {0};
    
    // Nintendo logo (obligatoire)
    uint8_t nintendo_logo[] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
        0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
        0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };
    memcpy(header + 0x04, nintendo_logo, sizeof(nintendo_logo));
    
    // Title
    strcpy((char*)(header + 0x34), "SIMPLE ANIM");
    
    // Cartridge type: ROM only
    header[0x47] = 0x00;
    
    // ROM size: 32KB
    header[0x48] = 0x00;
    
    // Destination: Japan
    header[0x49] = 0x00;
    
    // Version
    header[0x4C] = 0x00;
    
    // Header checksum
    header[0x4D] = 0x00;
    
    // Global checksum
    header[0x4E] = 0x00;
    header[0x4F] = 0x00;
    
    // Code très simple qui affiche un pattern
    uint8_t rom_code[] = {
        // Point d'entrée à 0x100
        0x00,                   // NOP
        0xC3, 0x00, 0x01,       // JP $0100 (boucle infinie)
        
        // Code simple à partir de 0x150
        0x3E, 0x00,             // LD A, $00
        0xE0, 0x40,             // LD (LCDC), A    ; Désactiver LCD
        
        // Palette (noir, gris foncé, gris clair, blanc)
        0x3E, 0xE4,             // LD A, $E4
        0xE0, 0x47,             // LD (BGP), A
        
        // Créer des tiles dans VRAM
        // Tile 0: Tout blanc
        0x21, 0x00, 0x80,       // LD HL, $8000
        0x06, 0x10,             // LD B, $10 (16 bytes)
        0x3E, 0x00,             // LD A, $00
        0x77,                   // LD (HL), A
        0x23,                   // INC HL
        0x05,                   // DEC B
        0x20, 0xFC,             // JR NZ, -4
        
        // Tile 1: Rectangle noir
        0x06, 0x10,             // LD B, $10 (16 bytes)
        0x3E, 0xFF,             // LD A, $FF
        0x77,                   // LD (HL), A
        0x23,                   // INC HL
        0x05,                   // DEC B
        0x20, 0xFC,             // JR NZ, -4
        
        // Remplir la tile map avec un pattern
        0x21, 0x00, 0x98,       // LD HL, $9800 (tile map)
        0x06, 0x12,             // LD B, $12 (18 lignes)
        0x0E, 0x14,             // LD C, $14 (20 colonnes)
        0x78,                   // LD A, B (Y)
        0x81,                   // ADD A, C (Y + X)
        0xE6, 0x01,             // AND $01 (modulo 2)
        0x77,                   // LD (HL), A
        0x23,                   // INC HL
        0x0D,                   // DEC C
        0x20, 0xF8,             // JR NZ, -8
        0x05,                   // DEC B
        0x20, 0xF0,             // JR NZ, -16
        
        // Activer le LCD
        0x3E, 0x91,             // LD A, $91 (LCD on, BG on)
        0xE0, 0x40,             // LD (LCDC), A
        
        // Boucle infinie
        0x76,                   // HALT
        0x00,                   // NOP
        0x18, 0xFD,             // JR -3
    };
    
    // Écrire le header
    fwrite(header, sizeof(header), 1, f);
    
    // Écrire le code
    fwrite(rom_code, sizeof(rom_code), 1, f);
    
    // Remplir le reste avec des zéros
    uint8_t zero = 0;
    for (int i = 0; i < 32768 - sizeof(header) - sizeof(rom_code); i++) {
        fwrite(&zero, 1, 1, f);
    }
    
    fclose(f);
    printf("ROM d'animation simple créée: simple_animation.gb\n");
    return 0;
}

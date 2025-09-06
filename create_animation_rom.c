#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main() {
    FILE* f = fopen("animation_test.gb", "wb");
    if (!f) {
        printf("Erreur: Impossible de créer animation_test.gb\n");
        return 1;
    }
    
    // Header Game Boy complet
    uint8_t header[0x150] = {0};
    
    // Nintendo logo (obligatoire)
    uint8_t nintendo_logo[] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
        0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
        0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };
    memcpy(header + 0x04, nintendo_logo, sizeof(nintendo_logo));
    
    // Title
    strcpy((char*)(header + 0x34), "ANIMATION");
    
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
    
    // Code principal qui fait une animation
    uint8_t rom_code[] = {
        // Point d'entrée à 0x100
        0x00,                   // NOP
        0xC3, 0x00, 0x01,       // JP $0100 (boucle infinie)
        
        // Code d'animation à partir de 0x150
        0x3E, 0x00,             // LD A, $00
        0xE0, 0x40,             // LD (LCDC), A    ; Désactiver LCD
        
        // Initialiser la palette (noir, gris foncé, gris clair, blanc)
        0x3E, 0xE4,             // LD A, $E4
        0xE0, 0x47,             // LD (BGP), A
        
        // Créer des tiles dans VRAM
        // Tile 0: Tout blanc (fond)
        0x21, 0x00, 0x80,       // LD HL, $8000
        0x06, 0x10,             // LD B, $10 (16 bytes)
        0x3E, 0x00,             // LD A, $00
tile0: 0x77,                   // LD (HL), A
        0x23,                   // INC HL
        0x05,                   // DEC B
        0x20, 0xFC,             // JR NZ, tile0
        
        // Tile 1: Rectangle noir 8x8
        0x06, 0x10,             // LD B, $10 (16 bytes)
        0x3E, 0xFF,             // LD A, $FF
tile1: 0x77,                   // LD (HL), A
        0x23,                   // INC HL
        0x05,                   // DEC B
        0x20, 0xFC,             // JR NZ, tile1
        
        // Tile 2: Rectangle gris
        0x06, 0x08,             // LD B, $08
tile2: 0x3E, 0xAA,             // LD A, $AA
        0x77,                   // LD (HL), A
        0x23,                   // INC HL
        0x3E, 0x55,             // LD A, $55
        0x77,                   // LD (HL), A
        0x23,                   // INC HL
        0x05,                   // DEC B
        0x20, 0xF4,             // JR NZ, tile2
        
        // Tile 3: Rectangle blanc
        0x06, 0x10,             // LD B, $10
        0x3E, 0x00,             // LD A, $00
tile3: 0x77,                   // LD (HL), A
        0x23,                   // INC HL
        0x05,                   // DEC B
        0x20, 0xFC,             // JR NZ, tile3
        
        // Variables d'animation
        // 0xC000: position X (0-19)
        // 0xC001: position Y (0-17) 
        // 0xC002: direction X (0=right, 1=left)
        // 0xC003: direction Y (0=down, 1=up)
        
        // Initialiser les variables
        0x3E, 0x0A,             // LD A, $0A (X = 10)
        0xEA, 0x00, 0xC0,       // LD ($C000), A
        0x3E, 0x08,             // LD A, $08 (Y = 8)
        0xEA, 0x01, 0xC0,       // LD ($C001), A
        0x3E, 0x00,             // LD A, $00 (dir X = right)
        0xEA, 0x02, 0xC0,       // LD ($C002), A
        0x3E, 0x00,             // LD A, $00 (dir Y = down)
        0xEA, 0x03, 0xC0,       // LD ($C003), A
        
        // Activer le LCD
        0x3E, 0x91,             // LD A, $91 (LCD on, BG on)
        0xE0, 0x40,             // LD (LCDC), A
        
        // Boucle d'animation principale
anim_loop:
        // Effacer l'écran (remplir avec tile 0)
        0x21, 0x00, 0x98,       // LD HL, $9800 (tile map)
        0x06, 0x12,             // LD B, $12 (18 lignes)
clear_y: 0x0E, 0x14,           // LD C, $14 (20 colonnes)
clear_x: 0x3E, 0x00,           // LD A, $00 (tile 0 = blanc)
        0x77,                   // LD (HL), A
        0x23,                   // INC HL
        0x0D,                   // DEC C
        0x20, 0xF8,             // JR NZ, clear_x
        0x05,                   // DEC B
        0x20, 0xF0,             // JR NZ, clear_y
        
        // Dessiner le rectangle à la position actuelle
        0x3A, 0x00, 0xC0,       // LD A, ($C000) (X)
        0x4F,                   // LD C, A
        0x3A, 0x01, 0xC0,       // LD A, ($C001) (Y)
        0x47,                   // LD B, A
        0x21, 0x00, 0x98,       // LD HL, $9800 (tile map)
        
        // Calculer l'adresse: HL = $9800 + Y*20 + X
        // Y*20 = Y*16 + Y*4
        0x78,                   // LD A, B (Y)
        0x87,                   // ADD A, A (Y*2)
        0x87,                   // ADD A, A (Y*4)
        0x87,                   // ADD A, A (Y*8)
        0x87,                   // ADD A, A (Y*16)
        0x80,                   // ADD A, B (Y*17)
        0x80,                   // ADD A, B (Y*18)
        0x80,                   // ADD A, B (Y*19)
        0x80,                   // ADD A, B (Y*20)
        0x85,                   // ADD A, L
        0x6F,                   // LD L, A
        0x79,                   // LD A, C (X)
        0x85,                   // ADD A, L
        0x6F,                   // LD L, A
        
        // Dessiner le rectangle (tile 1 = noir)
        0x3E, 0x01,             // LD A, $01
        0x77,                   // LD (HL), A
        
        // Mettre à jour la position X
        0x3A, 0x00, 0xC0,       // LD A, ($C000) (X)
        0x3A, 0x02, 0xC0,       // LD A, ($C002) (dir X)
        0xFE, 0x00,             // CP $00
        0x20, 0x08,             // JR NZ, move_left
        
        // Move right
        0x3A, 0x00, 0xC0,       // LD A, ($C000)
        0x3C,                   // INC A
        0xFE, 0x13,             // CP $13 (19)
        0x20, 0x0A,             // JR NZ, store_x
        0x3E, 0x01,             // LD A, $01 (change direction)
        0xEA, 0x02, 0xC0,       // LD ($C002), A
        0x18, 0x04,             // JR store_x
        
move_left:
        0x3A, 0x00, 0xC0,       // LD A, ($C000)
        0x3D,                   // DEC A
        0xFE, 0xFF,             // CP $FF (-1)
        0x20, 0x04,             // JR NZ, store_x
        0x3E, 0x00,             // LD A, $00 (change direction)
        0xEA, 0x02, 0xC0,       // LD ($C002), A
        
store_x:
        0xEA, 0x00, 0xC0,       // LD ($C000), A
        
        // Mettre à jour la position Y
        0x3A, 0x01, 0xC0,       // LD A, ($C001) (Y)
        0x3A, 0x03, 0xC0,       // LD A, ($C003) (dir Y)
        0xFE, 0x00,             // CP $00
        0x20, 0x08,             // JR NZ, move_up
        
        // Move down
        0x3A, 0x01, 0xC0,       // LD A, ($C001)
        0x3C,                   // INC A
        0xFE, 0x11,             // CP $11 (17)
        0x20, 0x0A,             // JR NZ, store_y
        0x3E, 0x01,             // LD A, $01 (change direction)
        0xEA, 0x03, 0xC0,       // LD ($C003), A
        0x18, 0x04,             // JR store_y
        
move_up:
        0x3A, 0x01, 0xC0,       // LD A, ($C001)
        0x3D,                   // DEC A
        0xFE, 0xFF,             // CP $FF (-1)
        0x20, 0x04,             // JR NZ, store_y
        0x3E, 0x00,             // LD A, $00 (change direction)
        0xEA, 0x03, 0xC0,       // LD ($C003), A
        
store_y:
        0xEA, 0x01, 0xC0,       // LD ($C001), A
        
        // Attendre (boucle de délai)
        0x01, 0x00, 0x10,       // LD BC, $1000
delay: 0x0B,                   // DEC BC
        0x78,                   // LD A, B
        0xB1,                   // OR C
        0x20, 0xFB,             // JR NZ, delay
        
        // Retour à la boucle d'animation
        0x18, 0x80,             // JR anim_loop (relatif -128)
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
    printf("ROM d'animation créée: animation_test.gb\n");
    return 0;
}

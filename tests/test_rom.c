#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Types de base
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

// Créer une ROM de test simple
int create_test_rom(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Erreur: Impossible de créer %s\n", filename);
        return 1;
    }
    
    // En-tête de cartouche minimal
    u8 rom[0x8000] = {0};
    
    // Logo Nintendo (requis)
    u8 nintendo_logo[48] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
        0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
        0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };
    
    // Copier le logo à l'adresse 0x104
    memcpy(&rom[0x104], nintendo_logo, 48);
    
    // Titre
    strcpy((char*)&rom[0x134], "TEST ROM");
    
    // Type de cartouche (ROM seulement)
    rom[0x147] = 0x00;
    
    // Taille ROM (32KB)
    rom[0x148] = 0x00;
    
    // Taille RAM (aucune)
    rom[0x149] = 0x00;
    
    // Destination (non-Japon)
    rom[0x14A] = 0x01;
    
    // Version
    rom[0x14C] = 0x00;
    
    // Checksum d'en-tête (calculé)
    u8 checksum = 0;
    for (int i = 0x134; i <= 0x14C; i++) {
        checksum = checksum - rom[i] - 1;
    }
    rom[0x14D] = checksum;
    
    // Checksum global (simplifié)
    rom[0x14E] = 0x00;
    rom[0x14F] = 0x00;
    
    // Code de démarrage simple
    // 0x100: NOP
    rom[0x100] = 0x00;
    
    // 0x101: JP 0x150 (saut vers le code principal)
    rom[0x101] = 0xC3;
    rom[0x102] = 0x50;
    rom[0x103] = 0x01;
    
    // Code principal (0x150)
    // Boucle infinie simple
    rom[0x150] = 0x18;  // JR e (saut relatif)
    rom[0x151] = 0xFE;  // -2 (boucle sur soi-même)
    
    // Écrire la ROM
    fwrite(rom, 1, sizeof(rom), file);
    fclose(file);
    
    printf("ROM de test créée : %s\n", filename);
    return 0;
}

int main() {
    printf("Générateur de ROM de test pour CameBoy\n");
    
    if (create_test_rom("tests/test_rom.gb") == 0) {
        printf("ROM de test créée avec succès !\n");
        printf("Vous pouvez maintenant tester l'émulateur avec :\n");
        printf("  bin/cameboy.exe tests/test_rom.gb\n");
    }
    
    return 0;
}

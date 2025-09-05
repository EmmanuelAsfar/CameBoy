#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Types de base
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

// Créer une ROM de test qui écrit sur le port série
int create_serial_test_rom(const char* filename) {
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
    strcpy((char*)&rom[0x134], "SERIAL TEST");
    
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
    
    // Code principal (0x150) - Écrit "HELLO" sur le port série
    int addr = 0x150;
    
    // Charger 'H' dans A
    rom[addr++] = 0x3E;  // LD A, n
    rom[addr++] = 'H';   // 'H' = 0x48
    
    // Écrire A sur le port série (0xFF01)
    rom[addr++] = 0xE0;  // LDH (0xFF00 + n), A
    rom[addr++] = 0x01;  // 0x01 -> 0xFF01 (SB)
    
    // Activer la transmission série (0xFF02)
    rom[addr++] = 0x3E;  // LD A, n
    rom[addr++] = 0x81;  // 0x81 = bit 7 (start) + bit 0 (clock)
    rom[addr++] = 0xE0;  // LDH (0xFF00 + n), A
    rom[addr++] = 0x02;  // 0x02 -> 0xFF02 (SC)
    
    // Charger 'E' dans A
    rom[addr++] = 0x3E;  // LD A, n
    rom[addr++] = 'E';   // 'E' = 0x45
    
    // Écrire A sur le port série
    rom[addr++] = 0xE0;  // LDH (0xFF00 + n), A
    rom[addr++] = 0x01;  // 0x01 -> 0xFF01 (SB)
    
    // Activer la transmission série
    rom[addr++] = 0x3E;  // LD A, n
    rom[addr++] = 0x81;  // 0x81
    rom[addr++] = 0xE0;  // LDH (0xFF00 + n), A
    rom[addr++] = 0x02;  // 0x02 -> 0xFF02 (SC)
    
    // Charger 'L' dans A
    rom[addr++] = 0x3E;  // LD A, n
    rom[addr++] = 'L';   // 'L' = 0x4C
    
    // Écrire A sur le port série
    rom[addr++] = 0xE0;  // LDH (0xFF00 + n), A
    rom[addr++] = 0x01;  // 0x01 -> 0xFF01 (SB)
    
    // Activer la transmission série
    rom[addr++] = 0x3E;  // LD A, n
    rom[addr++] = 0x81;  // 0x81
    rom[addr++] = 0xE0;  // LDH (0xFF00 + n), A
    rom[addr++] = 0x02;  // 0x02 -> 0xFF02 (SC)
    
    // Charger 'L' dans A (deuxième L)
    rom[addr++] = 0x3E;  // LD A, n
    rom[addr++] = 'L';   // 'L' = 0x4C
    
    // Écrire A sur le port série
    rom[addr++] = 0xE0;  // LDH (0xFF00 + n), A
    rom[addr++] = 0x01;  // 0x01 -> 0xFF01 (SB)
    
    // Activer la transmission série
    rom[addr++] = 0x3E;  // LD A, n
    rom[addr++] = 0x81;  // 0x81
    rom[addr++] = 0xE0;  // LDH (0xFF00 + n), A
    rom[addr++] = 0x02;  // 0x02 -> 0xFF02 (SC)
    
    // Charger 'O' dans A
    rom[addr++] = 0x3E;  // LD A, n
    rom[addr++] = 'O';   // 'O' = 0x4F
    
    // Écrire A sur le port série
    rom[addr++] = 0xE0;  // LDH (0xFF00 + n), A
    rom[addr++] = 0x01;  // 0x01 -> 0xFF01 (SB)
    
    // Activer la transmission série
    rom[addr++] = 0x3E;  // LD A, n
    rom[addr++] = 0x81;  // 0x81
    rom[addr++] = 0xE0;  // LDH (0xFF00 + n), A
    rom[addr++] = 0x02;  // 0x02 -> 0xFF02 (SC)
    
    // Boucle infinie
    rom[addr++] = 0x18;  // JR e (saut relatif)
    rom[addr++] = 0xFE;  // -2 (boucle sur soi-même)
    
    // Écrire la ROM
    fwrite(rom, 1, sizeof(rom), file);
    fclose(file);
    
    printf("ROM de test série créée : %s\n", filename);
    return 0;
}

int main() {
    printf("Générateur de ROM de test série pour CameBoy\n");
    
    if (create_serial_test_rom("tests/serial_test.gb") == 0) {
        printf("ROM de test série créée avec succès !\n");
        printf("Cette ROM écrit 'HELLO' sur le port série.\n");
        printf("Vous pouvez maintenant tester l'émulateur avec :\n");
        printf("  bin/cameboy_test.exe tests/serial_test.gb\n");
    }
    
    return 0;
}

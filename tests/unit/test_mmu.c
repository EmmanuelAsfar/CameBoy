/**
 * TESTS UNITAIRES POUR LE MMU (Memory Management Unit)
 *
 * Ce fichier contient des tests unitaires pour valider le fonctionnement
 * correct du MMU Game Boy selon les spécifications Pan Docs.
 */

#include "../../src/common.h"
#include "../../src/mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Prototypes des fonctions de test
void test_mmu_init(void);
void test_mmu_reset(void);
void test_mmu_memory_mapping(void);
void test_mmu_cart_parsing(void);
void test_mmu_read_write_8bit(void);
void test_mmu_read_write_16bit(void);
void test_mmu_echo_ram(void);

// Table des tests MMU
typedef struct {
    const char* name;
    void (*test_func)(void);
} UnitTest;

UnitTest mmu_tests[] = {
    {"MMU Initialisation", test_mmu_init},
    {"MMU Reset", test_mmu_reset},
    {"MMU Memory Mapping", test_mmu_memory_mapping},
    {"MMU Cart Parsing", test_mmu_cart_parsing},
    {"MMU Read/Write 8-bit", test_mmu_read_write_8bit},
    {"MMU Read/Write 16-bit", test_mmu_read_write_16bit},
    {"MMU Echo RAM", test_mmu_echo_ram},
    {NULL, NULL} // Marqueur de fin
};

/**
 * FONCTION PRINCIPALE DE TEST
 */
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    printf("=== TESTS UNITAIRES MMU ===\n\n");

    int passed = 0;
    int total = 0;

    for (int i = 0; mmu_tests[i].name != NULL; i++) {
        printf("Test %d: %s... ", i + 1, mmu_tests[i].name);
        fflush(stdout);

        // Exécuter le test
        mmu_tests[i].test_func();

        printf("PASS\n");
        passed++;
        total++;
    }

    printf("\n=== RÉSULTATS ===\n");
    printf("Tests passés: %d/%d\n", passed, total);

    if (passed == total) {
        printf("✅ TOUS LES TESTS SONT PASSÉS !\n");
        return 0;
    } else {
        printf("❌ CERTAINS TESTS ONT ÉCHOUÉ\n");
        return 1;
    }
}

/**
 * IMPLEMENTATION DES TESTS
 */

void test_mmu_init(void) {
    MMU mmu;

    // Initialiser
    mmu_init(&mmu);

    // Vérifier que la mémoire a été allouée
    assert(mmu.memory != NULL);

    // Vérifier les pointeurs de zones mémoire
    assert(mmu.rom == &mmu.memory[0x0000]);
    assert(mmu.vram == &mmu.memory[0x8000]);
    assert(mmu.eram == &mmu.memory[0xA000]);
    assert(mmu.wram == &mmu.memory[0xC000]);
    assert(mmu.oam == &mmu.memory[0xFE00]);
    assert(mmu.io == &mmu.memory[0xFF00]);
    assert(mmu.hram == &mmu.memory[0xFF80]);

    mmu_cleanup(&mmu);
}

void test_mmu_reset(void) {
    MMU mmu;

    mmu_init(&mmu);

    // Modifier quelques valeurs
    mmu.memory[0xFF00] = 0xFF; // P1
    mmu.memory[0xFF40] = 0xFF; // LCDC

    // Reset
    mmu_reset(&mmu);

    // Vérifier les valeurs par défaut selon Pan Docs
    assert(mmu.memory[0xFF00] == 0xCF); // P1
    assert(mmu.memory[0xFF01] == 0x00); // SB
    assert(mmu.memory[0xFF02] == 0x7E); // SC
    assert(mmu.memory[0xFF04] == 0x00); // DIV
    assert(mmu.memory[0xFF05] == 0x00); // TIMA
    assert(mmu.memory[0xFF06] == 0x00); // TMA
    assert(mmu.memory[0xFF07] == 0x00); // TAC
    assert(mmu.memory[0xFF0F] == 0xE1); // IF
    assert(mmu.memory[0xFF40] == 0x91); // LCDC
    assert(mmu.memory[0xFF41] == 0x85); // STAT
    assert(mmu.memory[0xFF44] == 0x90); // LY
    assert(mmu.memory[0xFF45] == 0x00); // LYC
    assert(mmu.memory[0xFFFF] == 0x00); // IE

    mmu_cleanup(&mmu);
}

void test_mmu_memory_mapping(void) {
    MMU mmu;

    mmu_init(&mmu);

    // Test des limites des zones mémoire
    // ROM: 0x0000-0x7FFF
    assert(mmu_read8(&mmu, 0x0000) == 0xFF); // Valeur par défaut
    assert(mmu_read8(&mmu, 0x7FFF) == 0xFF);

    // VRAM: 0x8000-0x9FFF
    assert(mmu_read8(&mmu, 0x8000) == 0xFF);
    assert(mmu_read8(&mmu, 0x9FFF) == 0xFF);

    // ERAM: 0xA000-0xBFFF
    assert(mmu_read8(&mmu, 0xA000) == 0xFF);
    assert(mmu_read8(&mmu, 0xBFFF) == 0xFF);

    // WRAM: 0xC000-0xDFFF
    assert(mmu_read8(&mmu, 0xC000) == 0xFF);
    assert(mmu_read8(&mmu, 0xDFFF) == 0xFF);

    // Echo RAM: 0xE000-0xFDFF (miroir de WRAM)
    assert(mmu_read8(&mmu, 0xE000) == 0xFF);
    assert(mmu_read8(&mmu, 0xFDFF) == 0xFF);

    // OAM: 0xFE00-0xFE9F
    assert(mmu_read8(&mmu, 0xFE00) == 0xFF);
    assert(mmu_read8(&mmu, 0xFE9F) == 0xFF);

    // IO: 0xFF00-0xFF7F
    assert(mmu_read8(&mmu, 0xFF00) == 0xCF); // P1 après reset
    assert(mmu_read8(&mmu, 0xFF7F) == 0xFF);

    // HRAM: 0xFF80-0xFFFE
    assert(mmu_read8(&mmu, 0xFF80) == 0xFF);
    assert(mmu_read8(&mmu, 0xFFFE) == 0xFF);

    // IE: 0xFFFF
    assert(mmu_read8(&mmu, 0xFFFF) == 0x00);

    mmu_cleanup(&mmu);
}

void test_mmu_cart_parsing(void) {
    MMU mmu;
    u8 rom_data[0x150]; // Header + un peu plus

    mmu_init(&mmu);

    // Créer un header ROM valide avec logo Nintendo
    memset(rom_data, 0, sizeof(rom_data));

    // Entry point
    rom_data[0x100] = 0x00;
    rom_data[0x101] = 0xC3;
    rom_data[0x102] = 0x50;
    rom_data[0x103] = 0x01;

    // Logo Nintendo (premiers 16 octets seulement pour le test)
    rom_data[0x104] = 0xCE; rom_data[0x105] = 0xED; rom_data[0x106] = 0x66; rom_data[0x107] = 0x66;
    rom_data[0x108] = 0xCC; rom_data[0x109] = 0x0D; rom_data[0x10A] = 0x00; rom_data[0x10B] = 0x0B;
    rom_data[0x10C] = 0x03; rom_data[0x10D] = 0x73; rom_data[0x10E] = 0x00; rom_data[0x10F] = 0x83;
    rom_data[0x110] = 0x00; rom_data[0x111] = 0x0C; rom_data[0x112] = 0x00; rom_data[0x113] = 0x0D;

    // Title
    strcpy((char*)&rom_data[0x134], "TEST ROM");

    // Cart type
    rom_data[0x147] = 0x00; // ROM ONLY

    // ROM size
    rom_data[0x148] = 0x00; // 32KB

    // RAM size
    rom_data[0x149] = 0x00; // No RAM

    // Parser le header
    bool result = cart_parse_header(&mmu.cart, rom_data);
    assert(result);

    // Vérifier les valeurs parsées
    assert(mmu.cart.header.entry_point[0] == 0x00);
    assert(mmu.cart.header.entry_point[1] == 0xC3);
    assert(mmu.cart.header.entry_point[2] == 0x50);
    assert(mmu.cart.header.entry_point[3] == 0x01);

    assert(strcmp(mmu.cart.header.title, "TEST ROM") == 0);
    assert(mmu.cart.header.cart_type == 0x00); // ROM ONLY
    assert(mmu.cart.header.rom_size == 0x00);
    assert(mmu.cart.header.ram_size == 0x00);

    assert(mmu.cart.type == CART_ROM_ONLY);
    assert(mmu.cart.rom_size == 32 * 1024); // 32KB
    assert(mmu.cart.ram_size == 0);

    mmu_cleanup(&mmu);
}

void test_mmu_read_write_8bit(void) {
    MMU mmu;

    mmu_init(&mmu);

    // Test écriture/lecture dans différentes zones

    // WRAM
    mmu_write8(&mmu, 0xC000, 0xAB);
    assert(mmu_read8(&mmu, 0xC000) == 0xAB);

    // Echo RAM (miroir de WRAM)
    assert(mmu_read8(&mmu, 0xE000) == 0xAB);

    // VRAM
    mmu_write8(&mmu, 0x8000, 0xCD);
    assert(mmu_read8(&mmu, 0x8000) == 0xCD);

    // OAM
    mmu_write8(&mmu, 0xFE00, 0xEF);
    assert(mmu_read8(&mmu, 0xFE00) == 0xEF);

    // HRAM
    mmu_write8(&mmu, 0xFF80, 0x12);
    assert(mmu_read8(&mmu, 0xFF80) == 0x12);

    // IE
    mmu_write8(&mmu, 0xFFFF, 0x34);
    assert(mmu_read8(&mmu, 0xFFFF) == 0x34);

    mmu_cleanup(&mmu);
}

void test_mmu_read_write_16bit(void) {
    MMU mmu;

    mmu_init(&mmu);

    // Test écriture/lecture 16-bit
    mmu_write16(&mmu, 0xC000, 0xABCD);
    assert(mmu_read16(&mmu, 0xC000) == 0xABCD);

    // Vérifier que les octets individuels sont corrects
    assert(mmu_read8(&mmu, 0xC000) == 0xCD); // LSB
    assert(mmu_read8(&mmu, 0xC001) == 0xAB); // MSB

    // Test avec HRAM
    mmu_write16(&mmu, 0xFF80, 0x1234);
    assert(mmu_read16(&mmu, 0xFF80) == 0x1234);

    mmu_cleanup(&mmu);
}

void test_mmu_echo_ram(void) {
    MMU mmu;

    mmu_init(&mmu);

    // Test Echo RAM (miroir de WRAM)
    mmu_write8(&mmu, 0xC000, 0xAB);
    assert(mmu_read8(&mmu, 0xC000) == 0xAB); // WRAM original
    assert(mmu_read8(&mmu, 0xE000) == 0xAB); // Echo RAM

    // Test écriture dans Echo RAM
    mmu_write8(&mmu, 0xE000, 0xCD);
    assert(mmu_read8(&mmu, 0xE000) == 0xCD); // Echo RAM
    assert(mmu_read8(&mmu, 0xC000) == 0xCD); // WRAM original (modifié)

    // Test avec une adresse plus élevée
    mmu_write8(&mmu, 0xC100, 0xEF);
    assert(mmu_read8(&mmu, 0xE100) == 0xEF);

    mmu_cleanup(&mmu);
}

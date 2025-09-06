/**
 * TESTS UNITAIRES POUR LE PPU (Picture Processing Unit)
 *
 * Ce fichier contient des tests unitaires pour valider le fonctionnement
 * correct du PPU Game Boy selon les spécifications Pan Docs.
 */

#include "../../src/common.h"
#include "../../src/ppu.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Prototypes des fonctions de test
void test_ppu_init(void);
void test_ppu_reset(void);
void test_ppu_registers(void);
void test_ppu_modes(void);
void test_ppu_pixel_transfer(void);
void test_ppu_hblank(void);
void test_ppu_vblank(void);
void test_ppu_render_line(void);
void test_ppu_palettes(void);

// Table des tests PPU
typedef struct {
    const char* name;
    void (*test_func)(void);
} UnitTest;

UnitTest ppu_tests[] = {
    {"PPU Initialisation", test_ppu_init},
    {"PPU Reset", test_ppu_reset},
    {"PPU Registers", test_ppu_registers},
    {"PPU Modes", test_ppu_modes},
    {"PPU Pixel Transfer", test_ppu_pixel_transfer},
    {"PPU HBlank", test_ppu_hblank},
    {"PPU VBlank", test_ppu_vblank},
    {"PPU Render Line", test_ppu_render_line},
    {"PPU Palettes", test_ppu_palettes},
    {NULL, NULL} // Marqueur de fin
};

/**
 * FONCTION PRINCIPALE DE TEST
 */
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    printf("=== TESTS UNITAIRES PPU ===\n\n");

    int passed = 0;
    int total = 0;

    for (int i = 0; ppu_tests[i].name != NULL; i++) {
        printf("Test %d: %s... ", i + 1, ppu_tests[i].name);
        fflush(stdout);

        // Exécuter le test
        ppu_tests[i].test_func();

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

void test_ppu_init(void) {
    PPU ppu;

    // Initialiser
    ppu_init(&ppu);

    // Vérifier les valeurs par défaut
    assert(ppu.lcdc == 0x91);
    assert(ppu.stat == 0x85);
    assert(ppu.scy == 0);
    assert(ppu.scx == 0);
    assert(ppu.ly == 0);
    assert(ppu.lyc == 0);
    assert(ppu.bgp == 0xE4); // Palette Blargg
    assert(ppu.wy == 0);
    assert(ppu.wx == 0);

    // Vérifier l'état initial
    assert(ppu.mode == PPU_MODE_OAM_SEARCH);
    assert(ppu.mode_cycles == 0);
    assert(ppu.line_cycles == 0);

    // Vérifier que le framebuffer est alloué
    assert(ppu.framebuffer != NULL);

    // Vérifier que OAM est initialisé
    assert(ppu.oam != NULL);
}

void test_ppu_reset(void) {
    PPU ppu;

    ppu_init(&ppu);

    // Modifier quelques valeurs
    ppu.lcdc = 0xFF;
    ppu.ly = 50;
    ppu.mode = PPU_MODE_VBLANK;

    // Reset
    ppu_reset(&ppu);

    // Vérifier que c'est revenu aux valeurs par défaut
    assert(ppu.lcdc == 0x91);
    assert(ppu.ly == 0);
    assert(ppu.mode == PPU_MODE_OAM_SEARCH);
}

void test_ppu_registers(void) {
    PPU ppu;
    u8 vram[0x2000]; // 8KB VRAM

    ppu_init(&ppu);
    memset(vram, 0, sizeof(vram));

    // Test écriture/lecture LCDC
    ppu_write(&ppu, LCDC_REG, 0xAB);
    assert(ppu_read(&ppu, LCDC_REG) == 0xAB);

    // Test écriture/lecture STAT
    ppu_write(&ppu, STAT_REG, 0x45);
    assert(ppu_read(&ppu, STAT_REG) == 0x45);

    // Test écriture/lecture SCY
    ppu_write(&ppu, SCY_REG, 0x12);
    assert(ppu_read(&ppu, SCY_REG) == 0x12);

    // Test écriture/lecture SCX
    ppu_write(&ppu, SCX_REG, 0x34);
    assert(ppu_read(&ppu, SCX_REG) == 0x34);

    // Test écriture/lecture LY (read-only)
    ppu.ly = 100;
    assert(ppu_read(&ppu, LY_REG) == 100);

    // Test écriture/lecture LYC
    ppu_write(&ppu, LYC_REG, 0x56);
    assert(ppu_read(&ppu, LYC_REG) == 0x56);

    // Test écriture/lecture BGP
    ppu_write(&ppu, BGP_REG, 0x78);
    assert(ppu_read(&ppu, BGP_REG) == 0x78);

    // Test écriture/lecture palettes objets
    ppu_write(&ppu, OBP0_REG, 0x9A);
    assert(ppu_read(&ppu, OBP0_REG) == 0x9A);

    ppu_write(&ppu, OBP1_REG, 0xBC);
    assert(ppu_read(&ppu, OBP1_REG) == 0xBC);

    // Test écriture/lecture WY/WX
    ppu_write(&ppu, WY_REG, 0xDE);
    assert(ppu_read(&ppu, WY_REG) == 0xDE);

    ppu_write(&ppu, WX_REG, 0xF0);
    assert(ppu_read(&ppu, WX_REG) == 0xF0);
}

void test_ppu_modes(void) {
    PPU ppu;
    u8 vram[0x2000];

    ppu_init(&ppu);
    memset(vram, 0, sizeof(vram));

    // Test transition OAM_SEARCH -> PIXEL_TRANSFER
    ppu.mode = PPU_MODE_OAM_SEARCH;
    ppu.mode_cycles = 0;

    // Avancer de 80 cycles (durée OAM_SEARCH)
    for (int i = 0; i < 80; i++) {
        u8 interrupts = ppu_tick(&ppu, 1, vram);
        (void)interrupts; // Supprimer warning
    }

    assert(ppu.mode == PPU_MODE_PIXEL_TRANSFER);
    assert(ppu.mode_cycles == 0);

    // Test transition PIXEL_TRANSFER -> HBLANK
    ppu.mode_cycles = 0;

    // Avancer de 172 cycles (durée PIXEL_TRANSFER)
    for (int i = 0; i < 172; i++) {
        u8 interrupts = ppu_tick(&ppu, 1, vram);
        (void)interrupts;
    }

    assert(ppu.mode == PPU_MODE_HBLANK);
}

void test_ppu_pixel_transfer(void) {
    PPU ppu;
    u8 vram[0x2000];

    ppu_init(&ppu);
    memset(vram, 0, sizeof(vram));

    // Configurer pour le mode PIXEL_TRANSFER
    ppu.mode = PPU_MODE_PIXEL_TRANSFER;
    ppu.mode_cycles = 0;
    ppu.ly = 0;

    // Simuler les 172 cycles de PIXEL_TRANSFER
    for (int i = 0; i < 172; i++) {
        u8 interrupts = ppu_tick(&ppu, 1, vram);
        (void)interrupts;
    }

    // Après PIXEL_TRANSFER, on devrait être en HBLANK
    assert(ppu.mode == PPU_MODE_HBLANK);
}

void test_ppu_hblank(void) {
    PPU ppu;
    u8 vram[0x2000];

    ppu_init(&ppu);
    memset(vram, 0, sizeof(vram));

    // Configurer pour HBLANK
    ppu.mode = PPU_MODE_HBLANK;
    ppu.line_cycles = 456 - 80 - 172; // Cycles restants dans la ligne
    ppu.ly = 0;

    // Avancer jusqu'à la fin de la ligne
    while (ppu.line_cycles < 456) {
        u8 interrupts = ppu_tick(&ppu, 1, vram);
        (void)interrupts;
    }

    // Après HBLANK complet, on devrait être à la ligne suivante
    assert(ppu.ly == 1);
    assert(ppu.mode == PPU_MODE_OAM_SEARCH);
    assert(ppu.line_cycles == 0);
    assert(ppu.mode_cycles == 0);
}

void test_ppu_vblank(void) {
    PPU ppu;
    u8 vram[0x2000];

    ppu_init(&ppu);
    memset(vram, 0, sizeof(vram));

    // Aller à la ligne 143 (dernière ligne visible)
    ppu.ly = 143;
    ppu.mode = PPU_MODE_HBLANK;
    ppu.line_cycles = 456 - 80 - 172;

    // Simuler la transition vers VBLANK
    while (ppu.ly == 143) {
        u8 interrupts = ppu_tick(&ppu, 1, vram);
        if (interrupts & 0x01) { // VBLANK interrupt
            break;
        }
    }

    assert(ppu.ly == 144);
    assert(ppu.mode == PPU_MODE_VBLANK);

    // Simuler les 10 lignes de VBLANK
    for (int line = 144; line < 154; line++) {
        // Chaque ligne dure 456 cycles
        for (int cycle = 0; cycle < 456; cycle++) {
            u8 interrupts = ppu_tick(&ppu, 1, vram);
            (void)interrupts;
        }
        assert(ppu.ly == line + 1);
    }

    // Après VBLANK, retour à la ligne 0
    assert(ppu.ly == 0);
    assert(ppu.mode == PPU_MODE_OAM_SEARCH);
}

void test_ppu_render_line(void) {
    PPU ppu;
    u8 vram[0x2000];

    ppu_init(&ppu);
    memset(vram, 0, sizeof(vram));

    // Configurer LCD activé
    ppu.lcdc = 0x91; // LCD enabled, BG enabled
    ppu.ly = 0;

    // Créer des données de tuiles simples
    // Tile map à 0x9800
    vram[0x1800] = 0x00; // Première tuile (index 0)

    // Tile data à 0x8000
    vram[0x0000] = 0xFF; // Ligne 1 de la tuile
    vram[0x0001] = 0x00; // Ligne 1 de la tuile

    // Rendre la ligne
    ppu_render_line(&ppu, vram);

    // Vérifier que le framebuffer a été modifié
    // La première ligne devrait contenir des pixels
    bool has_pixels = false;
    for (int x = 0; x < 8; x++) { // 8 pixels pour une tuile
        if (ppu.framebuffer[x] != 0xFFFFFFFF) { // Couleur par défaut
            has_pixels = true;
            break;
        }
    }
    assert(has_pixels);
}

void test_ppu_palettes(void) {
    PPU ppu;

    ppu_init(&ppu);

    // Test palette BG par défaut
    assert(ppu.bg_palette[0] == 0xFF); // Blanc
    assert(ppu.bg_palette[1] == 0xAA); // Gris clair
    assert(ppu.bg_palette[2] == 0x55); // Gris foncé
    assert(ppu.bg_palette[3] == 0x00); // Noir

    // Changer la palette BGP
    ppu_write(&ppu, BGP_REG, 0xE4); // 11100100

    // Vérifier les couleurs
    assert(ppu.bg_palette[0] == 0xFF); // Blanc (11)
    assert(ppu.bg_palette[1] == 0xAA); // Gris clair (10)
    assert(ppu.bg_palette[2] == 0xAA); // Gris clair (01)
    assert(ppu.bg_palette[3] == 0x00); // Noir (00)

    // Test fonction get_pixel_color
    u32 color0 = ppu_get_pixel_color(&ppu, 0);
    u32 color1 = ppu_get_pixel_color(&ppu, 1);
    u32 color2 = ppu_get_pixel_color(&ppu, 2);
    u32 color3 = ppu_get_pixel_color(&ppu, 3);

    assert(color0 == 0xFFFFFFFF); // Blanc
    assert(color1 == 0xAAAAAAAA); // Gris clair
    assert(color2 == 0xAAAAAAAA); // Gris clair
    assert(color3 == 0x000000FF); // Noir
}

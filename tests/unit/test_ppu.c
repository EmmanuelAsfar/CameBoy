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
void test_ppu_stat_irq_hblank(void);
void test_ppu_stat_irq_vblank(void);
void test_ppu_stat_irq_oam(void);
void test_ppu_stat_irq_lyc(void);
void test_ppu_window_basic(void);
void test_ppu_sprites_basic(void);
void test_ppu_sprites_priority_vs_bg(void);
void test_ppu_sprites_limit10_per_line(void);
void test_ppu_sprites_flip_xy(void);
void test_ppu_lcdc_off_on(void);
void test_ppu_window_wx_clamp(void);
void test_ppu_fine_scroll_scx(void);
void test_ppu_fifo_basic(void);
void test_ppu_fifo_overflow(void);
void test_ppu_mode_timings(void);
void test_ppu_stat_irq_transitions(void);
void test_ppu_window_edge_cases(void);
void test_ppu_sprite_priority_detailed(void);
void test_ppu_fetcher_basic(void);

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
    {"PPU STAT IRQ HBlank", test_ppu_stat_irq_hblank},
    {"PPU STAT IRQ VBlank", test_ppu_stat_irq_vblank},
    {"PPU STAT IRQ OAM", test_ppu_stat_irq_oam},
    {"PPU STAT IRQ LYC", test_ppu_stat_irq_lyc},
    {"PPU Window Basic", test_ppu_window_basic},
    {"PPU Sprites Basic", test_ppu_sprites_basic},
    {"PPU Sprites Priority vs BG", test_ppu_sprites_priority_vs_bg},
    {"PPU Sprites Limit 10", test_ppu_sprites_limit10_per_line},
    {"PPU Sprites Flip XY", test_ppu_sprites_flip_xy},
    {"PPU LCDC Off/On", test_ppu_lcdc_off_on},
    {"PPU Window WX Clamp", test_ppu_window_wx_clamp},
    {"PPU Fine Scroll SCX", test_ppu_fine_scroll_scx},
    {"PPU FIFO Basic", test_ppu_fifo_basic},
    {"PPU FIFO Overflow", test_ppu_fifo_overflow},
    {"PPU Mode Timings", test_ppu_mode_timings},
    {"PPU STAT IRQ Transitions", test_ppu_stat_irq_transitions},
    {"PPU Window Edge Cases", test_ppu_window_edge_cases},
    {"PPU Sprite Priority Detailed", test_ppu_sprite_priority_detailed},
    {"PPU Fetcher Basic", test_ppu_fetcher_basic},
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

    // Avancer jusqu'à la fin de la ligne (mode sort de HBLANK)
    while (ppu.mode == PPU_MODE_HBLANK) {
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

    // Simuler les 10 lignes de VBLANK (144-153)
    for (int line = 144; line < 154; line++) {
        // Chaque ligne dure 456 cycles
        for (int cycle = 0; cycle < 456; cycle++) {
            u8 interrupts = ppu_tick(&ppu, 1, vram);
            (void)interrupts;
        }
        // Pour la ligne 153, on s'attend à un retour à 0 (début de frame)
        if (line == 153) {
            assert(ppu.ly == 0); // Retour au début de frame
        } else {
            assert(ppu.ly == line + 1); // Ligne suivante
        }
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

    // Vérifier les couleurs BGP=0xE4 (11100100)
    assert(ppu.bg_palette[0] == 0xFF); // Blanc (bits 1-0 = 00 → code 0)
    assert(ppu.bg_palette[1] == 0xAA); // Gris clair (bits 3-2 = 01 → code 1)
    assert(ppu.bg_palette[2] == 0x55); // Gris foncé (bits 5-4 = 10 → code 2)
    assert(ppu.bg_palette[3] == 0x00); // Noir (bits 7-6 = 11 → code 3)

    // Test fonction get_pixel_color
    u32 color0 = ppu_get_pixel_color(&ppu, 0);
    u32 color1 = ppu_get_pixel_color(&ppu, 1);
    u32 color2 = ppu_get_pixel_color(&ppu, 2);
    u32 color3 = ppu_get_pixel_color(&ppu, 3);

    assert(color0 == 0xFFFFFFFF); // Blanc
    assert(color1 == 0xAAAAAAFF); // Gris clair  
    assert(color2 == 0x555555FF); // Gris foncé
    assert(color3 == 0x000000FF); // Noir
}

void test_ppu_stat_irq_hblank(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    // Activer IRQ HBlank (STAT bit3)
    ppu.stat |= 0x08; // HBlank IRQ enable
    // Aller à Mode 3, puis basculer en HBlank et capter l'IRQ STAT
    ppu.mode = PPU_MODE_PIXEL_TRANSFER; ppu.mode_cycles = 171; ppu.ly = 0;
    u8 irq = ppu_tick(&ppu, 1, vram);
    assert(ppu.mode == PPU_MODE_HBLANK);
    assert(irq & 0x02); // STAT IRQ
}

void test_ppu_stat_irq_vblank(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    // Activer IRQ VBlank (STAT bit4)
    ppu.stat |= 0x10; // VBlank IRQ enable
    // Placer fin de ligne 143 pour entrer en VBlank
    ppu.ly = 143; ppu.mode = PPU_MODE_HBLANK; ppu.line_cycles = 456 - 1;
    u8 irq = ppu_tick(&ppu, 1, vram);
    assert(ppu.mode == PPU_MODE_VBLANK);
    assert(irq & 0x02); // STAT IRQ
    assert(irq & 0x01); // VBLANK IRQ
}

void test_ppu_stat_irq_oam(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    // Activer IRQ OAM (STAT bit5)
    ppu.stat |= 0x20; // OAM IRQ enable
    // Fin de HBlank ligne 0 → entrée OAM sur ligne 1 déclenche IRQ STAT
    ppu.mode = PPU_MODE_HBLANK; ppu.line_cycles = 456 - 1; ppu.ly = 0;
    u8 irq = ppu_tick(&ppu, 1, vram);
    assert(ppu.mode == PPU_MODE_OAM_SEARCH);
    assert(ppu.ly == 1);
    assert(irq & 0x02); // STAT IRQ
}

void test_ppu_stat_irq_lyc(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    // Activer IRQ LYC (STAT bit6)
    ppu.stat |= 0x40; // LYC IRQ enable
    // Forcer LYC = 1 et transition LY:0->1
    ppu.lyc = 1; ppu.ly = 0; ppu.mode = PPU_MODE_HBLANK; ppu.line_cycles = 456 - 1;
    // Première tick: fin ligne 0 -> ly=1, vérifier IRQ STAT
    u8 irq = ppu_tick(&ppu, 1, vram);
    assert(ppu.ly == 1);
    assert(ppu.stat & 0x04); // coincidence
    assert(irq & 0x02); // STAT IRQ (LYC)
}

void test_ppu_window_basic(void) {
    PPU ppu; u8 vram[0x2000];
    ppu_init(&ppu); memset(vram, 0, sizeof(vram));

    // Activer LCD et Window (LCDC bits7 et5)
    ppu.lcdc = 0x91 | 0x20; // LCD ON, BG ON, Window ON
    ppu.scx = 0; ppu.scy = 0;
    ppu.wy = 10; ppu.wx = 7; // WX = position écran +7 (Pan Docs)

    // Préparer BG et Window différentes pour les distinguer
    // BG: tile 0 -> vide (tout blanc)
    // Window map (0x9800 ou 0x9C00 selon bit6): on choisit 0x9C00
    ppu.lcdc |= 0x40; // Window map = 0x9C00

    // Remplir une tuile non blanche pour la Window (index 1)
    vram[0x1C00] = 0x01; // tile index 1 en haut-gauche de la window map
    // tile data index 1 à 0x8000 + 16
    vram[16 + 0] = 0xFF; vram[16 + 1] = 0x00; // pixels non blancs pour la ligne 0

    // Rendre la ligne 9 (avant WY): doit utiliser BG (blanc)
    ppu.ly = 9; ppu_render_line(&ppu, vram);
    bool all_white = true;
    for (int x = 0; x < GB_WIDTH; x++) {
        if (ppu.framebuffer[9 * GB_WIDTH + x] != 0xFFFFFFFF) { all_white = false; break; }
    }
    assert(all_white);

    // Rendre la ligne 10 (== WY): doit utiliser fenêtre à partir de WX-7
    ppu.ly = 10; ppu_render_line(&ppu, vram);
    bool window_pixels_seen = false;
    for (int x = (int)ppu.wx - 7; x < (int)ppu.wx - 7 + 8 && x < GB_WIDTH; x++) {
        if (x >= 0) {
            if (ppu.framebuffer[10 * GB_WIDTH + x] != 0xFFFFFFFF) { window_pixels_seen = true; break; }
        }
    }
    assert(window_pixels_seen);
}

void test_ppu_sprites_basic(void) {
    PPU ppu; u8 vram[0x2000];
    ppu_init(&ppu); memset(vram, 0, sizeof(vram));

    // Activer LCD et BG, OBJ (LCDC bits7,1)
    ppu.lcdc = 0x91 | 0x02; // LCD ON, BG ON, OBJ ON (8x8)

    // Sprite à X=8, Y=16 (affiché à l'écran à x=0..7, ligne 0)
    ppu.oam[0] = 16; // Y
    ppu.oam[1] = 8;  // X
    ppu.oam[2] = 1;  // tile index 1
    ppu.oam[3] = 0;  // flags

    // Tile data index 1 (0x8000 + 16)
    vram[16 + 0] = 0xFF; vram[16 + 1] = 0x00; // ligne 0 opaque

    // Rendre la ligne 0 (LY=0)
    ppu.ly = 0; ppu_render_line(&ppu, vram);

    // Des pixels non blancs autour de X=0..7 (X-8)
    bool sprite_pixels = false;
    for (int x = 0; x < 8 && x < GB_WIDTH; x++) {
        if (ppu.framebuffer[0 * GB_WIDTH + x] != 0xFFFFFFFF) { sprite_pixels = true; break; }
    }
    assert(sprite_pixels);
}

void test_ppu_sprites_priority_vs_bg(void) {
    PPU ppu; u8 vram[0x2000];
    ppu_init(&ppu); memset(vram, 0, sizeof(vram));

    // BG: tile 0 blanc; tile 1 noir
    ppu.lcdc = 0x91 | 0x02; // BG ON, OBJ ON
    ppu.ly = 0;
    // BG map 0x9800: mettre tile 1 à x=0..7 (pour croiser le sprite)
    vram[0x1800] = 1; // tile index 1 en (x=0..7, y=0)
    // tile data 1: pixels noirs
    vram[16 + 0] = 0xFF; vram[16 + 1] = 0xFF;

    // Sprite au même endroit, couleur non transparente
    ppu.oam[0] = 16; // Y
    ppu.oam[1] = 8;  // X (affiché à 0..7)
    ppu.oam[2] = 0;  // tile 0 (noir)
    ppu.oam[3] = 0x80; // derrière BG
    // tile 0 noir
    vram[0x0000] = 0xFF; vram[0x0001] = 0xFF;

    ppu_render_line(&ppu, vram);

    // Pixels 0..7 doivent rester BG (noirs) car sprite derrière et BG index != 0
    for (int x = 0; x < 8; x++) {
        assert(ppu.framebuffer[x] == 0x000000FF);
    }

    // Mettre BG transparent (index 0) sur cette zone pour laisser passer le sprite
    vram[0x1800] = 0; // tile 0 (blanc)
    ppu_render_line(&ppu, vram);
    bool sprite_seen = false;
    for (int x = 0; x < 8; x++) {
        if (ppu.framebuffer[x] == 0x000000FF) { sprite_seen = true; break; }
    }
    assert(sprite_seen);
}

void test_ppu_sprites_limit10_per_line(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    ppu.lcdc = 0x80 | 0x02; // LCD ON, BG OFF pour éviter interférence
    ppu.ly = 0;
    // Tuile 0 opaque pour sprites
    vram[0] = 0xFF; vram[1] = 0xFF;
    // Placer 12 sprites alignés sur la ligne 0
    for (int i = 0; i < 12; i++) {
        ppu.oam[i*4+0] = 16;            // Y
        ppu.oam[i*4+1] = (u8)(8 + i*8); // X (affiché à 0,8,16,...)
        ppu.oam[i*4+2] = 0;             // tile 0 (opaque)
        ppu.oam[i*4+3] = 0;             // flags
    }
    ppu_render_line(&ppu, vram);
    // Vérifier qu'au plus 10 groupes de 8 pixels ont été écrits
    int written_groups = 0;
    for (int i = 0; i < 12; i++) {
        int sx = (8 + i*8) - 8; // position écran
        if (sx < 0 || sx >= GB_WIDTH) continue;
        if (ppu.framebuffer[sx] != 0xFFFFFFFF) written_groups++;
    }
    assert(written_groups <= 10);
}

void test_ppu_sprites_flip_xy(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    ppu.lcdc = 0x91 | 0x02; ppu.ly = 0;
    // Tile 1: motif 1000 0001 (b1=1000 0001, b2=0)
    vram[16+0] = 0x81; vram[16+1] = 0x00;

    // Sprite non flip à X=8 (affiché 0..7)
    ppu.oam[0]=16; ppu.oam[1]=8; ppu.oam[2]=1; ppu.oam[3]=0;
    ppu_render_line(&ppu, vram);
    u32 left = ppu.framebuffer[0];
    u32 right = ppu.framebuffer[7];

    // Sprite flip X à X=24 (affiché 16..23)
    ppu.oam[4]=16; ppu.oam[5]=24; ppu.oam[6]=1; ppu.oam[7]=0x20; // X flip
    ppu_render_line(&ppu, vram);
    u32 left_flipped = ppu.framebuffer[16];
    u32 right_flipped = ppu.framebuffer[23];

    assert(left != 0xFFFFFFFF && right != 0xFFFFFFFF);
    assert(left_flipped != 0xFFFFFFFF && right_flipped != 0xFFFFFFFF);
    // Le flip doit inverser la distribution gauche/droite
    assert(left == right_flipped);
    assert(right == left_flipped);
}

void test_ppu_lcdc_off_on(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    // LCD OFF
    ppu.lcdc &= (u8)~0x80;
    // Rendu d'une ligne ne doit rien faire (reste blanc)
    ppu.ly = 0; ppu_render_line(&ppu, vram);
    for (int x = 0; x < GB_WIDTH; x++) {
        assert(ppu.framebuffer[x] == 0xFFFFFFFF);
    }
    // STAT doit refléter mode 0 et LYC cleared
    assert((ppu.stat & 0x03) == PPU_MODE_OAM_SEARCH || (ppu.stat & 0x03) == 0);

    // LCD ON
    ppu.lcdc |= 0x80;
    // Écrire une tile non blanche pour vérifier rendu
    vram[0x0000] = 0xFF; vram[0x0001] = 0x00;
    ppu.ly = 0; ppu_render_line(&ppu, vram);
    bool any = false; for (int x = 0; x < 8; x++) if (ppu.framebuffer[x] != 0xFFFFFFFF) { any = true; break; }
    assert(any);
}

void test_ppu_window_wx_clamp(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    ppu.lcdc = 0x91 | 0x20 | 0x40; // LCD,BG,WIN, window map 9C00
    ppu.wy = 0; ppu.wx = 0; // WX-7 = -7 -> clamp visuel au bord gauche
    vram[0x1C00] = 1; vram[16] = 0xFF; vram[17] = 0x00;
    ppu.ly = 0; ppu_render_line(&ppu, vram);
    // On s'attend à voir des pixels à partir de x=0 malgré WX<7
    bool seen = false; for (int x = 0; x < 8; x++) if (ppu.framebuffer[x] != 0xFFFFFFFF) { seen = true; break; }
    assert(seen);
}

void test_ppu_fine_scroll_scx(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    ppu.lcdc = 0x91; // LCD,BG ON, tiles 8000h, BG map 9800h
    
    // Créer une tile avec un pattern visible (pixel 0 = couleur 3)
    vram[0x0000] = 0xFF; vram[0x0001] = 0xFF; // Tile 0: tous pixels couleur 3
    
    // Test SCX = 0 (pas de décalage)
    ppu.scx = 0; ppu.ly = 0; ppu_render_line(&ppu, vram);
    u32 color0_scx0 = ppu.framebuffer[0];
    u32 color7_scx0 = ppu.framebuffer[7];
    
    // Test SCX = 3 (décalage de 3 pixels vers la droite)
    ppu.scx = 3; ppu.ly = 0; ppu_render_line(&ppu, vram);
    u32 color0_scx3 = ppu.framebuffer[0];
    u32 color4_scx3 = ppu.framebuffer[4];
    
    // Avec SCX=0: pixel 0 devrait être couleur 3, pixel 7 aussi
    assert(color0_scx0 == ppu_get_pixel_color(&ppu, 3));
    assert(color7_scx0 == ppu_get_pixel_color(&ppu, 3));
    
    // Avec SCX=3: pixel 0 lit le pixel 3 de la tile (qui est couleur 3)
    // pixel 4 lit le pixel 7 de la tile (qui est couleur 3)
    assert(color0_scx3 == ppu_get_pixel_color(&ppu, 3)); // Pixel 3 de la tile
    assert(color4_scx3 == ppu_get_pixel_color(&ppu, 3)); // Pixel 7 de la tile
}

void test_ppu_fifo_basic(void) {
    PPU ppu; ppu_init(&ppu);
    
    // Test BG FIFO vide
    assert(ppu_bg_fifo_empty(&ppu));
    assert(!ppu_bg_fifo_full(&ppu));
    
    // Ajouter un pixel BG
    PixelFIFOEntry bg_pixel = {3, 0, false, false}; // Couleur 3, palette BG, pas sprite
    ppu_bg_fifo_push(&ppu, bg_pixel);
    assert(!ppu_bg_fifo_empty(&ppu));
    assert(!ppu_bg_fifo_full(&ppu));
    assert(ppu.bg_fifo_size == 1);
    
    // Ajouter un pixel sprite
    PixelFIFOEntry sprite_pixel = {2, 1, true, false}; // Couleur 2, palette OBP0, sprite
    ppu_sprite_fifo_push(&ppu, sprite_pixel);
    assert(ppu.sprite_fifo_size == 1);
    
    // Retirer les pixels (ordre FIFO)
    PixelFIFOEntry popped;
    assert(ppu_bg_fifo_pop(&ppu, &popped));
    assert(popped.color_index == 3);
    assert(popped.palette == 0);
    assert(!popped.sprite_priority);
    
    assert(ppu_sprite_fifo_pop(&ppu, &popped));
    assert(popped.color_index == 2);
    assert(popped.palette == 1);
    assert(popped.sprite_priority);
    
    // FIFOs vides après retrait
    assert(ppu_bg_fifo_empty(&ppu));
    assert(ppu_sprite_fifo_empty(&ppu));
    assert(!ppu_bg_fifo_pop(&ppu, &popped)); // Pas de pixel à retirer
    assert(!ppu_sprite_fifo_pop(&ppu, &popped)); // Pas de pixel à retirer
}

void test_ppu_fifo_overflow(void) {
    PPU ppu; ppu_init(&ppu);
    
    // Remplir la BG FIFO au maximum (16 pixels)
    PixelFIFOEntry pixel = {1, 0, false, false};
    for (int i = 0; i < 16; i++) {
        ppu_bg_fifo_push(&ppu, pixel);
        assert(ppu.bg_fifo_size == i + 1);
    }
    
    assert(ppu_bg_fifo_full(&ppu));
    assert(!ppu_bg_fifo_empty(&ppu));
    
    // Essayer d'ajouter un pixel de plus (devrait être ignoré)
    ppu_bg_fifo_push(&ppu, pixel);
    assert(ppu.bg_fifo_size == 16); // Taille inchangée
    assert(ppu_bg_fifo_full(&ppu));
    
    // Vider les FIFOs
    ppu_fifos_clear(&ppu);
    assert(ppu_bg_fifo_empty(&ppu));
    assert(ppu_sprite_fifo_empty(&ppu));
    assert(!ppu_bg_fifo_full(&ppu));
    assert(ppu.bg_fifo_size == 0);
    assert(ppu.sprite_fifo_size == 0);
}

void test_ppu_mode_timings(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    ppu.lcdc = 0x91; // LCD ON, BG ON
    ppu.stat |= 0x08; // Activer IRQ HBlank
    
    // Test timing d'une ligne complète (456 cycles)
    ppu.ly = 0; ppu.mode = PPU_MODE_OAM_SEARCH; ppu.mode_cycles = 0; ppu.line_cycles = 0;
    
    // Mode 2 (OAM Search) : 80 cycles
    u8 interrupts = ppu_tick(&ppu, 79, vram);
    assert(ppu.mode == PPU_MODE_OAM_SEARCH);
    assert(ppu.mode_cycles == 79);
    
    interrupts = ppu_tick(&ppu, 1, vram);
    assert(ppu.mode == PPU_MODE_PIXEL_TRANSFER);
    assert(ppu.mode_cycles == 0);
    
    // Mode 3 (Pixel Transfer) : 172 cycles
    interrupts = ppu_tick(&ppu, 171, vram);
    assert(ppu.mode == PPU_MODE_PIXEL_TRANSFER);
    assert(ppu.mode_cycles == 171);
    
    interrupts = ppu_tick(&ppu, 1, vram);
    assert(ppu.mode == PPU_MODE_HBLANK);
    assert(ppu.mode_cycles == 0);
    assert(interrupts & 0x02); // STAT IRQ HBlank
    
    // Mode 0 (HBlank) : 204 cycles restants
    interrupts = ppu_tick(&ppu, 203, vram);
    assert(ppu.mode == PPU_MODE_HBLANK);
    assert(ppu.line_cycles == 455);
    
    // Fin de ligne (456 cycles total)
    interrupts = ppu_tick(&ppu, 1, vram);
    assert(ppu.ly == 1);
    assert(ppu.mode == PPU_MODE_OAM_SEARCH);
    assert(ppu.line_cycles == 0);
    assert(ppu.mode_cycles == 0);
    
    // Test VBlank (ligne 144)
    ppu.ly = 143; ppu.mode = PPU_MODE_HBLANK; ppu.line_cycles = 455;
    interrupts = ppu_tick(&ppu, 1, vram);
    assert(ppu.ly == 144);
    assert(ppu.mode == PPU_MODE_VBLANK);
    assert(interrupts & 0x01); // VBLANK IRQ
    
    // Test fin VBlank (ligne 153 -> 0)
    ppu.ly = 153; ppu.line_cycles = 455;
    interrupts = ppu_tick(&ppu, 1, vram);
    assert(ppu.ly == 0);
    assert(ppu.mode == PPU_MODE_OAM_SEARCH);
}

void test_ppu_stat_irq_transitions(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    ppu.lcdc = 0x91; // LCD ON, BG ON
    
    // Test IRQ HBlank (bit 3)
    ppu.stat = 0x08; // HBlank IRQ activé
    ppu.ly = 0; ppu.mode = PPU_MODE_PIXEL_TRANSFER; ppu.mode_cycles = 171;
    u8 interrupts = ppu_tick(&ppu, 1, vram);
    assert(ppu.mode == PPU_MODE_HBLANK);
    assert(interrupts & 0x02); // STAT IRQ déclenché
    
    // Test IRQ VBlank (bit 4)
    ppu.stat = 0x10; // VBlank IRQ activé
    ppu.ly = 143; ppu.mode = PPU_MODE_HBLANK; ppu.line_cycles = 455;
    interrupts = ppu_tick(&ppu, 1, vram);
    assert(ppu.mode == PPU_MODE_VBLANK);
    assert(interrupts & 0x01); // VBLANK IRQ
    assert(interrupts & 0x02); // STAT IRQ aussi
    
    // Test IRQ OAM (bit 5)
    ppu.stat = 0x20; // OAM IRQ activé
    ppu.ly = 0; ppu.mode = PPU_MODE_HBLANK; ppu.line_cycles = 455;
    interrupts = ppu_tick(&ppu, 1, vram);
    assert(ppu.mode == PPU_MODE_OAM_SEARCH);
    assert(interrupts & 0x02); // STAT IRQ déclenché
    
    // Test IRQ LYC (bit 6) - transition 0->1
    ppu.stat = 0x40; // LYC IRQ activé
    ppu.lyc = 5; ppu.ly = 4; ppu.lyc_prev_eq = false;
    ppu.mode = PPU_MODE_HBLANK; ppu.line_cycles = 455;
    interrupts = ppu_tick(&ppu, 1, vram);
    assert(ppu.ly == 5);
    assert(ppu.stat & 0x04); // LYC==LY bit set
    assert(interrupts & 0x02); // STAT IRQ LYC déclenché
    
    // Test pas d'IRQ LYC si déjà égal
    ppu.lyc_prev_eq = true; // Déjà égal
    interrupts = ppu_tick(&ppu, 1, vram);
    assert(!(interrupts & 0x02)); // Pas d'IRQ
}

void test_ppu_window_edge_cases(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    ppu.lcdc = 0x91 | 0x20; // LCD,BG,WIN ON
    
    // Test WY = 0 (window dès la première ligne)
    ppu.wy = 0; ppu.wx = 7; // WX-7 = 0
    vram[0x1800] = 1; // Tile 1 dans window map 9800h
    vram[16] = 0xFF; vram[17] = 0xFF; // Tile 1: couleur 3
    ppu.ly = 0; ppu_render_line(&ppu, vram);
    assert(ppu.framebuffer[0] == ppu_get_pixel_color(&ppu, 3));
    
    // Test WY = 144 (window après l'écran)
    ppu.wy = 144; ppu.wx = 7;
    ppu.ly = 0; 
    // Nettoyer le framebuffer avant le test
    for (int i = 0; i < GB_WIDTH * GB_HEIGHT; i++) {
        ppu.framebuffer[i] = 0xFFFFFFFF;
    }
    ppu_render_line(&ppu, vram);
    assert(ppu.framebuffer[0] == ppu_get_pixel_color(&ppu, 0)); // Background (pas de window)
    
    // Test WX = 0 (WX-7 = -7, window invisible)
    ppu.wy = 0; ppu.wx = 0;
    ppu.ly = 0; 
    // Nettoyer le framebuffer avant le test
    for (int i = 0; i < GB_WIDTH * GB_HEIGHT; i++) {
        ppu.framebuffer[i] = 0xFFFFFFFF;
    }
    ppu_render_line(&ppu, vram);
    assert(ppu.framebuffer[0] == ppu_get_pixel_color(&ppu, 0)); // Background (pas de window)
    
    // Test WX = 166 (WX-7 = 159, window à droite)
    ppu.wy = 0; ppu.wx = 166;
    ppu.ly = 0; 
    // Nettoyer le framebuffer avant le test
    for (int i = 0; i < GB_WIDTH * GB_HEIGHT; i++) {
        ppu.framebuffer[i] = 0xFFFFFFFF;
    }
    ppu_render_line(&ppu, vram);
    assert(ppu.framebuffer[0] == ppu_get_pixel_color(&ppu, 0)); // Background (pas de window)
    
    // Test window map 9C00h (bit 6 de LCDC)
    ppu.lcdc = 0x91 | 0x20 | 0x40; // Window map 9C00h
    ppu.wy = 0; ppu.wx = 7;
    vram[0x1C00] = 2; // Tile 2 dans window map 9C00h
    vram[32] = 0xFF; vram[33] = 0xFF; // Tile 2: couleur 3
    ppu.ly = 0; ppu_render_line(&ppu, vram);
    assert(ppu.framebuffer[0] == ppu_get_pixel_color(&ppu, 3));
    
    // Test tile selection 8800h (bit 4 de LCDC = 0)
    ppu.lcdc = 0x91 | 0x20; // Tile selection 8800h
    ppu.wy = 0; ppu.wx = 7;
    vram[0x1800] = 0x80; // Tile -128 (0x80) dans window map
    vram[0] = 0xFF; vram[1] = 0xFF; // Tile -128: couleur 3
    ppu.ly = 0; ppu_render_line(&ppu, vram);
    assert(ppu.framebuffer[0] == ppu_get_pixel_color(&ppu, 3));
}

void test_ppu_sprite_priority_detailed(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    ppu.lcdc = 0x91 | 0x02; // LCD,BG,OBJ ON
    
    // Créer un BG avec couleur 1
    vram[0x0000] = 0xFF; vram[0x0001] = 0x00; // Tile 0: couleur 1
    vram[0x9800] = 0; // BG map: tile 0
    
    // Sprite 0: couleur 2, priorité normale (devant BG)
    ppu.oam[0] = 20; // SY = 20 (Y = 4)
    ppu.oam[1] = 12; // SX = 12 (X = 4)
    ppu.oam[2] = 0;  // Tile 0
    ppu.oam[3] = 0;  // Attr: priorité normale, OBP0
    
    // Sprite 1: couleur 3, derrière BG (bit 7 = 1)
    ppu.oam[4] = 20; // SY = 20 (Y = 4)
    ppu.oam[5] = 20; // SX = 20 (X = 12)
    ppu.oam[6] = 0;  // Tile 0
    ppu.oam[7] = 0x80; // Attr: derrière BG, OBP0
    
    // Créer les tiles de sprites
    vram[0x0000] = 0xFF; vram[0x0001] = 0x00; // Tile 0: couleur 1
    vram[0x0010] = 0xFF; vram[0x0011] = 0xFF; // Tile 0: couleur 3
    
    ppu.ly = 4; ppu_render_line(&ppu, vram);
    
    // Pixel 4: sprite 0 (couleur 2) devant BG (couleur 1)
    assert(ppu.framebuffer[4] == ppu_get_obj_color(&ppu, 2, false));
    
    // Pixel 12: sprite 1 (couleur 3) derrière BG (couleur 1)
    assert(ppu.framebuffer[12] == ppu_get_pixel_color(&ppu, 1)); // BG visible
    
    // Test transparence (couleur 0)
    ppu.oam[8] = 20; // SY = 20 (Y = 4)
    ppu.oam[9] = 28; // SX = 28 (X = 20)
    ppu.oam[10] = 0; // Tile 0
    ppu.oam[11] = 0; // Attr: priorité normale, OBP0
    
    // Tile avec couleur 0 (transparente)
    vram[0x0020] = 0x00; vram[0x0021] = 0x00; // Tile 0: couleur 0 (transparente)
    
    ppu.ly = 4; ppu_render_line(&ppu, vram);
    
    // Pixel 20: sprite transparent, BG visible
    assert(ppu.framebuffer[20] == ppu_get_pixel_color(&ppu, 1)); // BG visible
}

void test_ppu_fetcher_basic(void) {
    PPU ppu; u8 vram[0x2000]; ppu_init(&ppu); memset(vram, 0, sizeof(vram));
    ppu.lcdc = 0x91; // LCD ON, BG ON, tiles 8000h, BG map 9800h
    
    // Créer une tile avec pattern visible
    vram[0x0000] = 0xFF; vram[0x0001] = 0xFF; // Tile 0: couleur 3
    vram[0x9800] = 0; // BG map: tile 0
    
    // Démarrer le fetcher pour tile (0,0), ligne 0
    ppu_fetcher_start(&ppu, 0, 0, 0, false);
    assert(ppu.fetcher.state == FETCHER_GET_TILE);
    assert(ppu.fetcher.state_cycles == 0);
    
    // Simuler les cycles du fetcher
    for (int cycle = 0; cycle < 10; cycle++) {
        ppu_fetcher_tick(&ppu, vram);
    }
    
    // Vérifier que des pixels ont été poussés dans la BG FIFO
    assert(!ppu_bg_fifo_empty(&ppu));
    assert(ppu.bg_fifo_size == 8); // 8 pixels poussés
    
    // Vérifier le contenu des pixels
    PixelFIFOEntry pixel;
    for (int i = 0; i < 8; i++) {
        assert(ppu_bg_fifo_pop(&ppu, &pixel));
        assert(pixel.color_index == 3); // Couleur 3
        assert(pixel.palette == 0); // BG palette
        assert(!pixel.sprite_priority);
    }
    
    assert(ppu_bg_fifo_empty(&ppu));
}

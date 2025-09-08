// GBDK: carré centré de 12x12 pixels sur le BG
// Implémentation: 4 tuiles partiellement remplies autour du centre (80,72)
#include <gb/gb.h>
#include <gb/hardware.h>

// Tuile partielle: notation (lo,hi) par ligne, avec bits à 1 sur la zone du carré
// Carré 12x12: x = 74..85, y = 66..77 (centre 80,72, demi-côté 6)
// Tuiles touchées: (9,8), (10,8), (9,9), (10,9)

// (9,8): lignes locales 2..7, colonnes locales 2..7 => masque 0x3F
static const unsigned char tile_9_8[16] = {
    0x00,0x00,
    0x00,0x00,
    0x3F,0x3F,
    0x3F,0x3F,
    0x3F,0x3F,
    0x3F,0x3F,
    0x3F,0x3F,
    0x3F,0x3F,
};

// (10,8): lignes locales 2..7, colonnes locales 0..5 => masque 0xFC
static const unsigned char tile_10_8[16] = {
    0x00,0x00,
    0x00,0x00,
    0xFC,0xFC,
    0xFC,0xFC,
    0xFC,0xFC,
    0xFC,0xFC,
    0xFC,0xFC,
    0xFC,0xFC,
};

// (9,9): lignes locales 0..5, colonnes locales 2..7 => masque 0x3F
static const unsigned char tile_9_9[16] = {
    0x3F,0x3F,
    0x3F,0x3F,
    0x3F,0x3F,
    0x3F,0x3F,
    0x3F,0x3F,
    0x3F,0x3F,
    0x00,0x00,
    0x00,0x00,
};

// (10,9): lignes locales 0..5, colonnes locales 0..5 => masque 0xFC
static const unsigned char tile_10_9[16] = {
    0xFC,0xFC,
    0xFC,0xFC,
    0xFC,0xFC,
    0xFC,0xFC,
    0xFC,0xFC,
    0xFC,0xFC,
    0x00,0x00,
    0x00,0x00,
};

void main(void) {
    disable_interrupts();
    DISPLAY_OFF;

    // Palette DMG (clair->foncé): 11 10 01 00
    BGP_REG = 0xE4;

    // Effacer la tilemap
    fill_bkg_rect(0, 0, 20, 18, 0);

    // Charger nos 4 tuiles (index 0..3) de manière contiguë
    // NOTE: si l'orientation des bits 7..0 dans le PPU place le bit7 à gauche (classique GB),
    // alors pour que les deux moitiés se joignent au centre, la tuile de gauche doit utiliser 0xFC
    // (colonnes locales 0..5) et celle de droite 0x3F (colonnes locales 2..7).
    static const unsigned char tiles_all[64] = {
        // tile 0 (9,8) haut-gauche: colonnes 0..5 => 0xFC sur lignes 2..7
        0x00,0x00, 0x00,0x00, 0xFC,0xFC, 0xFC,0xFC,
        0xFC,0xFC, 0xFC,0xFC, 0xFC,0xFC, 0xFC,0xFC,
        // tile 1 (10,8) haut-droite: colonnes 2..7 => 0x3F sur lignes 2..7
        0x00,0x00, 0x00,0x00, 0x3F,0x3F, 0x3F,0x3F,
        0x3F,0x3F, 0x3F,0x3F, 0x3F,0x3F, 0x3F,0x3F,
        // tile 2 (9,9) bas-gauche: colonnes 0..5 => 0xFC sur lignes 0..5
        0xFC,0xFC, 0xFC,0xFC, 0xFC,0xFC, 0xFC,0xFC,
        0xFC,0xFC, 0xFC,0xFC, 0x00,0x00, 0x00,0x00,
        // tile 3 (10,9) bas-droite: colonnes 2..7 => 0x3F sur lignes 0..5
        0x3F,0x3F, 0x3F,0x3F, 0x3F,0x3F, 0x3F,0x3F,
        0x3F,0x3F, 0x3F,0x3F, 0x00,0x00, 0x00,0x00,
    };
    set_bkg_data(0, 4, tiles_all);

    // Poser les tuiles aux positions (x=9..10, y=8..9)
    unsigned char t;
    t = 0; set_bkg_tiles(9, 8, 1, 1, &t);   // 9,8 (HG)
    t = 1; set_bkg_tiles(10, 8, 1, 1, &t);  // 10,8 (HD)
    t = 2; set_bkg_tiles(9, 9, 1, 1, &t);   // 9,9 (BG)
    t = 3; set_bkg_tiles(10, 9, 1, 1, &t);  // 10,9 (BD)

    SHOW_BKG;
    DISPLAY_ON;
    enable_interrupts();
    while (1) wait_vbl_done();
}

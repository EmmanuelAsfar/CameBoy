#include <gb/gb.h>
#include <stdio.h>

// Palette de couleurs (noir et blanc)
const UWORD palette[] = {
    RGB(31, 31, 31),  // Blanc
    RGB(0, 0, 0),     // Noir
    RGB(0, 0, 0),     // Noir
    RGB(0, 0, 0)      // Noir
};

// Données de la tile pour un carré noir (8x8 pixels)
const UBYTE tile_data[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // 8 lignes de pixels noirs
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

void main() {
    // Initialiser le système
    DISPLAY_OFF;
    
    // Charger la palette
    set_bkg_palette(0, 1, palette);
    
    // Charger les données de tile dans la VRAM
    set_bkg_data(1, 1, tile_data);
    
    // Effacer l'écran
    fill_bkg_rect(0, 0, 20, 18, 0);
    
    // Dessiner la croix (deux diagonales)
    // Diagonale principale (de haut-gauche à bas-droite)
    for (int i = 0; i < 18; i++) {
        set_bkg_tile(i, i, 1);  // Tile 1 = carré noir
    }
    
    // Diagonale secondaire (de haut-droite à bas-gauche)
    for (int i = 0; i < 18; i++) {
        set_bkg_tile(17 - i, i, 1);  // Tile 1 = carré noir
    }
    
    // Activer l'affichage
    SHOW_BKG;
    DISPLAY_ON;
    
    // Boucle infinie
    while(1) {
        wait_vbl_done();
    }
}

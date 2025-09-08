// Exemple GBDK: affiche une croix en tuiles sur l'écran
// Remarques:
// - Compatible DMG (pas de fonctions CGB comme set_bkg_palette)
// - Utilise set_bkg_tiles() pour poser une tuile à une position donnée

#include <gb/gb.h>
#include <stdio.h>

// Données d'une tuile 8x8 entièrement « encré » (16 octets, 2 plans/ligne)
// Chaque ligne: [plan0, plan1]. 0xFF/0xFF => valeur de pixel 3 (foncé) partout.
const UBYTE tile_black[16] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

void main(void) {
    // Préparer l'affichage
    DISPLAY_OFF;

    // Charger 1 tuile (index 1) dans la VRAM
    set_bkg_data(1, 1, tile_black);

    // Effacer la tilemap en mettant des tuiles 0
    fill_bkg_rect(0, 0, 20, 18, 0);

    // Dessiner la croix (deux diagonales) avec la tuile 1
    UBYTE t = 1;
    // Diagonale principale (haut-gauche -> bas-droite)
    for (UINT8 i = 0; i < 18; i++) {
        set_bkg_tiles(i, i, 1, 1, &t);
    }
    // Diagonale secondaire (haut-droite -> bas-gauche)
    for (UINT8 i = 0; i < 18; i++) {
        UINT8 x = 17u - i;
        set_bkg_tiles(x, i, 1, 1, &t);
    }

    SHOW_BKG;
    DISPLAY_ON;

    // Boucle principale
    while (1) {
        wait_vbl_done();
    }
}

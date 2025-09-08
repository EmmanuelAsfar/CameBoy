// Exemple GBDK: simple printf vers la console GB/Emu
// Remarques:
// - printf s'affiche dans la fenêtre/console supportée par l'émulateur
// - La boucle attend les VBlank pour laisser le PPU respirer

#include <gb/gb.h>
#include <stdio.h>

void main(void) {
    printf("Hello, GBDK!\n");
    while (1) {
        wait_vbl_done();
    }
}



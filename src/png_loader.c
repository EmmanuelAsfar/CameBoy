#include "png_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Version simplifiée sans libpng pour l'instant
// On va utiliser une approche hybride : charger le PNG et le convertir en BMP

PNGImage* png_load_from_file(const char* filename) {
    printf("PNG Loader: Tentative de chargement de %s\n", filename);
    
    // Pour l'instant, on va essayer de charger avec LoadImage
    // Si ça ne marche pas, on implémentera libpng plus tard
    HBITMAP hBitmap = (HBITMAP)LoadImageA(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    
    if (!hBitmap) {
        printf("PNG Loader: LoadImage ne peut pas charger %s (format non supporté)\n", filename);
        return NULL;
    }
    
    // Si LoadImage a réussi, c'est que le fichier était en fait un BMP
    PNGImage* image = malloc(sizeof(PNGImage));
    if (!image) {
        DeleteObject(hBitmap);
        return NULL;
    }
    
    image->hBitmap = hBitmap;
    image->has_alpha = false; // LoadImage ne gère pas l'alpha
    
    // Obtenir les dimensions
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);
    image->width = bm.bmWidth;
    image->height = bm.bmHeight;
    
    printf("PNG Loader: Image chargée %dx%d\n", image->width, image->height);
    return image;
}

void png_free(PNGImage* image) {
    if (image) {
        if (image->hBitmap) {
            DeleteObject(image->hBitmap);
        }
        free(image);
    }
}

HBITMAP png_to_hbitmap(PNGImage* image) {
    if (!image || !image->hBitmap) {
        return NULL;
    }
    
    // Pour l'instant, on retourne directement le HBITMAP
    // Plus tard, on pourra faire une conversion PNG -> HBITMAP
    return image->hBitmap;
}

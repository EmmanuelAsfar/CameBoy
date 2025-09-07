#ifndef PNG_LOADER_H
#define PNG_LOADER_H

#include "common.h"
#include <windows.h>

// Structure pour une image PNG chargée
typedef struct {
    HBITMAP hBitmap;
    int width;
    int height;
    bool has_alpha;
} PNGImage;

// Charger une image PNG depuis un fichier
PNGImage* png_load_from_file(const char* filename);

// Libérer une image PNG
void png_free(PNGImage* image);

// Convertir PNG en HBITMAP pour Windows
HBITMAP png_to_hbitmap(PNGImage* image);

#endif // PNG_LOADER_H

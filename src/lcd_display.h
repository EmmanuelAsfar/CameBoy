#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "common.h"

// Dimensions du LCD Game Boy
#define LCD_WIDTH 160
#define LCD_HEIGHT 144
#define LCD_SCALE 4  // Facteur d'agrandissement

// Structure pour l'affichage LCD
typedef struct {
    HWND hwnd;
    HDC hdc;
    HBITMAP hbitmap;
    u32* framebuffer;
    bool is_initialized;
} LCDDisplay;

// Fonctions d'affichage LCD
bool lcd_display_init(LCDDisplay* display);
void lcd_display_cleanup(LCDDisplay* display);
void lcd_display_update(LCDDisplay* display, u32* framebuffer);
void lcd_display_show(LCDDisplay* display);
void lcd_display_hide(LCDDisplay* display);

// Callback de la fenêtre
LRESULT CALLBACK lcd_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#endif // LCD_DISPLAY_H

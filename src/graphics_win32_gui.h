#ifndef GRAPHICS_WIN32_GUI_H
#define GRAPHICS_WIN32_GUI_H

#include "common.h"
#include "joypad.h"
#include <windows.h>

// Callback pour la sortie série
typedef void (*mmu_serial_cb_t)(u8 ch);

// Structure pour l'interface graphique Win32 avec GUI complète
typedef struct {
    // Fenêtre principale
    HWND hwnd;
    HDC hdc;
    HBITMAP hbitmap;
    BITMAPINFO bmi;
    u8* framebuffer;
    bool running;
    int width;
    int height;
    int scale;
    bool visible;
    
    // Image de fond Game Boy
    HBITMAP hBackground;
    int bg_width, bg_height;
    
    // Zone LCD (position dans l'image de fond)
    int lcd_x, lcd_y, lcd_w, lcd_h;
    
    // Panneaux de texte
    HWND hSerial;      // Port série en haut à droite
    HWND hLogs;        // Logs en bas à droite
    HFONT hFont;
    char serial_buffer[8192];
    int serial_length;
    char logs_buffer[8192];
    int logs_length;
    
    // Boutons Game Boy
    HWND btnA, btnB, btnStart, btnSelect;
    HWND btnUp, btnDown, btnLeft, btnRight;
    
    // Joypad lié
    Joypad* joypad;
    
    // Callback série
    mmu_serial_cb_t serial_callback;
} GraphicsWin32GUI;

// Fonctions graphiques Win32 GUI
bool graphics_win32_gui_init(GraphicsWin32GUI* gfx);
void graphics_win32_gui_cleanup(GraphicsWin32GUI* gfx);
void graphics_win32_gui_update(GraphicsWin32GUI* gfx, u32* ppu_framebuffer);
void graphics_win32_gui_present(GraphicsWin32GUI* gfx);
void graphics_win32_gui_handle_events(GraphicsWin32GUI* gfx, bool* running);
void graphics_win32_gui_show(GraphicsWin32GUI* gfx);
void graphics_win32_gui_hide(GraphicsWin32GUI* gfx);

// Fonctions spécifiques GUI
void graphics_win32_gui_bind_joypad(GraphicsWin32GUI* gfx, Joypad* joypad);
void graphics_win32_gui_append_serial(GraphicsWin32GUI* gfx, const char* text, int length);
void graphics_win32_gui_append_logs(GraphicsWin32GUI* gfx, const char* text, int length);
void graphics_win32_gui_set_serial_callback(GraphicsWin32GUI* gfx, mmu_serial_cb_t callback);

#endif // GRAPHICS_WIN32_GUI_H

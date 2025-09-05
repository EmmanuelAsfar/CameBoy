#ifndef GRAPHICS_WIN32_H
#define GRAPHICS_WIN32_H

#include "common.h"
#include <windows.h>

// Structure pour l'interface graphique Win32
typedef struct {
    HWND hwnd;
    HDC hdc;
    HBITMAP hbitmap;
    BITMAPINFO bmi;
    u8* framebuffer;
    bool running;
    int width;
    int height;
    bool visible;  // Pour contrôler la visibilité de la fenêtre
} GraphicsWin32;

// Fonctions graphiques Win32
bool graphics_win32_init(GraphicsWin32* gfx);
void graphics_win32_cleanup(GraphicsWin32* gfx);
void graphics_win32_update(GraphicsWin32* gfx, u32* ppu_framebuffer);
void graphics_win32_present(GraphicsWin32* gfx);
void graphics_win32_handle_events(GraphicsWin32* gfx, bool* running);
void graphics_win32_show(GraphicsWin32* gfx);
void graphics_win32_hide(GraphicsWin32* gfx);

#endif // GRAPHICS_WIN32_H

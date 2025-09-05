#include "graphics_win32.h"
#include <windows.h>

// Fenêtre de message pour les événements
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    GraphicsWin32* gfx = (GraphicsWin32*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (uMsg) {
        case WM_CLOSE:
            if (gfx) gfx->running = false;
            PostQuitMessage(0);
            return 0;
            
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                if (gfx) gfx->running = false;
                PostQuitMessage(0);
            }
            return 0;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            if (gfx && gfx->hbitmap) {
                // Afficher le framebuffer
                SetDIBitsToDevice(hdc, 0, 0, gfx->width, gfx->height,
                                 0, 0, 0, gfx->height,
                                 gfx->framebuffer, &gfx->bmi, DIB_RGB_COLORS);
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Initialisation de l'interface graphique Win32
bool graphics_win32_init(GraphicsWin32* gfx) {
    memset(gfx, 0, sizeof(GraphicsWin32));
    
    gfx->width = 160;   // Largeur Game Boy
    gfx->height = 144;  // Hauteur Game Boy
    gfx->running = true;
    gfx->visible = false;  // Commencer caché
    
    // Allouer le framebuffer
    gfx->framebuffer = calloc(gfx->width * gfx->height * 3, 1);
    if (!gfx->framebuffer) {
        printf("Erreur: Impossible d'allouer le framebuffer\n");
        return false;
    }
    
    // Configuration du BITMAPINFO
    gfx->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    gfx->bmi.bmiHeader.biWidth = gfx->width;
    gfx->bmi.bmiHeader.biHeight = -gfx->height; // Top-down
    gfx->bmi.bmiHeader.biPlanes = 1;
    gfx->bmi.bmiHeader.biBitCount = 24;
    gfx->bmi.bmiHeader.biCompression = BI_RGB;
    
    // Enregistrer la classe de fenêtre
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "CameBoy";
    
    if (!RegisterClassEx(&wc)) {
        printf("Erreur: Impossible d'enregistrer la classe de fenêtre\n");
        free(gfx->framebuffer);
        return false;
    }
    
    // Créer la fenêtre (cachée par défaut)
    gfx->hwnd = CreateWindowEx(
        0,
        "CameBoy",
        "CameBoy - Game Boy LCD",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        gfx->width * 4, gfx->height * 4, // 4x zoom
        NULL, NULL,
        GetModuleHandle(NULL),
        NULL
    );
    
    if (!gfx->hwnd) {
        printf("Erreur: Impossible de créer la fenêtre\n");
        free(gfx->framebuffer);
        return false;
    }
    
    // Stocker le pointeur vers GraphicsWin32
    SetWindowLongPtr(gfx->hwnd, GWLP_USERDATA, (LONG_PTR)gfx);
    
    // Obtenir le HDC
    gfx->hdc = GetDC(gfx->hwnd);
    
    // Créer le bitmap
    gfx->hbitmap = CreateCompatibleBitmap(gfx->hdc, gfx->width, gfx->height);
    
    // Afficher la fenêtre
    ShowWindow(gfx->hwnd, SW_SHOW);
    UpdateWindow(gfx->hwnd);
    
    return true;
}

// Nettoyage de l'interface graphique
void graphics_win32_cleanup(GraphicsWin32* gfx) {
    if (gfx->hbitmap) {
        DeleteObject(gfx->hbitmap);
    }
    
    if (gfx->hdc) {
        ReleaseDC(gfx->hwnd, gfx->hdc);
    }
    
    if (gfx->hwnd) {
        DestroyWindow(gfx->hwnd);
    }
    
    if (gfx->framebuffer) {
        free(gfx->framebuffer);
        gfx->framebuffer = NULL;
    }
}

// Mettre à jour le framebuffer
void graphics_win32_update(GraphicsWin32* gfx, u32* ppu_framebuffer) {
    if (!gfx || !ppu_framebuffer) return;
    
    // Copier les données du framebuffer PPU (u32 ARGB) vers notre framebuffer RGB
    for (int y = 0; y < gfx->height; y++) {
        for (int x = 0; x < gfx->width; x++) {
            int src_idx = y * gfx->width + x;
            int dst_idx = (y * gfx->width + x) * 3;
            
            // Convertir de u32 ARGB vers RGB
            u32 pixel = ppu_framebuffer[src_idx];
            u8 r = (pixel >> 16) & 0xFF;
            u8 g = (pixel >> 8) & 0xFF;
            u8 b = pixel & 0xFF;
            
            gfx->framebuffer[dst_idx + 0] = b; // B
            gfx->framebuffer[dst_idx + 1] = g; // G
            gfx->framebuffer[dst_idx + 2] = r; // R
        }
    }
}

// Afficher le framebuffer
void graphics_win32_present(GraphicsWin32* gfx) {
    if (!gfx || !gfx->hwnd) return;
    
    // Forcer le redessin de la fenêtre
    InvalidateRect(gfx->hwnd, NULL, FALSE);
    UpdateWindow(gfx->hwnd);
}

// Gérer les événements
void graphics_win32_handle_events(GraphicsWin32* gfx, bool* running) {
    if (!gfx || !running) return;
    
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            *running = false;
            gfx->running = false;
            break;
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Afficher la fenêtre
void graphics_win32_show(GraphicsWin32* gfx) {
    if (!gfx || !gfx->hwnd) return;
    
    gfx->visible = true;
    ShowWindow(gfx->hwnd, SW_SHOW);
    UpdateWindow(gfx->hwnd);
}

// Cacher la fenêtre
void graphics_win32_hide(GraphicsWin32* gfx) {
    if (!gfx || !gfx->hwnd) return;
    
    gfx->visible = false;
    ShowWindow(gfx->hwnd, SW_HIDE);
}

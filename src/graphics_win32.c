#include "graphics_win32.h"
#include <windows.h>

// Fenêtre de message pour les événements
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    GraphicsWin32* gfx = (GraphicsWin32*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (uMsg) {
        case WM_CLOSE:
            // Ignorer la fermeture via la croix pour maintenir la fenêtre ouverte
            // (Utiliser Échap pour quitter proprement)
            OutputDebugStringA("WM_CLOSE ignoré (utiliser ESC)\n");
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
            
            if (gfx && gfx->framebuffer) {
                // Récupérer la taille cliente de la fenêtre
                RECT rc;
                GetClientRect(hwnd, &rc);
                int client_w = rc.right - rc.left;
                int client_h = rc.bottom - rc.top;

                // Calculer une échelle entière qui respecte l'aspect ratio 160x144
                int scale_x = client_w / gfx->width;
                int scale_y = client_h / gfx->height;
                int scale = scale_x < scale_y ? scale_x : scale_y;
                if (scale <= 0) scale = 1; // au minimum 1:1

                int dst_w = gfx->width * scale;
                int dst_h = gfx->height * scale;

                // Centrer l'image dans la fenêtre
                int dst_x = (client_w - dst_w) / 2;
                int dst_y = (client_h - dst_h) / 2;

                // Afficher le framebuffer avec mise à l'échelle
                StretchDIBits(
                    hdc,
                    dst_x, dst_y, dst_w, dst_h,
                    0, 0, gfx->width, gfx->height,
                    gfx->framebuffer,
                    &gfx->bmi,
                    DIB_RGB_COLORS,
                    SRCCOPY
                );
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
    gfx->scale = 4;     // Facteur d'échelle par défaut (4x)
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
    
    // Créer la fenêtre en respectant l'aspect ratio de la Game Boy classique (4,7 x 4,3 cm)
    int win_w = 800;  // Largeur fixe de la fenêtre
    // Calculer la hauteur basée sur l'aspect ratio 4,7:4,3
    int win_h = (int)(win_w * 4.3 / 4.7);  // win_h ≈ 732 pixels
    
    // Calculer l'échelle pour remplir la fenêtre
    int scale_x = win_w / gfx->width;
    int scale_y = win_h / gfx->height;
    gfx->scale = (scale_x < scale_y) ? scale_x : scale_y;  // Prendre le plus petit pour garder les proportions
    
    printf("Fenêtre: %dx%d, échelle calculée: %d\n", win_w, win_h, gfx->scale);
    
    gfx->hwnd = CreateWindowEx(
        0,
        "CameBoy",
        "CameBoy - Game Boy LCD",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        win_w, win_h,
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
    
    // Copier les données du framebuffer PPU (u32 RGBA: RRGGBBAA) vers notre framebuffer RGB
    for (int y = 0; y < gfx->height; y++) {
        for (int x = 0; x < gfx->width; x++) {
            int src_idx = y * gfx->width + x;
            int dst_idx = (y * gfx->width + x) * 3;
            
            // Convertir de u32 RGBA (RRGGBBAA) vers RGB
            u32 pixel = ppu_framebuffer[src_idx];
            u8 r = (pixel >> 24) & 0xFF; // R
            u8 g = (pixel >> 16) & 0xFF; // G
            u8 b = (pixel >> 8)  & 0xFF; // B
            
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
#include "graphics_win32_gui.h"
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

// Constantes pour le layout
#define CONSOLE_WIDTH 320  // Largeur de la console Game Boy (basée sur la hauteur)
#define PANELS_WIDTH 300   // Largeur des panneaux de droite
#define BUTTON_HEIGHT 40
#define BUTTON_WIDTH 60
#define BUTTON_SPACING 10
#define LCD_SCALE 4

// Couleur verte Game Boy classique
#define GAMEBOY_GREEN_R 0x7A
#define GAMEBOY_GREEN_G 0xC2
#define GAMEBOY_GREEN_B 0x3C

// Fonction pour charger une image bitmap
static HBITMAP load_bitmap_from_file(const char* filename) {
    HBITMAP hBitmap = (HBITMAP)LoadImageA(
        NULL,
        filename,
        IMAGE_BITMAP,
        0, 0,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION
    );
    return hBitmap;
}

// Fonction pour obtenir les dimensions d'un bitmap
static void get_bitmap_size(HBITMAP hBitmap, int* width, int* height) {
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);
    *width = bm.bmWidth;
    *height = bm.bmHeight;
}

// Callback pour les événements de fenêtre
LRESULT CALLBACK WindowProcGUI(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    GraphicsWin32GUI* gfx = (GraphicsWin32GUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (uMsg) {
        case WM_CLOSE:
            if (gfx) gfx->running = false;
            PostQuitMessage(0);
            return 0;
            
        case WM_KEYDOWN:
        case WM_KEYUP: {
            if (!gfx || !gfx->joypad) break;
            
            bool pressed = (uMsg == WM_KEYDOWN);
            JoypadButton button = 0;
            
            switch (wParam) {
                case VK_UP: button = JOYPAD_UP; break;
                case VK_DOWN: button = JOYPAD_DOWN; break;
                case VK_LEFT: button = JOYPAD_LEFT; break;
                case VK_RIGHT: button = JOYPAD_RIGHT; break;
                case 'Z': button = JOYPAD_A; break;
                case 'X': button = JOYPAD_B; break;
                case VK_RETURN: button = JOYPAD_START; break;
                case VK_RSHIFT: button = JOYPAD_SELECT; break;
                case VK_ESCAPE:
                    gfx->running = false;
                    PostQuitMessage(0);
                    return 0;
            }
            
            if (button) {
                if (pressed) {
                    joypad_press(gfx->joypad, button);
                } else {
                    joypad_release(gfx->joypad, button);
                }
            }
            return 0;
        }
        
        case WM_COMMAND: {
            if (!gfx || !gfx->joypad) break;
            
            int id = LOWORD(wParam);
            bool pressed = (HIWORD(wParam) == BN_CLICKED);
            
            JoypadButton button = 0;
            switch (id) {
                case 1001: button = JOYPAD_A; break;
                case 1002: button = JOYPAD_B; break;
                case 1003: button = JOYPAD_START; break;
                case 1004: button = JOYPAD_SELECT; break;
                case 1005: button = JOYPAD_UP; break;
                case 1006: button = JOYPAD_DOWN; break;
                case 1007: button = JOYPAD_LEFT; break;
                case 1008: button = JOYPAD_RIGHT; break;
            }
            
            if (button && pressed) {
                joypad_press(gfx->joypad, button);
                // Simuler un relâchement après 100ms
                SetTimer(hwnd, button, 100, NULL);
            }
            return 0;
        }
        
        case WM_TIMER: {
            if (!gfx || !gfx->joypad) break;
            
            JoypadButton button = (JoypadButton)wParam;
            joypad_release(gfx->joypad, button);
            KillTimer(hwnd, wParam);
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            if (gfx) {
                // Dessiner l'image de fond
                if (gfx->hBackground) {
                    HDC hdcMem = CreateCompatibleDC(hdc);
                    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, gfx->hBackground);
                    
                    RECT rc;
                    GetClientRect(hwnd, &rc);
                    int client_h = rc.bottom - rc.top - BUTTON_HEIGHT;
                    
                    // Calculer les dimensions de l'image ajustées à la hauteur
                    int image_h = client_h;
                    int image_w = gfx->bg_width * image_h / gfx->bg_height;
                    
                    // Dessiner l'image ajustée à gauche
                    StretchBlt(hdc, 0, 0, image_w, image_h,
                              hdcMem, 0, 0, gfx->bg_width, gfx->bg_height, SRCCOPY);
                    
                    // Remplir le reste avec du vert Game Boy
                    if (image_w < rc.right - rc.left) {
                        RECT fill_rc = {image_w, 0, rc.right - rc.left, image_h};
                        HBRUSH hBrush = CreateSolidBrush(RGB(GAMEBOY_GREEN_R, GAMEBOY_GREEN_G, GAMEBOY_GREEN_B));
                        FillRect(hdc, &fill_rc, hBrush);
                        DeleteObject(hBrush);
                    }
                    
                    SelectObject(hdcMem, hOldBitmap);
                    DeleteDC(hdcMem);
                 } else {
                     // Dessiner un fond blanc simple quand pas d'image
                     RECT rc;
                     GetClientRect(hwnd, &rc);
                     
                     HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
                     FillRect(hdc, &rc, hBrush);
                     DeleteObject(hBrush);
                 }
                
                // Dessiner l'écran LCD
                if (gfx->framebuffer) {
                    // Position de l'écran LCD dans la fenêtre
                    int lcd_dst_x = gfx->lcd_x;
                    int lcd_dst_y = gfx->lcd_y;
                    int lcd_dst_w = gfx->lcd_w;
                    int lcd_dst_h = gfx->lcd_h;

                    SetStretchBltMode(hdc, STRETCH_HALFTONE);
                    // Rendu direct du framebuffer (BGR, 24 bits, top-down) via StretchDIBits
                    StretchDIBits(
                        hdc,
                        lcd_dst_x,
                        lcd_dst_y,
                        lcd_dst_w,
                        lcd_dst_h,
                        0,
                        0,
                        gfx->width,
                        gfx->height,
                        gfx->framebuffer,
                        &gfx->bmi,
                        DIB_RGB_COLORS,
                        SRCCOPY
                    );
                }
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_SIZE: {
            if (!gfx) break;
            
            RECT rc;
            GetClientRect(hwnd, &rc);
            int client_w = rc.right - rc.left;
            int client_h = rc.bottom - rc.top;
            
            // Calculer la largeur de l'image basée sur la hauteur (fixe la position X des panneaux)
            int image_h = client_h - BUTTON_HEIGHT;
            int image_w;
            if (gfx->hBackground) {
                image_w = gfx->bg_width * image_h / gfx->bg_height;
            } else {
                // Si pas d'image, utiliser un aspect ratio 4:3
                image_w = image_h * 4 / 3;
            }
            
             // Mettre à jour la position de l'écran LCD
             // y1 = 21% * hauteur de la fenêtre, y2 = 68% * hauteur de la fenêtre
             // x1 = 24% * largeur de l'image, x2 = 76% * largeur de l'image
             gfx->lcd_x = image_w * 0.24;
             gfx->lcd_y = image_h * 0.21;
             gfx->lcd_w = image_w * (0.76 - 0.24);  // 52% de la largeur de l'image
             gfx->lcd_h = image_h * (0.68 - 0.21);  // 47% de la hauteur de l'image
            
            // Repositionner le fond (controle STATIC) pour epouser la hauteur fenetre
            if (gfx->hBgStatic && gfx->hBackground) {
                int draw_w;
                if (gfx->bg_height > 0) {
                    draw_w = gfx->bg_width * image_h / gfx->bg_height;
                } else {
                    draw_w = image_w;
                }
                SetWindowPos(gfx->hBgStatic, NULL, 0, 0, draw_w, image_h, SWP_NOZORDER);
            }

            // Repositionner les panneaux de droite: X ancre a droite de l'image, largeur variable
            int panels_x = image_w;
            int panels_w = client_w - image_w;
            if (panels_w < 0) panels_w = 0;
            int panels_h = client_h - BUTTON_HEIGHT;
            
            if (gfx->hSerial) {
                // Port série en haut (2/3 de la hauteur)
                int serial_h = panels_h * 2 / 3;
                SetWindowPos(gfx->hSerial, NULL, panels_x, 0, panels_w, serial_h, SWP_NOZORDER);
            }
            
            if (gfx->hLogs) {
                // Logs en bas (1/3 de la hauteur)
                int logs_y = panels_h * 2 / 3;
                int logs_h = panels_h - logs_y;
                SetWindowPos(gfx->hLogs, NULL, panels_x, logs_y, panels_w, logs_h, SWP_NOZORDER);
            }
            
            // Repositionner les boutons sous l'image
            int button_y = client_h - BUTTON_HEIGHT;
            int button_start_x = (image_w - (8 * BUTTON_WIDTH + 7 * BUTTON_SPACING)) / 2;
            
            if (gfx->btnA) {
                SetWindowPos(gfx->btnA, NULL, button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
                button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
            }
            if (gfx->btnB) {
                SetWindowPos(gfx->btnB, NULL, button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
                button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
            }
            if (gfx->btnStart) {
                SetWindowPos(gfx->btnStart, NULL, button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
                button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
            }
            if (gfx->btnSelect) {
                SetWindowPos(gfx->btnSelect, NULL, button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
                button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
            }
            if (gfx->btnUp) {
                SetWindowPos(gfx->btnUp, NULL, button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
                button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
            }
            if (gfx->btnDown) {
                SetWindowPos(gfx->btnDown, NULL, button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
                button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
            }
            if (gfx->btnLeft) {
                SetWindowPos(gfx->btnLeft, NULL, button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
                button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
            }
            if (gfx->btnRight) {
                SetWindowPos(gfx->btnRight, NULL, button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
            }
            
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Initialisation de l'interface graphique Win32 GUI
bool graphics_win32_gui_init(GraphicsWin32GUI* gfx) {
    memset(gfx, 0, sizeof(GraphicsWin32GUI));
    
    gfx->width = 160;   // Largeur Game Boy
    gfx->height = 144;  // Hauteur Game Boy
    gfx->running = true;
    gfx->visible = false;
    
    // Charger l'image de fond Game Boy
    // Essayer d'abord BMP (plus fiable pour LoadImageA), puis JPG
    gfx->hBackground = load_bitmap_from_file("gameboy_bg.bmp");
    if (!gfx->hBackground) {
        gfx->hBackground = load_bitmap_from_file("gameboy_bg.jpg");
    }
    if (gfx->hBackground) {
        get_bitmap_size(gfx->hBackground, &gfx->bg_width, &gfx->bg_height);
        printf("Image de fond chargée: %dx%d\n", gfx->bg_width, gfx->bg_height);
        
         // Position de l'écran LCD dans l'image
         // y1 = 21% * hauteur, y2 = 68% * hauteur
         // x1 = 24% * largeur, x2 = 76% * largeur
         gfx->lcd_x = gfx->bg_width * 0.24;
         gfx->lcd_y = gfx->bg_height * 0.21;
         gfx->lcd_w = gfx->bg_width * (0.76 - 0.24);  // 52% de la largeur
         gfx->lcd_h = gfx->bg_height * (0.68 - 0.21); // 47% de la hauteur
     } else {
         printf("Impossible de charger l'image de fond, utilisation d'un fond vert Game Boy\n");
         gfx->bg_width = 800;
         gfx->bg_height = 600;
         // Position de l'écran LCD avec les mêmes pourcentages
         gfx->lcd_x = gfx->bg_width * 0.24;
         gfx->lcd_y = gfx->bg_height * 0.21;
         gfx->lcd_w = gfx->bg_width * (0.76 - 0.24);
         gfx->lcd_h = gfx->bg_height * (0.68 - 0.21);
     }
    
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
    wc.lpfnWndProc = WindowProcGUI;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "CameBoyGUI";
    
    if (!RegisterClassEx(&wc)) {
        printf("Erreur: Impossible d'enregistrer la classe de fenêtre GUI\n");
        free(gfx->framebuffer);
        return false;
    }
    
    // Créer la fenêtre
    int win_h = 911;  // Hauteur de l'image de fond demandée
    int win_w = 800 + PANELS_WIDTH;  // Largeur fixe
    
    gfx->hwnd = CreateWindowEx(
        0,
        "CameBoyGUI",
        "CameBoy - Game Boy Emulator",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        win_w, win_h,
        NULL, NULL,
        GetModuleHandle(NULL),
        NULL
    );
    
    if (!gfx->hwnd) {
        printf("Erreur: Impossible de créer la fenêtre GUI\n");
        free(gfx->framebuffer);
        return false;
    }
    
    // Stocker le pointeur vers GraphicsWin32GUI
    SetWindowLongPtr(gfx->hwnd, GWLP_USERDATA, (LONG_PTR)gfx);
    
    // Obtenir le HDC
    gfx->hdc = GetDC(gfx->hwnd);

    // Creer un controle STATIC pour l'image de fond afin qu'elle reste affichee
    if (gfx->hBackground) {
        HBITMAP hBmp = gfx->hBackground;
        gfx->hBgStatic = CreateWindowEx(
            0,
            "STATIC",
            NULL,
            WS_CHILD | WS_VISIBLE | SS_BITMAP,
            0, 0,
            gfx->bg_width,
            gfx->bg_height,
            gfx->hwnd,
            (HMENU)3000,
            GetModuleHandle(NULL),
            NULL
        );
        SendMessage(gfx->hBgStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
    }

    // Créer le bitmap
    gfx->hbitmap = CreateCompatibleBitmap(gfx->hdc, gfx->width, gfx->height);
    
    // Créer la police pour le panneau série
    gfx->hFont = CreateFont(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas"
    );
    
    // Créer le panneau série (en haut à droite)
    gfx->hSerial = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "Port Serie:",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        0, 0, PANELS_WIDTH, (win_h - BUTTON_HEIGHT) * 2 / 3,
        gfx->hwnd,
        (HMENU)2000,
        GetModuleHandle(NULL),
        NULL
    );
    
    if (gfx->hSerial) {
        SendMessage(gfx->hSerial, WM_SETFONT, (WPARAM)gfx->hFont, TRUE);
    }
    
    // Créer le panneau logs (en bas à droite)
    gfx->hLogs = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "Logs:",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        0, (win_h - BUTTON_HEIGHT) * 2 / 3, PANELS_WIDTH, (win_h - BUTTON_HEIGHT) / 3,
        gfx->hwnd,
        (HMENU)2001,
        GetModuleHandle(NULL),
        NULL
    );
    
    if (gfx->hLogs) {
        SendMessage(gfx->hLogs, WM_SETFONT, (WPARAM)gfx->hFont, TRUE);
    }
    
    // Créer les boutons Game Boy
    int button_y = win_h - BUTTON_HEIGHT;
    int image_w = win_w - PANELS_WIDTH;  // Largeur fixe de l'image
    int button_start_x = (image_w - (8 * BUTTON_WIDTH + 7 * BUTTON_SPACING)) / 2;
    
    gfx->btnA = CreateWindowEx(0, "BUTTON", "A", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                               button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT,
                               gfx->hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);
    button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
    
    gfx->btnB = CreateWindowEx(0, "BUTTON", "B", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                               button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT,
                               gfx->hwnd, (HMENU)1002, GetModuleHandle(NULL), NULL);
    button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
    
    gfx->btnStart = CreateWindowEx(0, "BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                   gfx->hwnd, (HMENU)1003, GetModuleHandle(NULL), NULL);
    button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
    
    gfx->btnSelect = CreateWindowEx(0, "BUTTON", "Select", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                    gfx->hwnd, (HMENU)1004, GetModuleHandle(NULL), NULL);
    button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
    
    gfx->btnUp = CreateWindowEx(0, "BUTTON", "Up", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                gfx->hwnd, (HMENU)1005, GetModuleHandle(NULL), NULL);
    button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
    
    gfx->btnDown = CreateWindowEx(0, "BUTTON", "Down", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                  button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                  gfx->hwnd, (HMENU)1006, GetModuleHandle(NULL), NULL);
    button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
    
    gfx->btnLeft = CreateWindowEx(0, "BUTTON", "Left", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                  button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                  gfx->hwnd, (HMENU)1007, GetModuleHandle(NULL), NULL);
    button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
    
    gfx->btnRight = CreateWindowEx(0, "BUTTON", "Right", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                   gfx->hwnd, (HMENU)1008, GetModuleHandle(NULL), NULL);
    
    // Initialiser les buffers
    gfx->serial_buffer[0] = '\0';
    gfx->serial_length = 0;
    gfx->logs_buffer[0] = '\0';
    gfx->logs_length = 0;
    
    // Afficher la fenêtre
    ShowWindow(gfx->hwnd, SW_SHOW);
    UpdateWindow(gfx->hwnd);
    
    return true;
}

// Nettoyage de l'interface graphique GUI
void graphics_win32_gui_cleanup(GraphicsWin32GUI* gfx) {
    if (gfx->hFont) {
        DeleteObject(gfx->hFont);
    }
    
    if (gfx->hBackground) {
        DeleteObject(gfx->hBackground);
    }
    
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
void graphics_win32_gui_update(GraphicsWin32GUI* gfx, u32* ppu_framebuffer) {
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
            
            // Remplacement uniquement pour les pixels noirs -> vert Game Boy
            if (r == 0 && g == 0 && b == 0) {
                gfx->framebuffer[dst_idx + 0] = GAMEBOY_GREEN_B;
                gfx->framebuffer[dst_idx + 1] = GAMEBOY_GREEN_G;
                gfx->framebuffer[dst_idx + 2] = GAMEBOY_GREEN_R;
            } else {
                gfx->framebuffer[dst_idx + 0] = b; // B
                gfx->framebuffer[dst_idx + 1] = g; // G
                gfx->framebuffer[dst_idx + 2] = r; // R
            }
        }
    }
}

// Afficher le framebuffer
void graphics_win32_gui_present(GraphicsWin32GUI* gfx) {
    if (!gfx || !gfx->hwnd) return;
    
    // Forcer le redessin de la fenêtre
    InvalidateRect(gfx->hwnd, NULL, FALSE);
    UpdateWindow(gfx->hwnd);
}

// Gérer les événements
void graphics_win32_gui_handle_events(GraphicsWin32GUI* gfx, bool* running) {
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
void graphics_win32_gui_show(GraphicsWin32GUI* gfx) {
    if (!gfx || !gfx->hwnd) return;
    
    gfx->visible = true;
    ShowWindow(gfx->hwnd, SW_SHOW);
    UpdateWindow(gfx->hwnd);
}

// Cacher la fenêtre
void graphics_win32_gui_hide(GraphicsWin32GUI* gfx) {
    if (!gfx || !gfx->hwnd) return;
    
    gfx->visible = false;
    ShowWindow(gfx->hwnd, SW_HIDE);
}

// Lier le joypad
void graphics_win32_gui_bind_joypad(GraphicsWin32GUI* gfx, Joypad* joypad) {
    gfx->joypad = joypad;
}

// Ajouter du texte au panneau série
void graphics_win32_gui_append_serial(GraphicsWin32GUI* gfx, const char* text, int length) {
    if (!gfx || !gfx->hSerial || !text) return;
    
    // Ajouter au buffer interne
    int remaining = sizeof(gfx->serial_buffer) - gfx->serial_length - 1;
    if (remaining > 0) {
        int copy_len = (length < remaining) ? length : remaining;
        memcpy(gfx->serial_buffer + gfx->serial_length, text, copy_len);
        gfx->serial_length += copy_len;
        gfx->serial_buffer[gfx->serial_length] = '\0';
    }
    
    // Mettre à jour l'affichage
    SetWindowTextA(gfx->hSerial, gfx->serial_buffer);
    
    // Faire défiler vers le bas
    SendMessage(gfx->hSerial, EM_SETSEL, gfx->serial_length, gfx->serial_length);
    SendMessage(gfx->hSerial, EM_SCROLL, SB_BOTTOM, 0);
}

// Ajouter du texte au panneau logs
void graphics_win32_gui_append_logs(GraphicsWin32GUI* gfx, const char* text, int length) {
    if (!gfx || !gfx->hLogs || !text) return;
    
    // Ajouter au buffer interne
    int remaining = sizeof(gfx->logs_buffer) - gfx->logs_length - 1;
    if (remaining > 0) {
        int copy_len = (length < remaining) ? length : remaining;
        memcpy(gfx->logs_buffer + gfx->logs_length, text, copy_len);
        gfx->logs_length += copy_len;
        gfx->logs_buffer[gfx->logs_length] = '\0';
    }
    
    // Mettre à jour l'affichage
    SetWindowTextA(gfx->hLogs, gfx->logs_buffer);
    
    // Faire défiler vers le bas
    SendMessage(gfx->hLogs, EM_SETSEL, gfx->logs_length, gfx->logs_length);
    SendMessage(gfx->hLogs, EM_SCROLL, SB_BOTTOM, 0);
}

// Définir le callback série
void graphics_win32_gui_set_serial_callback(GraphicsWin32GUI* gfx, mmu_serial_cb_t callback) {
    gfx->serial_callback = callback;
}

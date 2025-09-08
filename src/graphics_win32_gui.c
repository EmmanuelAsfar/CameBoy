#include "graphics_win32_gui.h"
#include "resource_manager.h"
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

// Constantes pour le layout
#define GUI_BG_ONLY 0  // Mode complet avec panneaux
#define BUTTON_HEIGHT 40
#define BUTTON_WIDTH 60
#define BUTTON_SPACING 10
#define ACTION_BAR_HEIGHT 50  // Hauteur fixe du bandeau d'actions
#define PANEL_MIN_WIDTH 300   // Largeur minimale du panneau droit

// Offsets LCD dans la console (pourcentages spécifiés par l'utilisateur)
#define LCD_OFFSET_X1_PERCENT (168.0f / 694.0f)
#define LCD_OFFSET_X2_PERCENT (530.0f / 694.0f)
#define LCD_OFFSET_Y1_PERCENT (154.0f / 1151.0f)
#define LCD_OFFSET_Y2_PERCENT (482.0f / 1151.0f)

// Couleur verte Game Boy classique
#define GAMEBOY_GREEN_R 0x7A
#define GAMEBOY_GREEN_G 0xC2
#define GAMEBOY_GREEN_B 0x3C

// Structure pour le layout
typedef struct {
    int window_w, window_h;
    int console_w, console_h;
    int lcd_x, lcd_y, lcd_w, lcd_h;
    int panels_x, panels_w, panels_h;
    int action_bar_y, action_bar_h;
    int serial_y, serial_h;
    int logs_y, logs_h;
    int splitter_y;
} Layout;

// Calculer le layout complet
static void calculate_layout(GraphicsWin32GUI* gfx, Layout* layout) {
    RECT rc;
    GetClientRect(gfx->hwnd, &rc);
    layout->window_w = rc.right - rc.left;
    layout->window_h = rc.bottom - rc.top;
    
    // Zone console (gauche) - hauteur complète
    layout->console_h = layout->window_h;
    
    if (gfx->hBackground && gfx->bg_height > 0) {
        // Avec image de fond : ajuster selon aspect ratio
        layout->console_w = gfx->bg_width * layout->console_h / gfx->bg_height;
    } else {
        // Sans image : aspect ratio 4:3
        layout->console_w = layout->console_h * 4 / 3;
    }
    
    // LCD : positions relatives dans la zone console (pourcentages spécifiés)
    layout->lcd_x = (int)(layout->console_w * LCD_OFFSET_X1_PERCENT);
    layout->lcd_y = (int)(layout->console_h * LCD_OFFSET_Y1_PERCENT);
    layout->lcd_w = (int)(layout->console_w * (LCD_OFFSET_X2_PERCENT - LCD_OFFSET_X1_PERCENT));
    layout->lcd_h = (int)(layout->console_h * (LCD_OFFSET_Y2_PERCENT - LCD_OFFSET_Y1_PERCENT));
    
    // Assurer une taille minimale pour éviter les problèmes d'affichage
    if (layout->lcd_w < 2) layout->lcd_w = 2;
    if (layout->lcd_h < 2) layout->lcd_h = 2;
    
    // Panneau droit
    layout->panels_x = layout->console_w;
    layout->panels_w = layout->window_w - layout->console_w;
    if (layout->panels_w < PANEL_MIN_WIDTH) layout->panels_w = PANEL_MIN_WIDTH;
    layout->panels_h = layout->window_h;
    
    // Bandeau d'actions (fixe en haut du panneau)
    layout->action_bar_y = 0;
    layout->action_bar_h = ACTION_BAR_HEIGHT;
    
    // Zone restante pour les panneaux scrollables
    int remaining_h = layout->panels_h - layout->action_bar_h;
    
    // Splitter au milieu de la zone restante
    layout->splitter_y = layout->action_bar_h + remaining_h / 2;
    
    // Panneau série (en haut)
    layout->serial_y = layout->action_bar_h;
    layout->serial_h = layout->splitter_y - layout->action_bar_h;
    
    // Panneau logs (en bas)
    layout->logs_y = layout->splitter_y;
    layout->logs_h = remaining_h - layout->serial_h;
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
                    // Log only pressed events to stdout (will be captured by GUI logs panel)
                    const char* name = "UNKNOWN";
                    switch (wParam) {
                        case VK_UP: name = "UP"; break;
                        case VK_DOWN: name = "DOWN"; break;
                        case VK_LEFT: name = "LEFT"; break;
                        case VK_RIGHT: name = "RIGHT"; break;
                        case 'Z': name = "A"; break;
                        case 'X': name = "B"; break;
                        case VK_RETURN: name = "START"; break;
                        case VK_RSHIFT: name = "SELECT"; break;
                        default: break;
                    }
                    printf("[JOY] KEY %s PRESS\n", name);
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
                SetTimer(hwnd, button, 100, NULL);
                // Log press event via stdout
                const char* name = "UNKNOWN";
                switch (id) {
                    case 1001: name = "A"; break;
                    case 1002: name = "B"; break;
                    case 1003: name = "START"; break;
                    case 1004: name = "SELECT"; break;
                    case 1005: name = "UP"; break;
                    case 1006: name = "DOWN"; break;
                    case 1007: name = "LEFT"; break;
                    case 1008: name = "RIGHT"; break;
                    default: break;
                }
                printf("[JOY] GUI %s PRESS\n", name);
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
                Layout layout;
                calculate_layout(gfx, &layout);
                
                // 1. Dessiner le fond de la zone console
                if (gfx->hBackground) {
                    // Dessiner l'image de fond redimensionnée
                    HDC hdcMem = CreateCompatibleDC(hdc);
                    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, gfx->hBackground);
                    
                    StretchBlt(hdc, 0, 0, layout.console_w, layout.console_h,
                              hdcMem, 0, 0, gfx->bg_width, gfx->bg_height, SRCCOPY);
                    
                    SelectObject(hdcMem, hOldBitmap);
                    DeleteDC(hdcMem);
                } else {
                    // Fond blanc pour la zone console
                    RECT console_rc = {0, 0, layout.console_w, layout.console_h};
                    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
                    FillRect(hdc, &console_rc, hBrush);
                    DeleteObject(hBrush);
                }
                
                // 2. Dessiner le fond des panneaux (gris clair)
                if (layout.panels_w > 0) {
                    RECT panels_rc = {layout.panels_x, 0, layout.panels_x + layout.panels_w, layout.panels_h};
                    HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
                    FillRect(hdc, &panels_rc, hBrush);
                    DeleteObject(hBrush);
                    
                    // Dessiner le splitter vertical
                    RECT splitter_rc = {layout.panels_x, layout.splitter_y - 2, 
                                       layout.panels_x + layout.panels_w, layout.splitter_y + 2};
                    HBRUSH hSplitterBrush = CreateSolidBrush(RGB(180, 180, 180));
                    FillRect(hdc, &splitter_rc, hSplitterBrush);
                    DeleteObject(hSplitterBrush);
                }
                
                // 3. Dessiner l'écran LCD (toujours par-dessus le fond)
                if (gfx->framebuffer) {
                    // Rendu net (nearest-neighbor) pour conserver les pixels GB
                    SetStretchBltMode(hdc, COLORONCOLOR);
                    
                    // Rendu direct du framebuffer pour remplir exactement le rectangle LCD (sans préserver l'aspect ratio)
                    StretchDIBits(
                        hdc,
                        layout.lcd_x,
                        layout.lcd_y,
                        layout.lcd_w,
                        layout.lcd_h,
                        0,
                        0,
                        gfx->width,
                        gfx->height,
                        gfx->framebuffer,
                        &gfx->bmi,
                        DIB_RGB_COLORS,
                        SRCCOPY
                    );
                    
                    // Cadre retiré (ancien debug)
                }
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_SIZE: {
            if (!gfx) break;
            
            Layout layout;
            calculate_layout(gfx, &layout);
            
            // Repositionner les panneaux de droite
            if (gfx->hSerial) {
                SetWindowPos(gfx->hSerial, HWND_TOP, layout.panels_x, layout.serial_y, 
                           layout.panels_w, layout.serial_h, SWP_SHOWWINDOW);
            }
            
            if (gfx->hLogs) {
                SetWindowPos(gfx->hLogs, HWND_TOP, layout.panels_x, layout.logs_y, 
                           layout.panels_w, layout.logs_h, SWP_SHOWWINDOW);
            }
            
            // Repositionner les boutons dans le bandeau d'actions
            int button_start_x = layout.panels_x + 10;
            int button_y = layout.action_bar_y + (layout.action_bar_h - BUTTON_HEIGHT) / 2;
            HWND buttons[] = {gfx->btnA, gfx->btnB, gfx->btnStart, gfx->btnSelect,
                             gfx->btnUp, gfx->btnDown, gfx->btnLeft, gfx->btnRight};
            for (int i = 0; i < 8; i++) {
                if (buttons[i]) {
                    SetWindowPos(buttons[i], HWND_TOP, button_start_x, button_y,
                               BUTTON_WIDTH, BUTTON_HEIGHT, SWP_SHOWWINDOW);
                    button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
                }
            }
            
            // Forcer le redessin
            InvalidateRect(hwnd, NULL, TRUE);
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
    
    // Initialiser le gestionnaire de ressources et charger le BMP
    if (!resource_manager_init()) {
        printf("ERREUR: Ressources introuvables\n");
        gfx->bg_width = 800;
        gfx->bg_height = 600;
    } else {
        gfx->hBackground = resource_load_image("gameboy_bg.bmp");
        if (gfx->hBackground) {
            BITMAP bm;
            GetObject(gfx->hBackground, sizeof(BITMAP), &bm);
            gfx->bg_width = bm.bmWidth;
            gfx->bg_height = bm.bmHeight;
            printf("Image de fond BMP chargee: %dx%d\n", gfx->bg_width, gfx->bg_height);
        } else {
            printf("Impossible de charger l'image BMP, utilisation d'un fond blanc\n");
            gfx->bg_width = 800;
            gfx->bg_height = 600;
        }
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
        printf("Erreur: Impossible d'enregistrer la classe de fenetre GUI\n");
        free(gfx->framebuffer);
        return false;
    }
    
    // Créer la fenêtre - hauteur fixe 800px, largeur basée sur l'image
    int win_h = 800;
    int win_w = 1200;
    
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
        printf("Erreur: Impossible de creer la fenetre GUI\n");
        free(gfx->framebuffer);
        return false;
    }
    
    // Stocker le pointeur vers GraphicsWin32GUI
    SetWindowLongPtr(gfx->hwnd, GWLP_USERDATA, (LONG_PTR)gfx);
    
    // Obtenir le HDC
    gfx->hdc = GetDC(gfx->hwnd);
    
    // Créer la police pour les panneaux
    gfx->hFont = CreateFont(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas"
    );
    
    // Créer les contrôles APRÈS la fenêtre principale
    Layout layout;
    calculate_layout(gfx, &layout);
    
    // Panneau série (en haut à droite)
    gfx->hSerial = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        layout.panels_x, layout.serial_y, layout.panels_w, layout.serial_h,
        gfx->hwnd,
        (HMENU)(INT_PTR)2000,
        GetModuleHandle(NULL),
        NULL
    );
    if (gfx->hSerial) {
        SendMessage(gfx->hSerial, WM_SETFONT, (WPARAM)gfx->hFont, TRUE);
        SetWindowTextA(gfx->hSerial, "Port Serie:\r\n");
    }
    
    // Panneau logs (en bas à droite)
    gfx->hLogs = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        layout.panels_x, layout.logs_y, layout.panels_w, layout.logs_h,
        gfx->hwnd,
        (HMENU)(INT_PTR)2001,
        GetModuleHandle(NULL),
        NULL
    );
    if (gfx->hLogs) {
        SendMessage(gfx->hLogs, WM_SETFONT, (WPARAM)gfx->hFont, TRUE);
        SetWindowTextA(gfx->hLogs, "Logs:\r\n");
    }
    
    // Créer les boutons Game Boy dans le bandeau d'actions
    int button_start_x = layout.panels_x + 10;
    int button_y = layout.action_bar_y + (layout.action_bar_h - BUTTON_HEIGHT) / 2;
    const char* button_labels[] = {"A", "B", "Start", "Select", "Up", "Down", "Left", "Right"};
    HWND* button_handles[] = {&gfx->btnA, &gfx->btnB, &gfx->btnStart, &gfx->btnSelect,
                             &gfx->btnUp, &gfx->btnDown, &gfx->btnLeft, &gfx->btnRight};
    for (int i = 0; i < 8; i++) {
        *button_handles[i] = CreateWindowEx(0, "BUTTON", button_labels[i],
                                          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                          button_start_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                          gfx->hwnd, (HMENU)(INT_PTR)(1001 + i), GetModuleHandle(NULL), NULL);
        button_start_x += BUTTON_WIDTH + BUTTON_SPACING;
    }
    
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

    // Palette DMG teintée "Game Boy" (vert clair -> gris foncé)
    // Niveaux du plus clair (index 0) au plus foncé (index 3)
    static const u8 DMG_TINT[4][3] = {
        { 0xE0, 0xF8, 0xD0 }, // #E0F8D0 (clair)
        { 0xA8, 0xD0, 0x80 }, // #A8D080 (moyen clair)
        { 0x56, 0x8F, 0x5C }, // #568F5C (olive foncé)
        { 0x2E, 0x2E, 0x2E }, // #2E2E2E (gris foncé, cristaux)
    };

    // Copier + teinter depuis le framebuffer PPU (u32 RGBA: RRGGBBAA) vers RGB 24bpp (BGR en mémoire)
    for (int y = 0; y < gfx->height; y++) {
        for (int x = 0; x < gfx->width; x++) {
            int src_idx = y * gfx->width + x;
            int dst_idx = (y * gfx->width + x) * 3;

            u32 pixel = ppu_framebuffer[src_idx];
            u8 r = (u8)((pixel >> 24) & 0xFF);
            // Les 4 niveaux DMG que nous émettons sont en niveaux de gris (R=G=B). Déterminer l'index.
            // Seuils robustes pour {0xFF, 0xAA, 0x55, 0x00}
            int level = (r > 0xCC) ? 0 : (r > 0x88) ? 1 : (r > 0x33) ? 2 : 3;
            // Inverser la perception (texte foncé sur fond clair):
            // sur DMG, l'index 0 est le plus clair (fond), 3 le plus sombre (encre).
            // Si le framebuffer encode 0=blanc, 3=noir, on veut conserver cela et
            // teinter en conséquence. Si l'écran te paraît inversé, c'était le cas
            // contraire: appliquer shade = 3 - level corrige visuellement.
            int shade = 3 - level;

            u8 tr = DMG_TINT[shade][0];
            u8 tg = DMG_TINT[shade][1];
            u8 tb = DMG_TINT[shade][2];

            gfx->framebuffer[dst_idx + 0] = tb; // B
            gfx->framebuffer[dst_idx + 1] = tg; // G
            gfx->framebuffer[dst_idx + 2] = tr; // R
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

    // Si la fenêtre a été détruite, arrêter la boucle
    if (!IsWindow(gfx->hwnd)) {
        *running = false;
        gfx->running = false;
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
    
    // Ajouter au buffer interne avec normalisation des fins de ligne (\n -> \r\n)
    int remaining = (int)sizeof(gfx->serial_buffer) - gfx->serial_length - 1;
    for (int i = 0; i < length && remaining > 0; i++) {
        char c = text[i];
        if (c == '\n') {
            // Insérer \r si précédent n'est pas déjà \r
            if (gfx->serial_length == 0 || gfx->serial_buffer[gfx->serial_length - 1] != '\r') {
                if (remaining <= 0) break;
                gfx->serial_buffer[gfx->serial_length++] = '\r';
                remaining--;
            }
        }
        if (remaining <= 0) break;
        gfx->serial_buffer[gfx->serial_length++] = c;
        remaining--;
    }
    gfx->serial_buffer[gfx->serial_length] = '\0';
    
    // Mettre à jour l'affichage
    SetWindowTextA(gfx->hSerial, gfx->serial_buffer);
    
    // Faire défiler vers le bas
    SendMessage(gfx->hSerial, EM_SETSEL, gfx->serial_length, gfx->serial_length);
    SendMessage(gfx->hSerial, EM_SCROLL, SB_BOTTOM, 0);
}

// Ajouter du texte au panneau logs
void graphics_win32_gui_append_logs(GraphicsWin32GUI* gfx, const char* text, int length) {
    if (!gfx || !gfx->hLogs || !text) return;
    
    // Ajouter au buffer interne avec normalisation des fins de ligne (\n -> \r\n)
    int remaining = (int)sizeof(gfx->logs_buffer) - gfx->logs_length - 1;
    for (int i = 0; i < length && remaining > 0; i++) {
        char c = text[i];
        if (c == '\n') {
            if (gfx->logs_length == 0 || gfx->logs_buffer[gfx->logs_length - 1] != '\r') {
                if (remaining <= 0) break;
                gfx->logs_buffer[gfx->logs_length++] = '\r';
                remaining--;
            }
        }
        if (remaining <= 0) break;
        gfx->logs_buffer[gfx->logs_length++] = c;
        remaining--;
    }
    gfx->logs_buffer[gfx->logs_length] = '\0';
    
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

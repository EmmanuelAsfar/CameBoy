#include "common.h"
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"
#include "timer.h"
#include "joypad.h"
#include "interrupt.h"
#include "graphics_win32_gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

// Structure de l'émulateur
typedef struct {
    CPU cpu;
    MMU mmu;
    PPU ppu;
    Timer timer;
    Joypad joypad;
    InterruptManager interrupt;
    GraphicsWin32GUI graphics;
    bool running;
    HANDLE stdoutRead;
    HANDLE stdoutWrite;
    HANDLE stdoutThread;
} Emulator;

// Pointeur global vers l'émulateur pour le callback série
static Emulator* g_emulator = NULL;

// Callback pour la sortie série
static void gui_serial_callback(u8 ch) {
    if (g_emulator) {
        char text[2] = {ch, '\0'};
        graphics_win32_gui_append_serial(&g_emulator->graphics, text, 1);
    }
}

// Fonction pour ajouter des logs
static void gui_log(const char* format, ...) {
    if (!g_emulator) return;
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (len > 0) {
        graphics_win32_gui_append_logs(&g_emulator->graphics, buffer, len);
    }
}

// Thread de lecture stdout/stderr redirigés vers le panneau logs
static DWORD WINAPI stdout_reader_thread(LPVOID param) {
    (void)param;
    if (!g_emulator || !g_emulator->stdoutRead) return 0;
    char buffer[512];
    DWORD readBytes = 0;
    for (;;) {
        BOOL ok = ReadFile(g_emulator->stdoutRead, buffer, (DWORD)(sizeof(buffer)), &readBytes, NULL);
        if (!ok || readBytes == 0) {
            Sleep(10);
            if (!g_emulator || !g_emulator->running) break;
            continue;
        }
        graphics_win32_gui_append_logs(&g_emulator->graphics, buffer, (int)readBytes);
    }
    return 0;
}

// Fonction pour initialiser l'émulateur
bool emulator_init(Emulator* emu) {
    memset(emu, 0, sizeof(Emulator));
    
    // Définir le pointeur global pour le callback
    g_emulator = emu;
    
    // Initialiser les composants
    cpu_init(&emu->cpu);
    mmu_init(&emu->mmu);
    ppu_init(&emu->ppu);
    timer_init(&emu->timer);
    joypad_init(&emu->joypad);
    interrupt_init(&emu->interrupt);
    
    // Les composants sont indépendants, pas besoin de les lier
    
    // Initialiser l'interface graphique GUI
    if (!graphics_win32_gui_init(&emu->graphics)) {
        printf("Erreur: Impossible d'initialiser l'interface graphique GUI\n");
        return false;
    }
    
    // Lier le joypad à l'interface graphique
    graphics_win32_gui_bind_joypad(&emu->graphics, &emu->joypad);
    // Lier le joypad à la MMU pour P1
    mmu_set_joypad(&emu->mmu, &emu->joypad);
    
    // Définir le callback série
    mmu_set_serial_callback(&emu->mmu, gui_serial_callback);
    
    // Rediriger stdout/stderr vers le panneau logs via un pipe
    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    if (CreatePipe(&emu->stdoutRead, &emu->stdoutWrite, &sa, 0)) {
        SetHandleInformation(emu->stdoutRead, HANDLE_FLAG_INHERIT, 0);
        int fd = _open_osfhandle((intptr_t)emu->stdoutWrite, _O_TEXT);
        if (fd != -1) {
            _dup2(fd, _fileno(stdout));
            _dup2(fd, _fileno(stderr));
            setvbuf(stdout, NULL, _IONBF, 0);
            setvbuf(stderr, NULL, _IONBF, 0);
            // Lancer le thread lecteur
            emu->stdoutThread = CreateThread(NULL, 0, stdout_reader_thread, NULL, 0, NULL);
        }
    }
    
    emu->running = true;
    return true;
}

// Fonction pour nettoyer l'émulateur
void emulator_cleanup(Emulator* emu) {
    graphics_win32_gui_cleanup(&emu->graphics);
    mmu_cleanup(&emu->mmu);
    // Nettoyer redirection stdout/stderr
    if (emu->stdoutThread) {
        WaitForSingleObject(emu->stdoutThread, 100);
        CloseHandle(emu->stdoutThread);
    }
    if (emu->stdoutRead) CloseHandle(emu->stdoutRead);
    // emu->stdoutWrite est possiblement possédé par la CRT après _open_osfhandle
}

// Fonction pour charger une ROM
bool emulator_load_rom(Emulator* emu, const char* filename) {
    return mmu_load_rom(&emu->mmu, filename);
}

// Fonction pour exécuter l'émulateur
void emulator_run(Emulator* emu) {
    if (!emu->running) return;
    
    // Afficher la fenêtre
    graphics_win32_gui_show(&emu->graphics);
    
    // Phase de rechauffement CPU pour initialiser la ROM
    gui_log("Phase de rechauffement CPU...\n");
    for (int i = 0; i < 100000; i++) {
        cpu_step(&emu->cpu, &emu->mmu);
    }
    
    // Forcer un rendu initial
    gui_log("Rendu initial...\n");
    for (int line = 0; line < 144; line++) {
        ppu_render_line(&emu->ppu, emu->mmu.vram);
    }
    graphics_win32_gui_update(&emu->graphics, (u32*)emu->ppu.framebuffer);
    graphics_win32_gui_present(&emu->graphics);
    
    gui_log("Demarrage de la boucle principale...\n");
    
    // Boucle principale d'émulation
    while (emu->running && emu->graphics.running) {
        // Exécuter une instruction CPU
        u8 cycles = cpu_step(&emu->cpu, &emu->mmu);
        
        // Mettre à jour le timer
        timer_tick(&emu->timer, cycles);
        
        // Mettre à jour le PPU
        ppu_tick(&emu->ppu, cycles, emu->mmu.vram);
        
        // Vérifier si on doit rendre une frame complète (comme la version run)
        static u32 frame_cycles = 0;
        frame_cycles += cycles;
        if (frame_cycles >= (GB_FREQ / 60)) {  // 60 FPS
            frame_cycles = 0;
            
            // Rendre toutes les lignes de la frame (comme la version run)
            for (int y = 0; y < GB_HEIGHT; y++) {
                emu->ppu.ly = (u8)y;
                ppu_render_line(&emu->ppu, emu->mmu.vram);
            }
            graphics_win32_gui_update(&emu->graphics, (u32*)emu->ppu.framebuffer);
            graphics_win32_gui_present(&emu->graphics);
        }
        
        // Gérer les événements
        graphics_win32_gui_handle_events(&emu->graphics, &emu->running);
        
        // Petite pause pour éviter de surcharger le système
        Sleep(1);
    }
    
    printf("Arret de l'emulateur\n");
}

// Fonction principale
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <rom_file>\n", argv[0]);
        printf("Controles clavier:\n");
        printf("  Fleches: D-pad\n");
        printf("  Z: Bouton A\n");
        printf("  X: Bouton B\n");
        printf("  Entree: Start\n");
        printf("  Maj droit: Select\n");
        printf("  Echap: Quitter\n");
        return 1;
    }
    
    Emulator emu;
    
    // Initialiser l'émulateur
    if (!emulator_init(&emu)) {
        printf("Erreur: Impossible d'initialiser l'emulateur\n");
        return 1;
    }
    
    // Charger la ROM
    if (!emulator_load_rom(&emu, argv[1])) {
        printf("Erreur: Impossible de charger la ROM %s\n", argv[1]);
        emulator_cleanup(&emu);
        return 1;
    }
    
    printf("ROM chargee: %s\n", argv[1]);
    printf("Type de cartouche: %s\n", cart_type_name(emu.mmu.cart.type));
    
    // Exécuter l'émulateur
    emulator_run(&emu);
    
    // Nettoyer
    emulator_cleanup(&emu);
    
    return 0;
}
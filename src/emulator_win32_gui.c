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
#include <time.h>

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
    char rom_name[128];
    char rom_path[260];
    char log_dir[260];
    FILE* f_log;
    FILE* f_serial;
    FILE* f_stdout;
} Emulator;

// Pointeur global vers l'émulateur pour le callback série
static Emulator* g_emulator = NULL;

// Callback pour la sortie série
static void gui_serial_callback(u8 ch) {
    if (g_emulator) {
        char text[2] = {ch, '\0'};
        graphics_win32_gui_append_serial(&g_emulator->graphics, text, 1);
        if (g_emulator->f_serial) {
            fprintf(g_emulator->f_serial, "%c", (char)ch);
            fflush(g_emulator->f_serial);
        }
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
        if (g_emulator->f_log) {
            SYSTEMTIME st; GetLocalTime(&st);
            fprintf(g_emulator->f_log, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] %.*s",
                (int)st.wYear, (int)st.wMonth, (int)st.wDay,
                (int)st.wHour, (int)st.wMinute, (int)st.wSecond, (int)st.wMilliseconds,
                len, buffer);
            fflush(g_emulator->f_log);
        }
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
        if (g_emulator->f_stdout) {
            SYSTEMTIME st; GetLocalTime(&st);
            fprintf(g_emulator->f_stdout, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
                (int)st.wYear, (int)st.wMonth, (int)st.wDay,
                (int)st.wHour, (int)st.wMinute, (int)st.wSecond, (int)st.wMilliseconds);
            fwrite(buffer, 1, readBytes, g_emulator->f_stdout);
            if (buffer[readBytes-1] != '\n') fputc('\n', g_emulator->f_stdout);
            fflush(g_emulator->f_stdout);
        }
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
    // Link components to MMU for coherent access
    emu->mmu.timer = &emu->timer;
    mmu_set_joypad(&emu->mmu, &emu->joypad);
    mmu_set_ppu(&emu->mmu, &emu->ppu);
    mmu_set_serial_callback(&emu->mmu, gui_serial_callback);
    
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
    // Brancher callback IRQ joypad -> MMU IF bit 4
    joypad_set_irq_callback(&emu->joypad, (void(*)(void*))mmu_request_joypad_irq, &emu->mmu);
    
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
    if (emu->f_log) { fclose(emu->f_log); emu->f_log = NULL; }
    if (emu->f_serial) { fclose(emu->f_serial); emu->f_serial = NULL; }
    if (emu->f_stdout) { fclose(emu->f_stdout); emu->f_stdout = NULL; }
}

// Fonction pour charger une ROM
bool emulator_load_rom(Emulator* emu, const char* filename) {
    bool ok = mmu_load_rom(&emu->mmu, filename);
    if (ok) {
        // Prepare logs directory and files under logs/rom/<romname>
        // Extract base name
        const char* base = filename;
        for (const char* p = filename; *p; ++p) if (*p=='/'||*p=='\\') base = p+1;
        char name[128]={0}; size_t n=0; while(base[n] && base[n]!='.' && n<sizeof(name)-1){name[n]=base[n];n++;}
        for (size_t i=0;i<n;i++){char c=name[i]; if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'||c=='-')) name[i]='_';}
        strncpy(emu->rom_name, name, sizeof(emu->rom_name)-1);
        CreateDirectoryA("logs", NULL);
        CreateDirectoryA("logs\\rom", NULL);
        snprintf(emu->log_dir, sizeof(emu->log_dir), "logs\\rom\\%s", emu->rom_name);
        CreateDirectoryA(emu->log_dir, NULL);
        char path[260];
        snprintf(path, sizeof(path), "%s\\%s.log", emu->log_dir, emu->rom_name); emu->f_log=fopen(path,"w");
        snprintf(path, sizeof(path), "%s\\%s_serial.log", emu->log_dir, emu->rom_name); emu->f_serial=fopen(path,"w");
        snprintf(path, sizeof(path), "%s\\%s_stdout.txt", emu->log_dir, emu->rom_name); emu->f_stdout=fopen(path,"w");
        gui_log("ROM chargee: %s\n", filename);
        gui_log("Type de cartouche: %s\n", cart_type_name(emu->mmu.cart.type));
    }
    return ok;
}

// Fonction pour exécuter l'émulateur
void emulator_run(Emulator* emu) {
    if (!emu->running) return;
    
    // Afficher la fenêtre
    graphics_win32_gui_show(&emu->graphics);
    
    // Phase de rechauffement CPU pour initialiser la ROM (alignée sur la version run)
    gui_log("Phase de rechauffement CPU...\n");
    const u32 warmup_cycles_target = 300000; // ~0.07s @ 4.19MHz
    u32 warmup_cycles = 0;
    while (warmup_cycles < warmup_cycles_target) {
        u8 cycles = cpu_step(&emu->cpu, &emu->mmu);
        warmup_cycles += cycles;
        timer_tick(&emu->timer, cycles);
        u8 ppu_interrupts_warm = ppu_tick(&emu->ppu, cycles, emu->mmu.vram);
        mmu_dma_tick(&emu->mmu, cycles);
        u8 timer_interrupts_warm = timer_get_interrupts(&emu->timer);
        if (ppu_interrupts_warm) interrupt_request(&emu->interrupt, ppu_interrupts_warm);
        if (timer_interrupts_warm) interrupt_request(&emu->interrupt, timer_interrupts_warm);
        interrupt_write_ie(&emu->interrupt, mmu_read8(&emu->mmu, IE_REG));
        interrupt_write_if(&emu->interrupt, mmu_read8(&emu->mmu, IF_REG));
        u8 handled_warm = interrupt_handle(&emu->interrupt, &emu->cpu, &emu->mmu);
        if (handled_warm) {
            mmu_write8(&emu->mmu, IF_REG, interrupt_read_if(&emu->interrupt));
        }
    }
    
    // Forcer un rendu initial
    gui_log("Rendu initial...\n");
    for (int line = 0; line < 144; line++) {
        ppu_render_line(&emu->ppu, emu->mmu.vram);
    }
    graphics_win32_gui_update(&emu->graphics, (u32*)emu->ppu.framebuffer);
    graphics_win32_gui_present(&emu->graphics);
    
    gui_log("Chargement initial termine\n");
    printf("Chargement initial termine\n");
    
    gui_log("Demarrage de la boucle principale...\n");
    
    // Boucle principale d'émulation
    while (emu->running && emu->graphics.running) {
        // Exécuter une instruction CPU
        u8 cycles = cpu_step(&emu->cpu, &emu->mmu);
        
        // Mettre à jour le timer
        timer_tick(&emu->timer, cycles);
        
        // Update PPU and DMA; collect and handle interrupts via unified path
        u8 ppu_interrupts = ppu_tick(&emu->ppu, cycles, emu->mmu.vram);
        mmu_dma_tick(&emu->mmu, cycles);
        u8 timer_interrupts = timer_get_interrupts(&emu->timer);
        if (ppu_interrupts) interrupt_request(&emu->interrupt, ppu_interrupts);
        if (timer_interrupts) interrupt_request(&emu->interrupt, timer_interrupts);
        interrupt_write_ie(&emu->interrupt, mmu_read8(&emu->mmu, IE_REG));
        interrupt_write_if(&emu->interrupt, mmu_read8(&emu->mmu, IF_REG));
        u8 handled = interrupt_handle(&emu->interrupt, &emu->cpu, &emu->mmu);
        if (handled) {
            mmu_write8(&emu->mmu, IF_REG, interrupt_read_if(&emu->interrupt));
        }
        
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

            static unsigned int frame_count = 0;
            frame_count++;
            if ((frame_count % 60) == 0) {
                char buf[128];
                int n = snprintf(buf, sizeof(buf), "Frame %u: running=%d, graphics_running=%d\n", 
                                 frame_count, emu->running ? 1 : 0, emu->graphics.running ? 1 : 0);
                if (n > 0) {
                    gui_log("%.*s", n, buf);
                    printf("%s", buf);
                }
            }
        }
        
        // Gérer les événements
        graphics_win32_gui_handle_events(&emu->graphics, &emu->running);
        if (!emu->running || !emu->graphics.running || !IsWindow(emu->graphics.hwnd)) {
            emu->running = false;
            break;
        }
        
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

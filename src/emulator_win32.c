#include "common.h"
#include "mmu.h"
#include "cpu.h"
#include "timer.h"
#include "ppu.h"
#include "joypad.h"
#include "graphics_win32.h"
#include <windows.h>

// Structure principale de l'émulateur
typedef struct {
    CPU cpu;
    MMU mmu;
    Timer timer;
    PPU ppu;
    Joypad joypad;
    GraphicsWin32 graphics;
    
    bool running;
    u32 cycles_per_frame;
    u32 current_cycles;
} Emulator;

// Initialisation de l'émulateur
void emulator_init(Emulator* emu) {
    memset(emu, 0, sizeof(Emulator));
    
    cpu_init(&emu->cpu);
    mmu_init(&emu->mmu);
    timer_init(&emu->timer);
    ppu_init(&emu->ppu);
    joypad_init(&emu->joypad);
    
    if (!graphics_win32_init(&emu->graphics)) {
        printf("Erreur: Impossible d'initialiser l'interface graphique\n");
        exit(1);
    }
    
    emu->running = true;
    emu->cycles_per_frame = GB_FREQ / 60;  // 60 FPS
    emu->current_cycles = 0;
}

// Nettoyage de l'émulateur
void emulator_cleanup(Emulator* emu) {
    mmu_cleanup(&emu->mmu);
    graphics_win32_cleanup(&emu->graphics);
}

// Boucle principale d'émulation
void emulator_run(Emulator* emu) {
    printf("Démarrage de l'émulation avec interface graphique...\n");
    printf("Appuyez sur Échap pour quitter\n\n");
    
    // Afficher la fenêtre
    graphics_win32_show(&emu->graphics);
    
    // Exécution initiale du CPU pour permettre à la ROM d'initialiser VRAM/BG map
    printf("Exécution initiale du CPU pour charger les tiles...\n");
    const u32 warmup_cycles_target = 300000; // ~0.07s @ 4.19MHz
    u32 warmup_cycles = 0;
    while (warmup_cycles < warmup_cycles_target) {
        u8 cycles = cpu_step(&emu->cpu, &emu->mmu);
        warmup_cycles += cycles;
        timer_tick(&emu->timer, cycles);
        ppu_tick(&emu->ppu, cycles, emu->mmu.vram);
        
        // Gérer interruptions pendant warmup
        u8 if_reg = mmu_read8(&emu->mmu, IF_REG);
        u8 ie_reg = mmu_read8(&emu->mmu, IE_REG);
        if (emu->cpu.ime && (if_reg & ie_reg)) {
            for (int i = 0; i < 5; i++) {
                u8 interrupt = (u8)(1 << i);
                if ((if_reg & interrupt) && (ie_reg & interrupt)) {
                    cpu_interrupt(&emu->cpu, &emu->mmu, interrupt);
                    mmu_write8(&emu->mmu, IF_REG, (u8)(if_reg & (u8)~interrupt));
                    break;
                }
            }
        }
    }
    printf("Chargement initial terminé\n");
    
    // Forcer un premier rendu complet
    for (int y = 0; y < GB_HEIGHT; y++) {
        emu->ppu.ly = (u8)y;
        ppu_render_line(&emu->ppu, emu->mmu.vram);
    }
    graphics_win32_update(&emu->graphics, (u32*)emu->ppu.framebuffer);
    graphics_win32_present(&emu->graphics);
    
    while (emu->running && emu->graphics.running) {
        // Exécuter une instruction CPU
        u8 cycles = cpu_step(&emu->cpu, &emu->mmu);
        emu->current_cycles += cycles;
        
        // Mettre à jour les composants
        timer_tick(&emu->timer, cycles);
        ppu_tick(&emu->ppu, cycles, emu->mmu.vram);
        
        // Vérifier les interruptions
        u8 if_reg = mmu_read8(&emu->mmu, IF_REG);
        u8 ie_reg = mmu_read8(&emu->mmu, IE_REG);
        
        if (emu->cpu.ime && (if_reg & ie_reg)) {
            // Trouver la première interruption active
            for (int i = 0; i < 5; i++) {
                u8 interrupt = 1 << i;
                if ((if_reg & interrupt) && (ie_reg & interrupt)) {
                    cpu_interrupt(&emu->cpu, &emu->mmu, interrupt);
                    // Effacer le flag d'interruption
                    mmu_write8(&emu->mmu, IF_REG, if_reg & ~interrupt);
                    break;
                }
            }
        }
        
        // Vérifier si on a atteint une frame
        if (emu->current_cycles >= emu->cycles_per_frame) {
            emu->current_cycles -= emu->cycles_per_frame;
            
            // Debug: afficher BG map une fois
            static bool debug_done = false;
            if (!debug_done) {
                printf("BG Map (0x9800-0x981F): ");
                for (int i = 0; i < 32; i++) {
                    printf("%02X ", emu->mmu.vram[0x1800 + i]);
                }
                printf("\n");
                debug_done = true;
            }
            
            // Rendre toutes les lignes de la frame
            for (int y = 0; y < GB_HEIGHT; y++) {
                emu->ppu.ly = (u8)y;
                ppu_render_line(&emu->ppu, emu->mmu.vram);
            }
            graphics_win32_update(&emu->graphics, (u32*)emu->ppu.framebuffer);
            graphics_win32_present(&emu->graphics);
        }
        
        // Gérer les événements
        graphics_win32_handle_events(&emu->graphics, &emu->running);
        // Si la fenêtre a reçu WM_CLOSE, on ne quitte plus automatiquement
        if (!emu->graphics.running && emu->running) {
            // Réactiver l'état running de la fenêtre tant que l'utilisateur n'appuie pas ESC
            emu->graphics.running = true;
        }
        
        // Simple delay to keep window open
        static int frame_count = 0;
        frame_count++;
        if (frame_count % 60 == 0) {  // Toutes les 60 frames (1 seconde)
            printf("Frame %d: running=%d, graphics_running=%d\n", 
                   frame_count, emu->running, emu->graphics.running);
        }
        
        // Add small delay to prevent overwhelming the system
        Sleep(1);
        
        // Check for errors that might cause early exit
        if (!emu->running) {
            printf("Emulator stopped: running=%d\n", emu->running);
            break;
        }
        if (!emu->graphics.running) {
            printf("Graphics stopped: graphics_running=%d\n", emu->graphics.running);
            break;
        }
        
        // Debug: vérifier si le CPU tourne en boucle
        static u16 last_pc = 0;
        static int loop_count = 0;
        if (emu->cpu.pc == last_pc) {
            loop_count++;
            if (loop_count > 1000) {
                printf("CPU en boucle infinie à PC=0x%04X\n", emu->cpu.pc);
                loop_count = 0; // Reset pour éviter le spam
            }
        } else {
            last_pc = emu->cpu.pc;
            loop_count = 0;
        }
    }
    // Boucle sortie: diagnostiquer états
    printf("Boucle d'émulation terminée: running=%d, graphics_running=%d\n", emu->running, emu->graphics.running);

    // Maintenir la fenêtre affichée jusqu'à ESC, même si l'émulation s'arrête
    printf("Maintien de la fenêtre ouverte (ESC pour quitter)\n");
    emu->graphics.running = true; // s'assurer que la fenêtre reste active
    while (1) {
        graphics_win32_handle_events(&emu->graphics, &emu->running);
        if (!emu->running || !emu->graphics.running) break;
        Sleep(16);
    }
    
    printf("Fermeture de la fenêtre\n");
}

// Fonction principale
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <rom_file>\n", argv[0]);
        return 1;
    }
    
    Emulator emu;
    emulator_init(&emu);
    
    // Charger la ROM
    if (!mmu_load_rom(&emu.mmu, argv[1])) {
        printf("Erreur: Impossible de charger la ROM %s\n", argv[1]);
        emulator_cleanup(&emu);
        return 1;
    }
    
    printf("ROM chargée: %s\n", emu.mmu.cart.header.title);
    printf("Type de cartouche: %s\n", cart_type_name(emu.mmu.cart.type));
    printf("Taille ROM: %u KB\n", emu.mmu.cart.header.rom_size);
    
    // Lancer l'émulation
    emulator_run(&emu);
    
    emulator_cleanup(&emu);
    return 0;
}

#include "common.h"
#include "mmu.h"
#include "cpu.h"
#include "timer.h"
#include "ppu.h"
#include "joypad.h"
#include "graphics_win32.h"

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
            
            // Rendre la frame
            ppu_render_line(&emu->ppu, emu->mmu.vram);
            graphics_win32_update(&emu->graphics, (u8*)emu->ppu.framebuffer);
            graphics_win32_present(&emu->graphics);
        }
        
        // Gérer les événements
        graphics_win32_handle_events(&emu->graphics, &emu->running);
    }
    
    printf("Émulation terminée\n");
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

#include "common.h"
#include "mmu.h"
#include "cpu.h"
#include "timer.h"
#include "ppu.h"
#include "joypad.h"

// Structure principale de l'émulateur simple
typedef struct {
    CPU cpu;
    MMU mmu;
    Timer timer;
    PPU ppu;
    Joypad joypad;
    
    bool running;
    u32 cycles_per_frame;
    u32 current_cycles;
} EmulatorSimple;

// Initialisation de l'émulateur simple
void emulator_simple_init(EmulatorSimple* emu) {
    memset(emu, 0, sizeof(EmulatorSimple));
    
    cpu_init(&emu->cpu);
    mmu_init(&emu->mmu);
    timer_init(&emu->timer);
    ppu_init(&emu->ppu);
    joypad_init(&emu->joypad);
    
    emu->running = true;
    emu->cycles_per_frame = GB_FREQ / 60;  // 60 FPS
    emu->current_cycles = 0;
}

// Nettoyage de l'émulateur simple
void emulator_simple_cleanup(EmulatorSimple* emu) {
    mmu_cleanup(&emu->mmu);
}

// Boucle principale d'émulation simple (sans graphiques)
void emulator_simple_run(EmulatorSimple* emu, u32 max_cycles) {
    printf("Démarrage de l'émulation simple...\n");
    printf("Cycles maximum: %u\n", max_cycles);
    printf("PC initial: 0x%04X\n", emu->cpu.pc);
    printf("Première instruction: 0x%02X\n", emu->mmu.memory[emu->cpu.pc]);
    printf("\n");
    
    u32 total_cycles = 0;
    
    while (emu->running && total_cycles < max_cycles) {
        // Log de debug pour identifier le problème
        if (total_cycles < 200) {
            printf("PC: 0x%04X, Opcode: 0x%02X, AF: 0x%04X, Flags: Z=%d N=%d H=%d C=%d\n", 
                   emu->cpu.pc, emu->mmu.memory[emu->cpu.pc], emu->cpu.af,
                   (emu->cpu.af & FLAG_Z) ? 1 : 0,
                   (emu->cpu.af & FLAG_N) ? 1 : 0,
                   (emu->cpu.af & FLAG_H) ? 1 : 0,
                   (emu->cpu.af & FLAG_C) ? 1 : 0);
        }
        
        // Exécuter une instruction CPU
        u8 cycles = cpu_step(&emu->cpu, &emu->mmu);
        emu->current_cycles += cycles;
        total_cycles += cycles;
        
        // Mettre à jour les composants
        timer_tick(&emu->timer, cycles);
        ppu_tick(&emu->ppu, cycles);
        
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
            
            // Rendre la frame (sans affichage)
            ppu_render_line(&emu->ppu, emu->mmu.vram);
        }
        
        // Arrêter si PC pointe vers une adresse invalide
        if (emu->cpu.pc >= 0x10000) {
            printf("Arrêt: PC invalide (0x%04X)\n", emu->cpu.pc);
            break;
        }
    }
    
    printf("Émulation terminée après %u cycles\n", total_cycles);
    printf("PC final: 0x%04X\n", emu->cpu.pc);
    printf("AF: 0x%04X, BC: 0x%04X, DE: 0x%04X, HL: 0x%04X\n", 
           emu->cpu.af, emu->cpu.bc, emu->cpu.de, emu->cpu.hl);
    printf("SP: 0x%04X\n", emu->cpu.sp);
}

// Fonction principale
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <rom_file> [max_cycles]\n", argv[0]);
        printf("  max_cycles: nombre maximum de cycles (défaut: 1000000)\n");
        return 1;
    }
    
    EmulatorSimple emu;
    emulator_simple_init(&emu);
    
    // Charger la ROM
    printf("Tentative de chargement de la ROM: %s\n", argv[1]);
    if (!mmu_load_rom(&emu.mmu, argv[1])) {
        printf("Erreur: Impossible de charger la ROM %s\n", argv[1]);
        emulator_simple_cleanup(&emu);
        return 1;
    }
    printf("ROM chargée avec succès!\n");
    
    printf("ROM chargée: %s\n", emu.mmu.cart.header.title);
    printf("Type de cartouche: %s\n", cart_type_name(emu.mmu.cart.type));
    printf("Taille ROM: %u KB\n", emu.mmu.cart.header.rom_size);
    
    // Nombre maximum de cycles
    u32 max_cycles = 1000000; // 1M cycles par défaut
    if (argc >= 3) {
        max_cycles = (u32)atoi(argv[2]);
    }
    
    // Lancer l'émulation
    emulator_simple_run(&emu, max_cycles);
    
    emulator_simple_cleanup(&emu);
    return 0;
}

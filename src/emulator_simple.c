#include <stdio.h>
#include "common.h"
#include "mmu.h"
#include "cpu.h"
#include "timer.h"
#include "ppu.h"
#include "joypad.h"
#include "apu.h"
#include "interrupt.h"
#include "graphics_win32.h"

// Charger des tiles de caractères ASCII depuis console.bin
void load_console_tiles(u8* vram) {
    FILE* f = fopen("console.bin", "rb");
    if (!f) {
        printf("Erreur: impossible d'ouvrir console.bin, utilisation des tiles par défaut\n");
        load_ascii_tiles(vram);
        return;
    }
    
    // Lire 96 caractères * 16 octets = 1536 octets
    size_t read = fread(vram + 0x200, 1, 1536, f);
    fclose(f);
    
    if (read != 1536) {
        printf("Erreur: lecture incomplète de console.bin (%zu/1536 octets)\n", read);
        load_ascii_tiles(vram);
        return;
    }
    
    printf("Tiles ASCII chargées depuis console.bin (%zu octets)\n", read);
}

// Charger des tiles de caractères ASCII basiques
void load_ascii_tiles(u8* vram) {
    // Tiles ASCII simples (8x8 pixels chacune)
    // Chaque caractère prend 16 octets (2 octets par ligne, 8 lignes)
    
    // Tile 0: Espace (vide)
    for (int i = 0; i < 16; i++) {
        vram[0x200 + i] = 0x00;
    }
    
    // Tile 1: 'A' (lettre A simple)
    u8 tile_a[16] = {
        0x00, 0x00,  // ........
        0x18, 0x00,  // ...##...
        0x24, 0x00,  // ..#..#..
        0x24, 0x00,  // ..#..#..
        0x3C, 0x00,  // ..####..
        0x24, 0x00,  // ..#..#..
        0x24, 0x00,  // ..#..#..
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 16 + i] = tile_a[i];
    }
    
    // Tile 2: 'B'
    u8 tile_b[16] = {
        0x00, 0x00,  // ........
        0x38, 0x00,  // ..###...
        0x24, 0x00,  // ..#..#..
        0x38, 0x00,  // ..###...
        0x24, 0x00,  // ..#..#..
        0x24, 0x00,  // ..#..#..
        0x38, 0x00,  // ..###...
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 32 + i] = tile_b[i];
    }
    
    // Tile 3: 'C'
    u8 tile_c[16] = {
        0x00, 0x00,  // ........
        0x1C, 0x00,  // ...###..
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x1C, 0x00,  // ...###..
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 48 + i] = tile_c[i];
    }
    
    // Tile 4: 'D'
    u8 tile_d[16] = {
        0x00, 0x00,  // ........
        0x38, 0x00,  // ..###...
        0x24, 0x00,  // ..#..#..
        0x24, 0x00,  // ..#..#..
        0x24, 0x00,  // ..#..#..
        0x24, 0x00,  // ..#..#..
        0x38, 0x00,  // ..###...
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 64 + i] = tile_d[i];
    }
    
    // Tile 5: 'E'
    u8 tile_e[16] = {
        0x00, 0x00,  // ........
        0x3C, 0x00,  // ..####..
        0x20, 0x00,  // ..#.....
        0x38, 0x00,  // ..###...
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x3C, 0x00,  // ..####..
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 80 + i] = tile_e[i];
    }
    
    // Tile 6: 'F'
    u8 tile_f[16] = {
        0x00, 0x00,  // ........
        0x3C, 0x00,  // ..####..
        0x20, 0x00,  // ..#.....
        0x38, 0x00,  // ..###...
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 96 + i] = tile_f[i];
    }
    
    // Tile 7: 'G'
    u8 tile_g[16] = {
        0x00, 0x00,  // ........
        0x1C, 0x00,  // ...###..
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x26, 0x00,  // ..#..##.
        0x22, 0x00,  // ..#...#.
        0x1C, 0x00,  // ...###..
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 112 + i] = tile_g[i];
    }
    
    // Tile 8: 'H'
    u8 tile_h[16] = {
        0x00, 0x00,  // ........
        0x24, 0x00,  // ..#..#..
        0x24, 0x00,  // ..#..#..
        0x3C, 0x00,  // ..####..
        0x24, 0x00,  // ..#..#..
        0x24, 0x00,  // ..#..#..
        0x24, 0x00,  // ..#..#..
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 128 + i] = tile_h[i];
    }
    
    // Tile 9: 'I'
    u8 tile_i[16] = {
        0x00, 0x00,  // ........
        0x1C, 0x00,  // ...###..
        0x08, 0x00,  // ....#...
        0x08, 0x00,  // ....#...
        0x08, 0x00,  // ....#...
        0x08, 0x00,  // ....#...
        0x1C, 0x00,  // ...###..
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 144 + i] = tile_i[i];
    }
    
    // Tile 10: 'L'
    u8 tile_l[16] = {
        0x00, 0x00,  // ........
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x3C, 0x00,  // ..####..
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 160 + i] = tile_l[i];
    }
    
    // Tile 11: 'O'
    u8 tile_o[16] = {
        0x00, 0x00,  // ........
        0x1C, 0x00,  // ...###..
        0x22, 0x00,  // ..#...#.
        0x22, 0x00,  // ..#...#.
        0x22, 0x00,  // ..#...#.
        0x22, 0x00,  // ..#...#.
        0x1C, 0x00,  // ...###..
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 176 + i] = tile_o[i];
    }
    
    // Tile 12: 'P'
    u8 tile_p[16] = {
        0x00, 0x00,  // ........
        0x38, 0x00,  // ..###...
        0x24, 0x00,  // ..#..#..
        0x24, 0x00,  // ..#..#..
        0x38, 0x00,  // ..###...
        0x20, 0x00,  // ..#.....
        0x20, 0x00,  // ..#.....
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 192 + i] = tile_p[i];
    }
    
    // Tile 13: 'S'
    u8 tile_s[16] = {
        0x00, 0x00,  // ........
        0x1C, 0x00,  // ...###..
        0x20, 0x00,  // ..#.....
        0x1C, 0x00,  // ...###..
        0x02, 0x00,  // ......#.
        0x02, 0x00,  // ......#.
        0x1C, 0x00,  // ...###..
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 208 + i] = tile_s[i];
    }
    
    // Tile 14: 'T'
    u8 tile_t[16] = {
        0x00, 0x00,  // ........
        0x3E, 0x00,  // ..#####.
        0x08, 0x00,  // ....#...
        0x08, 0x00,  // ....#...
        0x08, 0x00,  // ....#...
        0x08, 0x00,  // ....#...
        0x08, 0x00,  // ....#...
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 224 + i] = tile_t[i];
    }
    
    // Tile 15: 'U'
    u8 tile_u[16] = {
        0x00, 0x00,  // ........
        0x22, 0x00,  // ..#...#.
        0x22, 0x00,  // ..#...#.
        0x22, 0x00,  // ..#...#.
        0x22, 0x00,  // ..#...#.
        0x22, 0x00,  // ..#...#.
        0x1C, 0x00,  // ...###..
        0x00, 0x00   // ........
    };
    for (int i = 0; i < 16; i++) {
        vram[0x200 + 240 + i] = tile_u[i];
    }
    
    // Remplir le reste avec des espaces
    for (int i = 16; i < 96; i++) {
        for (int j = 0; j < 16; j++) {
            vram[0x200 + i * 16 + j] = 0x00;
        }
    }
}

// Structure principale de l'émulateur simple
typedef struct {
    CPU cpu;
    MMU mmu;
    Timer timer;
    PPU ppu;
    Joypad joypad;
    APU apu;
    InterruptManager interrupt_mgr;
    GraphicsWin32 graphics;
    
    bool running;
    u32 cycles_per_frame;
    u32 current_cycles;
    bool show_lcd;
} EmulatorSimple;

// Initialisation de l'émulateur simple
void emulator_simple_init(EmulatorSimple* emu) {
    memset(emu, 0, sizeof(EmulatorSimple));
    
    cpu_init(&emu->cpu);
    mmu_init(&emu->mmu);
    timer_init(&emu->timer);
    ppu_init(&emu->ppu);
    joypad_init(&emu->joypad);
    apu_init(&emu->apu);
    interrupt_init(&emu->interrupt_mgr);
    
    // Connecter le timer et l'APU au MMU
    emu->mmu.timer = &emu->timer;
    emu->mmu.apu = &emu->apu;
    
    // Initialiser les graphiques (caché par défaut)
    if (!graphics_win32_init(&emu->graphics)) {
        printf("Erreur: Impossible d'initialiser les graphiques\n");
        emu->show_lcd = false;
    } else {
        printf("Graphiques initialisés avec succès\n");
        emu->show_lcd = false; // Commencer caché
    }
    
    emu->running = true;
    emu->cycles_per_frame = GB_FREQ / 60;  // 60 FPS
    emu->current_cycles = 0;
}

// Nettoyage de l'émulateur simple
void emulator_simple_cleanup(EmulatorSimple* emu) {
    mmu_cleanup(&emu->mmu);
    apu_cleanup(&emu->apu);
    graphics_win32_cleanup(&emu->graphics);
}

// Activer l'affichage LCD
void emulator_simple_show_lcd(EmulatorSimple* emu) {
    emu->show_lcd = true;
    graphics_win32_show(&emu->graphics);
    printf("Affichage LCD activé\n");
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
        // Log de debug réduit
        if (total_cycles < 50) {
            printf("PC: 0x%04X, Opcode: 0x%02X\n", emu->cpu.pc, emu->mmu.memory[emu->cpu.pc]);
        }
        
        // Exécuter une instruction CPU
        u8 cycles = cpu_step(&emu->cpu, &emu->mmu);
        emu->current_cycles += cycles;
        total_cycles += cycles;
        
        // Log spécial pour la zone de test Blargg
        if (emu->cpu.pc >= 0x0200 && emu->cpu.pc <= 0x0220) {
            printf("ZONE TEST: PC=0x%04X, AF=0x%04X, BC=0x%04X, DE=0x%04X, HL=0x%04X, SP=0x%04X\n", 
                   emu->cpu.pc, emu->cpu.af, emu->cpu.bc, emu->cpu.de, emu->cpu.hl, emu->cpu.sp);
        }
        
        // Log détaillé réduit
        if (total_cycles < 50) {
            printf("Cycle %u: PC=0x%04X, AF=0x%04X\n", total_cycles, emu->cpu.pc, emu->cpu.af);
        }
        
        // Log spécial pour les accès port série
        if (emu->cpu.pc >= 0x0200 && emu->cpu.pc <= 0x0300) {
            printf("Zone test: PC=0x%04X, Opcode=0x%02X\n", emu->cpu.pc, emu->mmu.memory[emu->cpu.pc]);
        }
        
        // Mettre à jour les composants
        timer_tick(&emu->timer, cycles);
        u8 ppu_interrupts = ppu_tick(&emu->ppu, cycles, emu->mmu.vram);
        u8 timer_interrupts = timer_get_interrupts(&emu->timer);
        apu_tick(&emu->apu, cycles);
        
        // Mettre à jour l'affichage LCD si nécessaire
        if (emu->show_lcd) {
            graphics_win32_update(&emu->graphics, emu->ppu.framebuffer);
            graphics_win32_handle_events(&emu->graphics, &emu->running);
            
            // Vérifier si la fenêtre a été fermée
            if (!emu->graphics.running) {
                printf("Fenêtre fermée par l'utilisateur\n");
                break;
            }
        }
        
        // Ajouter les interruptions au gestionnaire d'interruptions
        if (ppu_interrupts) {
            interrupt_request(&emu->interrupt_mgr, ppu_interrupts);
        }
        if (timer_interrupts) {
            interrupt_request(&emu->interrupt_mgr, timer_interrupts);
        }
        
        // Synchroniser les registres IE et IF avec le gestionnaire
        interrupt_write_ie(&emu->interrupt_mgr, mmu_read8(&emu->mmu, IE_REG));
        interrupt_write_if(&emu->interrupt_mgr, mmu_read8(&emu->mmu, IF_REG));
        
        // Traiter les interruptions
        u8 handled_interrupt = interrupt_handle(&emu->interrupt_mgr, &emu->cpu, &emu->mmu);
        if (handled_interrupt) {
            // Synchroniser le registre IF avec le gestionnaire
            mmu_write8(&emu->mmu, IF_REG, interrupt_read_if(&emu->interrupt_mgr));
            if (total_cycles < 1000) { // Log seulement les 1000 premiers cycles
                printf("Interruption traitée: %s (0x%02X)\n", 
                       interrupt_get_name(handled_interrupt), handled_interrupt);
            }
        }
        
        // Vérifier si on a atteint une frame
        if (emu->current_cycles >= emu->cycles_per_frame) {
            emu->current_cycles -= emu->cycles_per_frame;
            
            // Rendre la frame complète
            if (emu->show_lcd) {
                for (int y = 0; y < GB_HEIGHT; y++) {
                    emu->ppu.ly = y;
                    ppu_render_line(&emu->ppu, emu->mmu.vram);
                }
                graphics_win32_update(&emu->graphics, emu->ppu.framebuffer);
                graphics_win32_present(&emu->graphics);
            }
        }
        
        // Si on a l'affichage LCD, continuer indéfiniment jusqu'à fermeture manuelle
        if (emu->show_lcd && total_cycles >= max_cycles) {
            printf("Cycles maximum atteints, mais LCD ouvert - continuer jusqu'à fermeture manuelle...\n");
            max_cycles = 0xFFFFFFFF; // Continuer indéfiniment
        }
        
        // Si LCD ouvert, ne jamais s'arrêter automatiquement
        if (emu->show_lcd) {
            max_cycles = 0xFFFFFFFF;
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
    
    // Activer l'affichage LCD pour les tests Blargg
    emulator_simple_show_lcd(&emu);
    
    // Laisser le CPU s'exécuter d'abord pour charger les tiles
    
    // Nombre maximum de cycles
    u32 max_cycles = 1000000; // 1M cycles par défaut
    if (argc >= 3) {
        max_cycles = (u32)atoi(argv[2]);
    }
    
    // Si l'affichage LCD est activé, augmenter le nombre de cycles
    if (emu.show_lcd) {
        max_cycles = 10000000; // 10M cycles pour voir l'affichage
        printf("Affichage LCD activé - cycles augmentés à %u\n", max_cycles);
        
        // Laisser le CPU s'exécuter un peu avant de commencer le rendu
        printf("Exécution initiale du CPU pour charger les tiles...\n");
        for (int i = 0; i < 10000; i++) {
            u8 cycles = cpu_step(&emu.cpu, &emu.mmu);
            timer_tick(&emu.timer, cycles);
            ppu_tick(&emu.ppu, cycles, emu.mmu.vram);
        }
        printf("Chargement initial terminé\n");
        
        // Charger des tiles de caractères ASCII depuis console.bin
        printf("Chargement des tiles ASCII depuis console.bin...\n");
        load_console_tiles(emu.mmu.vram);
        
        // Vérifier si des tiles ont été chargées
        printf("Vérification des tiles chargées...\n");
        for (int i = 0; i < 16; i++) {
            printf("Tile %d: ", i);
            for (int j = 0; j < 16; j++) {
                printf("%02X ", emu.mmu.vram[i * 16 + j]);
            }
            printf("\n");
        }
    }
    
    // Lancer l'émulation
    emulator_simple_run(&emu, max_cycles);
    
    emulator_simple_cleanup(&emu);
    return 0;
}

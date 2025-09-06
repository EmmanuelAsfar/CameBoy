#include <stdio.h>
#include <stdint.h>
#include "src/mmu.h"
#include "src/cpu.h"

int main() {
    printf("=== Test du port série ===\n");
    
    // Initialiser MMU et CPU
    MMU mmu;
    CPU cpu;
    
    mmu_init(&mmu);
    cpu_init(&cpu);
    
    // Charger une ROM simple
    if (!mmu_load_rom(&mmu, "simple_test.gb")) {
        printf("Erreur: Impossible de charger simple_test.gb\n");
        return 1;
    }
    
    printf("ROM chargée\n");
    
    // Test direct du port série
    printf("\n--- Test direct du port série ---\n");
    
    // Écrire "H" sur le port série
    mmu_write8(&mmu, 0xFF01, 'H');  // SB = 'H'
    mmu_write8(&mmu, 0xFF02, 0x81); // SC = 0x81 (transmission)
    
    // Écrire "E"
    mmu_write8(&mmu, 0xFF01, 'E');
    mmu_write8(&mmu, 0xFF02, 0x81);
    
    // Écrire "L"
    mmu_write8(&mmu, 0xFF01, 'L');
    mmu_write8(&mmu, 0xFF02, 0x81);
    
    // Écrire "L"
    mmu_write8(&mmu, 0xFF01, 'L');
    mmu_write8(&mmu, 0xFF02, 0x81);
    
    // Écrire "O"
    mmu_write8(&mmu, 0xFF01, 'O');
    mmu_write8(&mmu, 0xFF02, 0x81);
    
    printf("\n--- Test via CPU ---\n");
    
    // Programmer le CPU pour écrire "HELLO"
    // LD A, 'H'
    mmu.memory[0x100] = 0x3E;  // LD A, imm8
    mmu.memory[0x101] = 'H';
    // LD (0xFF01), A
    mmu.memory[0x102] = 0xE0;  // LD (0xFF00+imm8), A
    mmu.memory[0x103] = 0x01;  // 0xFF01
    // LD A, 0x81
    mmu.memory[0x104] = 0x3E;  // LD A, imm8
    mmu.memory[0x105] = 0x81;
    // LD (0xFF02), A
    mmu.memory[0x106] = 0xE0;  // LD (0xFF00+imm8), A
    mmu.memory[0x107] = 0x02;  // 0xFF02
    
    // HALT
    mmu.memory[0x108] = 0x76;
    
    cpu.pc = 0x100;
    
    // Exécuter quelques instructions
    for (int i = 0; i < 10; i++) {
        u8 cycles = cpu_step(&cpu, &mmu);
        printf("Cycle %d: PC=0x%04X, A=0x%02X\n", i, cpu.pc, (cpu.af >> 8) & 0xFF);
    }
    
    printf("\nTest terminé\n");
    return 0;
}

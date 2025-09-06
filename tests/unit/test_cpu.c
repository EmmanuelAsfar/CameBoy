/**
 * TESTS UNITAIRES POUR LE CPU LR35902
 *
 * Ce fichier contient des tests unitaires pour valider le fonctionnement
 * correct du CPU Game Boy selon les spécifications Pan Docs.
 */

#include "../../src/common.h"
#include "../../src/cpu.h"
#include "../../src/mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Constantes pour les tests (adresses des handlers d'interruption)
#define INT_VBLANK_ADDR   0x0040
#define INT_LCD_STAT_ADDR 0x0048
#define INT_TIMER_ADDR    0x0050
#define INT_SERIAL_ADDR   0x0058
#define INT_JOYPAD_ADDR   0x0060

// Structure pour les tests
typedef struct {
    const char* name;
    void (*test_func)(void);
} UnitTest;

// Prototypes des fonctions de test
void test_cpu_init(void);
void test_cpu_reset(void);
void test_cpu_flags(void);
void test_cpu_registers(void);
void test_cpu_arithmetic_add(void);
void test_cpu_arithmetic_sub(void);
void test_cpu_arithmetic_adc(void);
void test_cpu_arithmetic_sbc(void);
void test_cpu_logical_and(void);
void test_cpu_logical_or(void);
void test_cpu_logical_xor(void);
void test_cpu_logical_cp(void);
void test_cpu_load_ld_r8_r8(void);
void test_cpu_load_ld_r8_n8(void);
void test_cpu_load_ld_r16_n16(void);
void test_cpu_jumps_jr_nz(void);
void test_cpu_jumps_jr_z(void);
void test_cpu_jumps_jr_nc(void);
void test_cpu_jumps_jr_c(void);
void test_cpu_stack_push_pop(void);
void test_cpu_interrupts(void);

// Table des tests CPU
UnitTest cpu_tests[] = {
    {"CPU Initialisation", test_cpu_init},
    {"CPU Reset", test_cpu_reset},
    {"CPU Flags", test_cpu_flags},
    {"CPU Registers", test_cpu_registers},
    {"Arithmetic ADD", test_cpu_arithmetic_add},
    {"Arithmetic SUB", test_cpu_arithmetic_sub},
    {"Arithmetic ADC", test_cpu_arithmetic_adc},
    {"Arithmetic SBC", test_cpu_arithmetic_sbc},
    {"Logical AND", test_cpu_logical_and},
    {"Logical OR", test_cpu_logical_or},
    {"Logical XOR", test_cpu_logical_xor},
    {"Logical CP", test_cpu_logical_cp},
    {"Load LD r8,r8", test_cpu_load_ld_r8_r8},
    {"Load LD r8,n8", test_cpu_load_ld_r8_n8},
    {"Load LD r16,n16", test_cpu_load_ld_r16_n16},
    {"Jumps JR NZ", test_cpu_jumps_jr_nz},
    {"Jumps JR Z", test_cpu_jumps_jr_z},
    {"Jumps JR NC", test_cpu_jumps_jr_nc},
    {"Jumps JR C", test_cpu_jumps_jr_c},
    {"Stack PUSH/POP", test_cpu_stack_push_pop},
    {"Interrupts", test_cpu_interrupts},
    {NULL, NULL} // Marqueur de fin
};

/**
 * FONCTION PRINCIPALE DE TEST
 */
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    printf("=== TESTS UNITAIRES CPU LR35902 ===\n\n");

    int passed = 0;
    int total = 0;

    for (int i = 0; cpu_tests[i].name != NULL; i++) {
        printf("Test %d: %s... ", i + 1, cpu_tests[i].name);
        fflush(stdout);

        // Exécuter le test
        cpu_tests[i].test_func();

        printf("PASS\n");
        passed++;
        total++;
    }

    printf("\n=== RÉSULTATS ===\n");
    printf("Tests passés: %d/%d\n", passed, total);

    if (passed == total) {
        printf("✅ TOUS LES TESTS SONT PASSÉS !\n");
        return 0;
    } else {
        printf("❌ CERTAINS TESTS ONT ÉCHOUÉ\n");
        return 1;
    }
}

/**
 * IMPLEMENTATION DES TESTS
 */

void test_cpu_init(void) {
    CPU cpu;
    MMU mmu;

    // Initialiser
    cpu_init(&cpu);
    mmu_init(&mmu);

    // Vérifier les valeurs d'initialisation selon Pan Docs
    assert(cpu.af == 0x01B0); // A=0x01, F=0xB0 (Z=1, N=0, H=1, C=1)
    assert(cpu.bc == 0x0013);
    assert(cpu.de == 0x00D8);
    assert(cpu.hl == 0x014D);
    assert(cpu.sp == 0xFFFE);
    assert(cpu.pc == 0x0100);
    assert(!cpu.halted);
    assert(!cpu.ime);
    assert(!cpu.ei_pending);

    mmu_cleanup(&mmu);
}

void test_cpu_reset(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Modifier quelques valeurs
    cpu.af = 0x1234;
    cpu.pc = 0x2000;
    cpu.halted = true;

    // Reset
    cpu_reset(&cpu);

    // Vérifier que c'est revenu aux valeurs d'initialisation
    assert(cpu.af == 0x01B0);
    assert(cpu.pc == 0x0100);
    assert(!cpu.halted);

    mmu_cleanup(&mmu);
}

void test_cpu_flags(void) {
    CPU cpu;

    cpu_init(&cpu);

    // Test des flags individuels
    set_flag(&cpu, FLAG_Z, true);
    assert(get_flag(&cpu, FLAG_Z));
    assert(get_flags(&cpu) & FLAG_Z);

    set_flag(&cpu, FLAG_Z, false);
    assert(!get_flag(&cpu, FLAG_Z));
    assert(!(get_flags(&cpu) & FLAG_Z));

    // Test de tous les flags
    set_flags(&cpu, FLAG_Z | FLAG_N | FLAG_H | FLAG_C);
    assert(get_flag(&cpu, FLAG_Z));
    assert(get_flag(&cpu, FLAG_N));
    assert(get_flag(&cpu, FLAG_H));
    assert(get_flag(&cpu, FLAG_C));
}

void test_cpu_registers(void) {
    CPU cpu;

    cpu_init(&cpu);

    // Test des accesseurs/modificateurs de registres 8-bit
    set_reg_a(&cpu, 0xAB);
    assert(get_reg_a(&cpu) == 0xAB);

    set_reg_b(&cpu, 0xCD);
    assert(get_reg_b(&cpu) == 0xCD);

    set_reg_c(&cpu, 0xEF);
    assert(get_reg_c(&cpu) == 0xEF);

    // Vérifier que BC contient la bonne valeur
    assert(cpu.bc == 0xCDEF);
}

void test_cpu_arithmetic_add(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test ADD A, r8
    set_reg_a(&cpu, 0x0F);
    set_reg_b(&cpu, 0x01);

    // Simuler l'instruction ADD A, B (opcode 0x80)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0x80; // ADD A, B

    cpu_step(&cpu, &mmu);

    // Vérifier le résultat
    assert(get_reg_a(&cpu) == 0x10);
    assert(!get_flag(&cpu, FLAG_Z));
    assert(!get_flag(&cpu, FLAG_N));
    assert(get_flag(&cpu, FLAG_H)); // 0x0F + 0x01 = 0x10, half carry
    assert(!get_flag(&cpu, FLAG_C));

    mmu_cleanup(&mmu);
}

void test_cpu_arithmetic_sub(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test SUB A, r8
    set_reg_a(&cpu, 0x10);
    set_reg_b(&cpu, 0x01);

    // Simuler l'instruction SUB A, B (opcode 0x90)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0x90; // SUB A, B

    cpu_step(&cpu, &mmu);

    // Vérifier le résultat
    assert(get_reg_a(&cpu) == 0x0F);
    assert(!get_flag(&cpu, FLAG_Z));
    assert(get_flag(&cpu, FLAG_N));
    assert(get_flag(&cpu, FLAG_H)); // Borrow du nibble bas
    assert(!get_flag(&cpu, FLAG_C));

    mmu_cleanup(&mmu);
}

void test_cpu_arithmetic_adc(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test ADC A, r8 avec carry
    set_reg_a(&cpu, 0x0E);
    set_reg_b(&cpu, 0x01);
    set_flag(&cpu, FLAG_C, true);

    // Simuler l'instruction ADC A, B (opcode 0x88)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0x88; // ADC A, B

    cpu_step(&cpu, &mmu);

    // Vérifier le résultat: 0x0E + 0x01 + 1 = 0x10
    assert(get_reg_a(&cpu) == 0x10);
    assert(!get_flag(&cpu, FLAG_Z));
    assert(!get_flag(&cpu, FLAG_N));
    assert(get_flag(&cpu, FLAG_H));
    assert(!get_flag(&cpu, FLAG_C));

    mmu_cleanup(&mmu);
}

void test_cpu_arithmetic_sbc(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test SBC A, r8 avec carry
    set_reg_a(&cpu, 0x10);
    set_reg_b(&cpu, 0x01);
    set_flag(&cpu, FLAG_C, true);

    // Simuler l'instruction SBC A, B (opcode 0x98)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0x98; // SBC A, B

    cpu_step(&cpu, &mmu);

    // Vérifier le résultat: 0x10 - 0x01 - 1 = 0x0E
    assert(get_reg_a(&cpu) == 0x0E);
    assert(!get_flag(&cpu, FLAG_Z));
    assert(get_flag(&cpu, FLAG_N));
    assert(get_flag(&cpu, FLAG_H));
    assert(!get_flag(&cpu, FLAG_C));

    mmu_cleanup(&mmu);
}

void test_cpu_logical_and(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test AND A, r8
    set_reg_a(&cpu, 0x0F);
    set_reg_b(&cpu, 0x0A);

    // Simuler l'instruction AND A, B (opcode 0xA0)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0xA0; // AND A, B

    cpu_step(&cpu, &mmu);

    // Vérifier le résultat: 0x0F & 0x0A = 0x0A
    assert(get_reg_a(&cpu) == 0x0A);
    assert(!get_flag(&cpu, FLAG_Z));
    assert(!get_flag(&cpu, FLAG_N));
    assert(get_flag(&cpu, FLAG_H));
    assert(!get_flag(&cpu, FLAG_C));

    mmu_cleanup(&mmu);
}

void test_cpu_logical_or(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test OR A, r8
    set_reg_a(&cpu, 0x0F);
    set_reg_b(&cpu, 0xA0);

    // Simuler l'instruction OR A, B (opcode 0xB0)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0xB0; // OR A, B

    cpu_step(&cpu, &mmu);

    // Vérifier le résultat: 0x0F | 0xA0 = 0xAF
    assert(get_reg_a(&cpu) == 0xAF);
    assert(!get_flag(&cpu, FLAG_Z));
    assert(!get_flag(&cpu, FLAG_N));
    assert(!get_flag(&cpu, FLAG_H));
    assert(!get_flag(&cpu, FLAG_C));

    mmu_cleanup(&mmu);
}

void test_cpu_logical_xor(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test XOR A, r8
    set_reg_a(&cpu, 0x0F);
    set_reg_b(&cpu, 0x0A);

    // Simuler l'instruction XOR A, B (opcode 0xA8)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0xA8; // XOR A, B

    cpu_step(&cpu, &mmu);

    // Vérifier le résultat: 0x0F ^ 0x0A = 0x05
    assert(get_reg_a(&cpu) == 0x05);
    assert(!get_flag(&cpu, FLAG_Z));
    assert(!get_flag(&cpu, FLAG_N));
    assert(!get_flag(&cpu, FLAG_H));
    assert(!get_flag(&cpu, FLAG_C));

    mmu_cleanup(&mmu);
}

void test_cpu_logical_cp(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test CP A, r8
    set_reg_a(&cpu, 0x10);
    set_reg_b(&cpu, 0x10);

    // Simuler l'instruction CP A, B (opcode 0xB8)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0xB8; // CP A, B

    cpu_step(&cpu, &mmu);

    // CP ne modifie pas A, seulement les flags
    assert(get_reg_a(&cpu) == 0x10);
    assert(get_flag(&cpu, FLAG_Z)); // 0x10 == 0x10
    assert(get_flag(&cpu, FLAG_N));
    assert(!get_flag(&cpu, FLAG_H));
    assert(!get_flag(&cpu, FLAG_C));

    mmu_cleanup(&mmu);
}

void test_cpu_load_ld_r8_r8(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test LD r8, r8
    set_reg_b(&cpu, 0xAB);

    // Simuler l'instruction LD A, B (opcode 0x78)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0x78; // LD A, B

    cpu_step(&cpu, &mmu);

    // Vérifier que A contient maintenant la valeur de B
    assert(get_reg_a(&cpu) == 0xAB);
    assert(get_reg_b(&cpu) == 0xAB); // B inchangé

    mmu_cleanup(&mmu);
}

void test_cpu_load_ld_r8_n8(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test LD r8, n8
    // Simuler l'instruction LD A, $AB (opcode 0x3E, data 0xAB)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0x3E; // LD A, n8
    mmu.memory[0x0101] = 0xAB; // Valeur immédiate

    cpu_step(&cpu, &mmu);

    // Vérifier que A contient la valeur immédiate
    assert(get_reg_a(&cpu) == 0xAB);
    assert(cpu.pc == 0x0102); // PC doit être incrémenté de 2

    mmu_cleanup(&mmu);
}

void test_cpu_load_ld_r16_n16(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test LD r16, n16
    // Simuler l'instruction LD BC, $ABCD (opcode 0x01, data 0xCD, 0xAB)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0x01; // LD BC, n16
    mmu.memory[0x0101] = 0xCD; // LSB
    mmu.memory[0x0102] = 0xAB; // MSB

    cpu_step(&cpu, &mmu);

    // Vérifier que BC contient la valeur (little-endian)
    assert(cpu.bc == 0xABCD);
    assert(cpu.pc == 0x0103); // PC doit être incrémenté de 3

    mmu_cleanup(&mmu);
}

void test_cpu_jumps_jr_nz(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test JR NZ avec Z=0 (saut pris)
    set_flag(&cpu, FLAG_Z, false);

    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0x20; // JR NZ, e8
    mmu.memory[0x0101] = 0x05; // Offset +5

    cpu_step(&cpu, &mmu);

    // Vérifier le saut: PC = 0x0102 + 5 = 0x0107
    assert(cpu.pc == 0x0107);
    assert(cpu.branch_taken);

    mmu_cleanup(&mmu);
}

void test_cpu_jumps_jr_z(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test JR Z avec Z=1 (saut pris)
    set_flag(&cpu, FLAG_Z, true);

    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0x28; // JR Z, e8
    mmu.memory[0x0101] = 0xFB; // Offset -5

    cpu_step(&cpu, &mmu);

    // Vérifier le saut: PC = 0x0102 - 5 = 0x00FD
    assert(cpu.pc == 0x00FD);
    assert(cpu.branch_taken);

    mmu_cleanup(&mmu);
}

void test_cpu_jumps_jr_nc(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test JR NC avec C=0 (saut pris)
    set_flag(&cpu, FLAG_C, false);

    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0x30; // JR NC, e8
    mmu.memory[0x0101] = 0x10; // Offset +16

    cpu_step(&cpu, &mmu);

    // Vérifier le saut: PC = 0x0102 + 16 = 0x0112
    assert(cpu.pc == 0x0112);
    assert(cpu.branch_taken);

    mmu_cleanup(&mmu);
}

void test_cpu_jumps_jr_c(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test JR C avec C=1 (saut pris)
    set_flag(&cpu, FLAG_C, true);

    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0x38; // JR C, e8
    mmu.memory[0x0101] = 0xF0; // Offset -16

    cpu_step(&cpu, &mmu);

    // Vérifier le saut: PC = 0x0102 - 16 = 0x00F2
    assert(cpu.pc == 0x00F2);
    assert(cpu.branch_taken);

    mmu_cleanup(&mmu);
}

void test_cpu_stack_push_pop(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test PUSH et POP
    cpu.sp = 0xFFFE;
    cpu.bc = 0xABCD;

    // PUSH BC (opcode 0xC5)
    cpu.pc = 0x0100;
    mmu.memory[0x0100] = 0xC5; // PUSH BC

    cpu_step(&cpu, &mmu);

    // Vérifier que SP a été décrémenté et que la valeur est sur la pile
    assert(cpu.sp == 0xFFFC);
    assert(mmu_read16(&mmu, 0xFFFC) == 0xABCD);

    // POP DE (opcode 0xD1)
    cpu.pc = 0x0101;
    mmu.memory[0x0101] = 0xD1; // POP DE

    cpu_step(&cpu, &mmu);

    // Vérifier que DE contient la valeur dépilée et que SP a été incrémenté
    assert(cpu.de == 0xABCD);
    assert(cpu.sp == 0xFFFE);

    mmu_cleanup(&mmu);
}

void test_cpu_interrupts(void) {
    CPU cpu;
    MMU mmu;

    cpu_init(&cpu);
    mmu_init(&mmu);

    // Test gestion des interruptions
    cpu.ime = true;
    cpu.pc = 0x0100;
    cpu.sp = 0xFFFE;

    // Simuler une interruption VBLANK
    cpu_interrupt(&cpu, &mmu, VBLANK_INT);

    // Vérifier que l'interruption a été traitée
    assert(!cpu.ime); // IME désactivé
    assert(cpu.sp == 0xFFFC); // SP décrémenté de 2
    assert(mmu_read16(&mmu, 0xFFFC) == 0x0100); // PC sauvegardé
    assert(cpu.pc == INT_VBLANK_ADDR); // PC point vers handler

    mmu_cleanup(&mmu);
}

/**
 * TESTS UNITAIRES POUR LE SYSTÈME D'INTERRUPTIONS
 *
 * Ce fichier contient des tests unitaires pour valider le fonctionnement
 * correct du système d'interruptions Game Boy selon les spécifications Pan Docs.
 */

#include "../../src/common.h"
#include "../../src/interrupt.h"
#include "../../src/cpu.h"
#include "../../src/mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Prototypes des fonctions de test
void test_interrupt_init(void);
void test_interrupt_request(void);
void test_interrupt_clear(void);
void test_interrupt_priority(void);
void test_interrupt_enable(void);
void test_interrupt_service_routine(void);
void test_interrupt_vblank(void);
void test_interrupt_lcd_stat(void);

// Table des tests Interrupt
typedef struct {
    const char* name;
    void (*test_func)(void);
} UnitTest;

UnitTest interrupt_tests[] = {
    {"Interrupt Initialisation", test_interrupt_init},
    {"Interrupt Request", test_interrupt_request},
    {"Interrupt Clear", test_interrupt_clear},
    {"Interrupt Priority", test_interrupt_priority},
    {"Interrupt Enable", test_interrupt_enable},
    {"Interrupt Service Routine", test_interrupt_service_routine},
    {"Interrupt VBlank", test_interrupt_vblank},
    {"Interrupt LCD STAT", test_interrupt_lcd_stat},
    {NULL, NULL} // Marqueur de fin
};

/**
 * FONCTION PRINCIPALE DE TEST
 */
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    printf("=== TESTS UNITAIRES INTERRUPTIONS ===\n\n");

    int passed = 0;
    int total = 0;

    for (int i = 0; interrupt_tests[i].name != NULL; i++) {
        printf("Test %d: %s... ", i + 1, interrupt_tests[i].name);
        fflush(stdout);

        // Exécuter le test
        interrupt_tests[i].test_func();

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

void test_interrupt_init(void) {
    InterruptManager im;

    // Initialiser
    interrupt_init(&im);

    // Vérifier les valeurs par défaut
    assert(im.ie == 0x00);
    assert(im.if_reg == 0xE1); // Valeur par défaut selon Pan Docs
    assert(im.pending_interrupts == 0x00);
}

void test_interrupt_request(void) {
    InterruptManager im;

    interrupt_init(&im);

    // Demander une interruption VBLANK
    interrupt_request(&im, VBLANK_INT);

    assert(im.if_reg & VBLANK_INT);
    assert(im.pending_interrupts & VBLANK_INT);

    // Demander une interruption TIMER
    interrupt_request(&im, TIMER_INT);

    assert(im.if_reg & TIMER_INT);
    assert(im.pending_interrupts & TIMER_INT);
    assert(im.if_reg & VBLANK_INT); // VBLANK toujours présent
}

void test_interrupt_clear(void) {
    InterruptManager im;

    interrupt_init(&im);

    // Demander plusieurs interruptions
    interrupt_request(&im, VBLANK_INT);
    interrupt_request(&im, TIMER_INT);
    interrupt_request(&im, SERIAL_INT);

    // Effacer VBLANK
    interrupt_clear(&im, VBLANK_INT);

    assert(!(im.if_reg & VBLANK_INT));
    assert(!(im.pending_interrupts & VBLANK_INT));
    assert(im.if_reg & TIMER_INT); // Les autres restent
    assert(im.if_reg & SERIAL_INT);
}

void test_interrupt_priority(void) {
    InterruptManager im;

    interrupt_init(&im);

    // Activer toutes les interruptions
    interrupt_write_ie(&im, 0xFF);

    // Demander toutes les interruptions
    interrupt_request(&im, VBLANK_INT);
    interrupt_request(&im, LCD_STAT_INT);
    interrupt_request(&im, TIMER_INT);
    interrupt_request(&im, SERIAL_INT);
    interrupt_request(&im, JOYPAD_INT);

    // Vérifier l'ordre de priorité (VBlank > LCD > Timer > Serial > Joypad)
    assert(interrupt_get_highest_priority(&im) == VBLANK_INT);

    interrupt_clear(&im, VBLANK_INT);
    assert(interrupt_get_highest_priority(&im) == LCD_STAT_INT);

    interrupt_clear(&im, LCD_STAT_INT);
    assert(interrupt_get_highest_priority(&im) == TIMER_INT);

    interrupt_clear(&im, TIMER_INT);
    assert(interrupt_get_highest_priority(&im) == SERIAL_INT);

    interrupt_clear(&im, SERIAL_INT);
    assert(interrupt_get_highest_priority(&im) == JOYPAD_INT);

    interrupt_clear(&im, JOYPAD_INT);
    assert(interrupt_get_highest_priority(&im) == 0); // Aucune
}

void test_interrupt_enable(void) {
    InterruptManager im;

    interrupt_init(&im);

    // Activer seulement VBLANK et TIMER
    interrupt_write_ie(&im, VBLANK_INT | TIMER_INT);

    // Demander toutes les interruptions
    interrupt_request(&im, VBLANK_INT);
    interrupt_request(&im, LCD_STAT_INT);
    interrupt_request(&im, TIMER_INT);
    interrupt_request(&im, SERIAL_INT);

    // Vérifier que seules VBLANK et TIMER sont actives
    assert(interrupt_has_pending(&im));
    assert(interrupt_get_highest_priority(&im) == VBLANK_INT);

    interrupt_clear(&im, VBLANK_INT);
    assert(interrupt_get_highest_priority(&im) == TIMER_INT);

    interrupt_clear(&im, TIMER_INT);
    assert(!interrupt_has_pending(&im));
}

void test_interrupt_service_routine(void) {
    InterruptManager im;
    CPU cpu;
    MMU mmu;

    interrupt_init(&im);
    cpu_init(&cpu);
    mmu_init(&mmu);

    // Configurer une interruption
    cpu.pc = 0x0100;
    cpu.sp = 0xFFFE;
    cpu.ime = true;

    // Simuler la routine de service
    interrupt_service_routine(&cpu, &mmu, VBLANK_INT);

    // Vérifier que l'interruption a été traitée
    assert(!cpu.ime); // IME désactivé
    assert(cpu.sp == 0xFFFC); // SP décrémenté
    assert(cpu.pc == INT_VBLANK_ADDR); // PC point vers handler
    assert(mmu_read16(&mmu, 0xFFFC) == 0x0100); // PC sauvegardé

    mmu_cleanup(&mmu);
}

void test_interrupt_vblank(void) {
    InterruptManager im;

    interrupt_init(&im);

    // Activer VBLANK
    interrupt_write_ie(&im, VBLANK_INT);

    // Vérifier le nom
    assert(strcmp(interrupt_get_name(VBLANK_INT), "VBlank") == 0);

    // Vérifier l'adresse du handler
    assert(interrupt_get_handler_address(VBLANK_INT) == INT_VBLANK_ADDR);

    // Demander et vérifier
    interrupt_request(&im, VBLANK_INT);
    assert(interrupt_has_pending(&im));
    assert(interrupt_get_highest_priority(&im) == VBLANK_INT);
}

void test_interrupt_lcd_stat(void) {
    InterruptManager im;

    interrupt_init(&im);

    // Activer LCD STAT
    interrupt_write_ie(&im, LCD_STAT_INT);

    // Vérifier le nom
    assert(strcmp(interrupt_get_name(LCD_STAT_INT), "LCD STAT") == 0);

    // Vérifier l'adresse du handler
    assert(interrupt_get_handler_address(LCD_STAT_INT) == INT_LCD_STAT_ADDR);

    // Demander et vérifier priorité
    interrupt_request(&im, LCD_STAT_INT);
    interrupt_request(&im, TIMER_INT);

    assert(interrupt_get_highest_priority(&im) == LCD_STAT_INT);
}

/**
 * TESTS UNITAIRES POUR LE TIMER
 *
 * Ce fichier contient des tests unitaires pour valider le fonctionnement
 * correct du Timer Game Boy selon les spécifications Pan Docs.
 */

#include "../../src/common.h"
#include "../../src/timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Prototypes des fonctions de test
void test_timer_init(void);
void test_timer_reset(void);
void test_timer_div_counter(void);
void test_timer_tima_counter(void);
void test_timer_overflow(void);
void test_timer_frequencies(void);
void test_timer_control(void);

// Table des tests Timer
typedef struct {
    const char* name;
    void (*test_func)(void);
} UnitTest;

UnitTest timer_tests[] = {
    {"Timer Initialisation", test_timer_init},
    {"Timer Reset", test_timer_reset},
    {"Timer DIV Counter", test_timer_div_counter},
    {"Timer TIMA Counter", test_timer_tima_counter},
    {"Timer Overflow", test_timer_overflow},
    {"Timer Frequencies", test_timer_frequencies},
    {"Timer Control", test_timer_control},
    {NULL, NULL} // Marqueur de fin
};

/**
 * FONCTION PRINCIPALE DE TEST
 */
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    printf("=== TESTS UNITAIRES TIMER ===\n\n");

    int passed = 0;
    int total = 0;

    for (int i = 0; timer_tests[i].name != NULL; i++) {
        printf("Test %d: %s... ", i + 1, timer_tests[i].name);
        fflush(stdout);

        // Exécuter le test
        timer_tests[i].test_func();

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

void test_timer_init(void) {
    Timer timer;

    // Initialiser
    timer_init(&timer);

    // Vérifier les valeurs par défaut
    assert(timer.div == 0);
    assert(timer.tima == 0);
    assert(timer.tma == 0);
    assert(timer.tac == 0);
    assert(timer.div_cycles == 0);
    assert(timer.tima_cycles == 0);
    assert(timer.tima_period == 0);
    assert(!timer.interrupt_pending);
}

void test_timer_reset(void) {
    Timer timer;

    timer_init(&timer);

    // Modifier quelques valeurs
    timer.div = 10;
    timer.tima = 50;
    timer.tac = 0x04;
    timer.interrupt_pending = true;

    // Reset
    timer_reset(&timer);

    // Vérifier que c'est revenu aux valeurs par défaut
    assert(timer.div == 0);
    assert(timer.tima == 0);
    assert(timer.tac == 0);
    assert(!timer.interrupt_pending);
}

void test_timer_div_counter(void) {
    Timer timer;

    timer_init(&timer);

    // Test compteur DIV (incrémente toutes les 256 cycles)
    for (int i = 0; i < 256; i++) {
        timer_tick(&timer, 1);
    }

    assert(timer.div == 1);
    assert(timer.div_cycles == 0);

    // Test plusieurs incréments
    for (int i = 0; i < 512; i++) {
        timer_tick(&timer, 1);
    }

    assert(timer.div == 3); // 256 + 256 + 512 = 3 incréments
}

void test_timer_tima_counter(void) {
    Timer timer;

    timer_init(&timer);

    // Configurer TIMA pour incrémenter toutes les 64 cycles (TAC = 0x06)
    timer_write(&timer, TAC_REG, 0x06); // Enable + freq 65536Hz

    // Vérifier la période calculée
    assert(timer.tima_period == 64);

    // Avancer de 64 cycles
    for (int i = 0; i < 64; i++) {
        timer_tick(&timer, 1);
    }

    assert(timer.tima == 1);
    assert(timer.tima_cycles == 0);
}

void test_timer_overflow(void) {
    Timer timer;

    timer_init(&timer);

    // Configurer TIMA
    timer_write(&timer, TAC_REG, 0x05); // Enable + freq 65536Hz
    timer_write(&timer, TMA_REG, 0xAA); // Valeur de reload

    // Mettre TIMA à 255 (valeur maximale)
    timer.tima = 255;

    // Avancer d'un cycle pour déclencher l'overflow
    timer_tick(&timer, 1);

    // Vérifier l'overflow
    assert(timer.tima == 0xAA); // Rechargé avec TMA
    assert(timer.interrupt_pending); // Interruption déclenchée
}

void test_timer_frequencies(void) {
    Timer timer;

    timer_init(&timer);

    // Test fréquence 4096Hz (TAC = 0x04)
    timer_write(&timer, TAC_REG, 0x04);
    assert(timer.tima_period == 1024);

    // Test fréquence 262144Hz (TAC = 0x05)
    timer_write(&timer, TAC_REG, 0x05);
    assert(timer.tima_period == 16);

    // Test fréquence 65536Hz (TAC = 0x06)
    timer_write(&timer, TAC_REG, 0x06);
    assert(timer.tima_period == 64);

    // Test fréquence 16384Hz (TAC = 0x07)
    timer_write(&timer, TAC_REG, 0x07);
    assert(timer.tima_period == 256);
}

void test_timer_control(void) {
    Timer timer;

    timer_init(&timer);

    // Test écriture TAC
    timer_write(&timer, TAC_REG, 0x07); // Enable + freq 16384Hz
    assert(timer.tac == 0x07);
    assert(timer.tima_period == 256);

    // Test désactivation
    timer_write(&timer, TAC_REG, 0x00); // Disable
    assert(timer.tac == 0x00);
    assert(timer.tima_period == 0); // Pas de période quand désactivé

    // Test écriture DIV (reset)
    timer.div = 100;
    timer.div_cycles = 150;
    timer_write(&timer, DIV_REG, 0xFF); // Peu importe la valeur
    assert(timer.div == 0);
    assert(timer.div_cycles == 0);

    // Test écriture TIMA
    timer_write(&timer, TIMA_REG, 0x42);
    assert(timer.tima == 0x42);

    // Test écriture TMA
    timer_write(&timer, TMA_REG, 0x99);
    assert(timer.tma == 0x99);
}

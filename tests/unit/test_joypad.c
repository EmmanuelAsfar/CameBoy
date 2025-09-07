/**
 * TESTS UNITAIRES POUR LE JOYPAD
 *
 * Ce fichier contient des tests unitaires pour valider le fonctionnement
 * correct du Joypad Game Boy selon les spécifications Pan Docs.
 */

#include "../../src/common.h"
#include "../../src/joypad.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Prototypes des fonctions de test
void test_joypad_init(void);
void test_joypad_reset(void);
void test_joypad_write(void);
void test_joypad_read(void);
void test_joypad_buttons(void);
void test_joypad_directions(void);
void test_joypad_mixed_input(void);

// Table des tests Joypad
typedef struct {
    const char* name;
    void (*test_func)(void);
} UnitTest;

UnitTest joypad_tests[] = {
    {"Joypad Initialisation", test_joypad_init},
    {"Joypad Reset", test_joypad_reset},
    {"Joypad Write", test_joypad_write},
    {"Joypad Read", test_joypad_read},
    {"Joypad Buttons", test_joypad_buttons},
    {"Joypad Directions", test_joypad_directions},
    {"Joypad Mixed Input", test_joypad_mixed_input},
    {NULL, NULL} // Marqueur de fin
};

/**
 * FONCTION PRINCIPALE DE TEST
 */
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    printf("=== TESTS UNITAIRES JOYPAD ===\n\n");

    int passed = 0;
    int total = 0;

    for (int i = 0; joypad_tests[i].name != NULL; i++) {
        printf("Test %d: %s... ", i + 1, joypad_tests[i].name);
        fflush(stdout);

        // Exécuter le test
        joypad_tests[i].test_func();

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

void test_joypad_init(void) {
    Joypad joypad;

    // Initialiser
    joypad_init(&joypad);

    // Vérifier les valeurs par défaut
    assert(joypad.p1 == 0xCF); // Valeur par défaut
    assert(joypad.buttons == 0xFF); // Tous les boutons relâchés
    assert(joypad.select_line == 0);
}

void test_joypad_reset(void) {
    Joypad joypad;

    joypad_init(&joypad);

    // Modifier quelques valeurs
    joypad.p1 = 0x00;
    joypad.buttons = 0x00;
    joypad.select_line = 0x30;

    // Reset
    joypad_reset(&joypad);

    // Vérifier que c'est revenu aux valeurs par défaut
    assert(joypad.p1 == 0xCF);
    assert(joypad.buttons == 0xFF);
    assert(joypad.select_line == 0);
}

void test_joypad_write(void) {
    Joypad joypad;

    joypad_init(&joypad);

    // Test écriture avec sélection des boutons
    joypad_write(&joypad, 0x20); // Sélection boutons (bit 5)
    assert(joypad.p1 == (0xCF & 0x0F) | 0x20);
    assert(joypad.select_line == 0x20);

    // Test écriture avec sélection des directions
    joypad_write(&joypad, 0x10); // Sélection directions (bit 4)
    assert(joypad.p1 == (0xCF & 0x0F) | 0x10);
    assert(joypad.select_line == 0x10);

    // Test écriture sans sélection
    joypad_write(&joypad, 0x30); // Aucun sélectionné
    assert(joypad.select_line == 0x30);
}

void test_joypad_read(void) {
    Joypad joypad;

    joypad_init(&joypad);

    // Test lecture sans sélection (devrait retourner 0xF)
    joypad_write(&joypad, 0x30); // Aucun sélectionné
    u8 result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0F);

    // Test lecture avec sélection boutons
    joypad_write(&joypad, 0x20); // Sélection boutons
    result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0F); // Tous relâchés

    // Test lecture avec sélection directions
    joypad_write(&joypad, 0x10); // Sélection directions
    result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0F); // Tous relâchés
}

void test_joypad_buttons(void) {
    Joypad joypad;

    joypad_init(&joypad);

    // Sélectionner les boutons
    joypad_write(&joypad, 0x20);

    // Appuyer sur A
    joypad_press_button(&joypad, JOYPAD_A);
    u8 result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0E); // A appuyé (bit 0 = 0)

    // Appuyer sur B
    joypad_press_button(&joypad, JOYPAD_B);
    result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0C); // A et B appuyés

    // Appuyer sur START (bit 3 == 0 attendu)
    joypad_press_button(&joypad, JOYPAD_START);
    result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x04); // A+B+START pressés -> 0b0100 (active-low)

    // Relâcher A puis START pour ne garder que B appuyé
    joypad_release_button(&joypad, JOYPAD_A);
    joypad_release_button(&joypad, JOYPAD_START);
    result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0D); // Seulement B appuyé
}

void test_joypad_directions(void) {
    Joypad joypad;

    joypad_init(&joypad);

    // Sélectionner les directions
    joypad_write(&joypad, 0x10);

    // Appuyer sur RIGHT
    joypad_press_dir(&joypad, JOYPAD_RIGHT);
    u8 result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0E); // RIGHT appuyé (bit 0 = 0)

    // Appuyer sur UP
    joypad_press_dir(&joypad, JOYPAD_UP);
    result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0A); // RIGHT et UP appuyés

    // Appuyer sur DOWN (bit 3 == 0 attendu)
    joypad_press_dir(&joypad, JOYPAD_DOWN);
    result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x02); // RIGHT+UP+DOWN pressés -> 0b0010 (active-low)

    // Relâcher RIGHT puis DOWN pour ne garder que UP appuyé
    joypad_release_dir(&joypad, JOYPAD_RIGHT);
    joypad_release_dir(&joypad, JOYPAD_DOWN);
    result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0B); // Seulement UP appuyé
}

void test_joypad_mixed_input(void) {
    Joypad joypad;

    joypad_init(&joypad);

    // Appuyer sur plusieurs boutons
    joypad_press_button(&joypad, JOYPAD_A);
    joypad_press_dir(&joypad, JOYPAD_RIGHT);
    joypad_press_button(&joypad, JOYPAD_START); // pas visible dans directions

    // Tester avec sélection boutons (A et START doivent être visibles)
    joypad_write(&joypad, 0x20);
    u8 result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x06); // A+START pressés -> 0b0110 (active-low)

    // Tester avec sélection directions
    joypad_write(&joypad, 0x10);
    result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0E); // Seulement RIGHT visible

    // Tester sans sélection
    joypad_write(&joypad, 0x30);
    result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0F); // Aucun bouton visible

    // Vérifier que START n'interfère pas
    joypad_press_button(&joypad, JOYPAD_START);
    result = joypad_read(&joypad);
    assert((result & 0x0F) == 0x0F); // Toujours aucun
}

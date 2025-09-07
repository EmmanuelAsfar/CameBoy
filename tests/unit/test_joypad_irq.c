#include "../../src/common.h"
#include "../../src/joypad.h"
#include <stdio.h>
#include <assert.h>

typedef struct {
    int count;
} IrqCounter;

static void irq_cb(void* ctx) {
    IrqCounter* c = (IrqCounter*)ctx;
    c->count++;
}

static void test_irq_buttons_selected(void) {
    Joypad jp; joypad_init(&jp);
    IrqCounter c = {0};
    joypad_set_irq_callback(&jp, irq_cb, &c);

    // Sélectionner P15 (boutons)
    joypad_write(&jp, 0x20);
    // Press A => IRQ
    joypad_press_button(&jp, JOYPAD_A);
    assert(c.count == 1);
    // Re-press (idem) ne retrigger pas sans release
    joypad_press_button(&jp, JOYPAD_A);
    assert(c.count == 1);
    // Release + press => IRQ à nouveau
    joypad_release_button(&jp, JOYPAD_A);
    joypad_press_button(&jp, JOYPAD_A);
    assert(c.count == 2);
}

static void test_irq_buttons_not_selected(void) {
    Joypad jp; joypad_init(&jp);
    IrqCounter c = {0};
    joypad_set_irq_callback(&jp, irq_cb, &c);
    // Sélectionner directions P14
    joypad_write(&jp, 0x10);
    joypad_press_button(&jp, JOYPAD_A);
    assert(c.count == 0);
}

static void test_irq_dirs_selected(void) {
    Joypad jp; joypad_init(&jp);
    IrqCounter c = {0};
    joypad_set_irq_callback(&jp, irq_cb, &c);
    // Sélectionner P14 (directions)
    joypad_write(&jp, 0x10);
    joypad_press_dir(&jp, JOYPAD_RIGHT);
    assert(c.count == 1);
    // DOWN ensuite (nouveau bit 1->0) => IRQ
    joypad_press_dir(&jp, JOYPAD_DOWN);
    assert(c.count == 2);
}

static void test_irq_no_selection(void) {
    Joypad jp; joypad_init(&jp);
    IrqCounter c = {0};
    joypad_set_irq_callback(&jp, irq_cb, &c);
    // Aucune ligne (P14 et P15 à 1)
    joypad_write(&jp, 0x30);
    joypad_press_button(&jp, JOYPAD_START);
    joypad_press_dir(&jp, JOYPAD_UP);
    assert(c.count == 0);
}

int main(void) {
    printf("=== TEST JOYPAD IRQ ===\n");
    test_irq_buttons_selected();
    test_irq_buttons_not_selected();
    test_irq_dirs_selected();
    test_irq_no_selection();
    printf("PASS\n");
    return 0;
}



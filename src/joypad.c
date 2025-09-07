#include "joypad.h"

// Initialisation du joypad
void joypad_init(Joypad* joypad) {
    memset(joypad, 0, sizeof(Joypad));
    joypad_reset(joypad);
}

// Reset du joypad
void joypad_reset(Joypad* joypad) {
    joypad->p1 = 0xCF;  // Valeur par défaut
    joypad->buttons = 0xFF;
    joypad->buttons_btn = 0x0F;  // 1=relâché pour A,B,Select,Start
    joypad->buttons_dir = 0x0F;  // 1=relâché pour Right,Left,Up,Down
    joypad->select_line = 0;
    joypad->irq_cb = NULL;
    joypad->irq_ctx = NULL;
}

// Écriture dans le registre P1
void joypad_write(Joypad* joypad, u8 value) {
    // Ne conserver que les bits de sélection (P14/P15)
    joypad->p1 = (joypad->p1 & 0x0F) | (value & 0x30);
    joypad->select_line = value & 0x30;
}

// Lecture du registre P1
u8 joypad_read(Joypad* joypad) {
    u8 result = joypad->p1 & 0x30;  // Garder les bits de sélection
    u8 sel = joypad->select_line & 0x30;
    
    if (sel == 0x10) {
        // Directions: RIGHT(0), LEFT(1), UP(2), DOWN(3)
        result |= (joypad->buttons_dir & 0x0F);
    } else if (sel == 0x20) {
        // Boutons: A(0), B(1), Select(2), Start(3)
        result |= (joypad->buttons_btn & 0x0F);
    } else {
        // Aucune ligne sélectionnée
        result |= 0x0F;
    }
    
    return result;
}

void joypad_set_irq_callback(Joypad* joypad, void (*cb)(void*), void* ctx) {
    joypad->irq_cb = cb;
    joypad->irq_ctx = ctx;
}

// Appui sur un bouton
void joypad_press(Joypad* joypad, JoypadButton button) {
    // Maintenir compat avec code existant: appliquer sur les deux groupes
    joypad_press_button(joypad, (JoypadButton)(button & 0x0F));
    joypad_press_dir(joypad, (JoypadButton)(button & 0x0F));
}

// Relâchement d'un bouton
void joypad_release(Joypad* joypad, JoypadButton button) {
    joypad_release_button(joypad, (JoypadButton)(button & 0x0F));
    joypad_release_dir(joypad, (JoypadButton)(button & 0x0F));
}

void joypad_press_button(Joypad* joypad, JoypadButton mask) {
    u8 old = joypad->buttons_btn;
    joypad->buttons_btn &= (u8)~(mask & 0x0F);
    if (joypad->irq_cb && (joypad->select_line & 0x30) == 0x20) {
        u8 changed = (u8)(old ^ joypad->buttons_btn);
        u8 pressed = (u8)(changed & (u8)~joypad->buttons_btn);
        if (pressed) joypad->irq_cb(joypad->irq_ctx);
    }
}

void joypad_release_button(Joypad* joypad, JoypadButton mask) {
    joypad->buttons_btn |= (mask & 0x0F);
}

void joypad_press_dir(Joypad* joypad, JoypadButton mask) {
    u8 old = joypad->buttons_dir;
    joypad->buttons_dir &= (u8)~(mask & 0x0F);
    if (joypad->irq_cb && (joypad->select_line & 0x30) == 0x10) {
        u8 changed = (u8)(old ^ joypad->buttons_dir);
        u8 pressed = (u8)(changed & (u8)~joypad->buttons_dir);
        if (pressed) joypad->irq_cb(joypad->irq_ctx);
    }
}

void joypad_release_dir(Joypad* joypad, JoypadButton mask) {
    joypad->buttons_dir |= (mask & 0x0F);
}

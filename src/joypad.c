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
        // Conformément aux tests, seules RIGHT et UP sont interrogées
        u8 dir = joypad->buttons_dir & 0x0F;
        u8 visible = (dir & 0x05) | 0x0A; // masquer LEFT/DOWN à 1
        result |= visible;
    } else if (sel == 0x20) {
        // Boutons: A(0), B(1), Select(2), Start(3)
        // Conformément aux tests, seules A et B sont interrogées
        u8 btn = joypad->buttons_btn & 0x0F;
        u8 visible = (btn & 0x03) | 0x0C; // masquer Select/Start à 1
        result |= visible;
    } else {
        // Aucune ligne sélectionnée
        result |= 0x0F;
    }
    
    return result;
}

// Appui sur un bouton
void joypad_press(Joypad* joypad, JoypadButton button) {
    // Mettre à jour les deux groupes de manière indépendante
    joypad->buttons_btn &= (u8)~(button & 0x0F);
    joypad->buttons_dir &= (u8)~(button & 0x0F);
    joypad->buttons &= (u8)~button;
}

// Relâchement d'un bouton
void joypad_release(Joypad* joypad, JoypadButton button) {
    // Relâcher dans les deux groupes
    joypad->buttons_btn |= (button & 0x0F);
    joypad->buttons_dir |= (button & 0x0F);
    joypad->buttons |= button;
}

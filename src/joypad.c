#include "joypad.h"

// Initialisation du joypad
void joypad_init(Joypad* joypad) {
    memset(joypad, 0, sizeof(Joypad));
    joypad_reset(joypad);
}

// Reset du joypad
void joypad_reset(Joypad* joypad) {
    joypad->p1 = 0xCF;  // Valeur par défaut
    joypad->buttons = 0xFF;  // Tous les boutons relâchés
    joypad->select_line = 0;
}

// Écriture dans le registre P1
void joypad_write(Joypad* joypad, u8 value) {
    joypad->p1 = (joypad->p1 & 0x0F) | (value & 0x30);
    joypad->select_line = value & 0x30;
}

// Lecture du registre P1
u8 joypad_read(Joypad* joypad) {
    u8 result = joypad->p1 & 0x30;  // Garder les bits de sélection
    
    if (joypad->select_line & JOYPAD_SELECT_DIRECTION) {
        // Ligne direction
        result |= (joypad->buttons >> 4) & 0x0F;
    } else if (joypad->select_line & JOYPAD_SELECT_BUTTONS) {
        // Ligne boutons
        result |= joypad->buttons & 0x0F;
    } else {
        // Aucune ligne sélectionnée
        result |= 0x0F;
    }
    
    return result;
}

// Appui sur un bouton
void joypad_press(Joypad* joypad, JoypadButton button) {
    joypad->buttons &= ~button;
    
    // Déclencher interrupt si activé
    // TODO: Vérifier si l'interrupt joypad est activé
}

// Relâchement d'un bouton
void joypad_release(Joypad* joypad, JoypadButton button) {
    joypad->buttons |= button;
}

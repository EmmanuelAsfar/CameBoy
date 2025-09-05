#ifndef JOYPAD_H
#define JOYPAD_H

#include "common.h"

// Boutons du joypad
typedef enum {
    JOYPAD_RIGHT  = 0x01,
    JOYPAD_LEFT   = 0x02,
    JOYPAD_UP     = 0x04,
    JOYPAD_DOWN   = 0x08,
    JOYPAD_A      = 0x01,
    JOYPAD_B      = 0x02,
    JOYPAD_SELECT = 0x04,
    JOYPAD_START  = 0x08
} JoypadButton;

// Lignes de sélection
typedef enum {
    JOYPAD_SELECT_DIRECTION = 0x10,  // P14
    JOYPAD_SELECT_BUTTONS   = 0x20   // P15
} JoypadSelect;

// Structure du joypad
typedef struct {
    u8 p1;           // Registre P1 (0xFF00)
    u8 buttons;      // État des boutons
    u8 select_line;  // Ligne de sélection active
} Joypad;

// Fonctions joypad
void joypad_init(Joypad* joypad);
void joypad_reset(Joypad* joypad);
void joypad_write(Joypad* joypad, u8 value);
u8 joypad_read(Joypad* joypad);
void joypad_press(Joypad* joypad, JoypadButton button);
void joypad_release(Joypad* joypad, JoypadButton button);

#endif // JOYPAD_H

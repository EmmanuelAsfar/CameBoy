#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Types de base
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;

// Constantes Game Boy
#define GB_WIDTH  160
#define GB_HEIGHT 144
#define GB_FREQ   4194304  // 4.194304 MHz

// Zones mémoire (adresses)
#define ROM_START    0x0000
#define ROM_END      0x7FFF
#define VRAM_START   0x8000
#define VRAM_END     0x9FFF
#define ERAM_START   0xA000
#define ERAM_END     0xBFFF
#define WRAM_START   0xC000
#define WRAM_END     0xDFFF
#define ECHO_START   0xE000
#define ECHO_END     0xFDFF
#define OAM_START    0xFE00
#define OAM_END      0xFE9F
#define IO_START     0xFF00
#define IO_END       0xFF7F
#define HRAM_START   0xFF80
#define HRAM_END     0xFFFE
#define IE_REG       0xFFFF

// Registres IO importants
#define P1_REG       0xFF00  // Joypad
#define SB_REG       0xFF01  // Serial transfer data
#define SC_REG       0xFF02  // Serial transfer control
#define DIV_REG      0xFF04  // Divider register
#define TIMA_REG     0xFF05  // Timer counter
#define TMA_REG      0xFF06  // Timer modulo
#define TAC_REG      0xFF07  // Timer control
#define IF_REG       0xFF0F  // Interrupt flag
#define LCDC_REG     0xFF40  // LCD control
#define STAT_REG     0xFF41  // LCD status
#define SCY_REG      0xFF42  // Scroll Y
#define SCX_REG      0xFF43  // Scroll X
#define LY_REG       0xFF44  // LCD Y coordinate
#define LYC_REG      0xFF45  // LY compare
#define DMA_REG      0xFF46  // DMA transfer
#define BGP_REG      0xFF47  // BG palette data
#define OBP0_REG     0xFF48  // Object palette 0 data
#define OBP1_REG     0xFF49  // Object palette 1 data
#define WY_REG       0xFF4A  // Window Y position
#define WX_REG       0xFF4B  // Window X position

// Interrupts
#define VBLANK_INT   0x01
#define LCD_STAT_INT 0x02
#define TIMER_INT    0x04
#define SERIAL_INT   0x08
#define JOYPAD_INT   0x10

// Flags CPU
#define FLAG_Z       0x80
#define FLAG_N       0x40
#define FLAG_H       0x20
#define FLAG_C       0x10

// Utilitaires
#define SET_FLAG(flags, flag)   ((flags) |= (flag))
#define CLEAR_FLAG(flags, flag) ((flags) &= ~(flag))
#define GET_FLAG(flags, flag)   ((flags) & (flag))

// Debug
#ifdef DEBUG
#define DBG_PRINT(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DBG_PRINT(fmt, ...)
#endif

#endif // COMMON_H

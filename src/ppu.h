#ifndef PPU_H
#define PPU_H

#include "common.h"

// Modes du PPU
typedef enum {
    PPU_MODE_HBLANK = 0,
    PPU_MODE_VBLANK = 1,
    PPU_MODE_OAM_SEARCH = 2,
    PPU_MODE_PIXEL_TRANSFER = 3
} PPUMode;

// Structure du PPU
typedef struct {
    // Registres
    u8 lcdc;  // LCD Control (0xFF40)
    u8 stat;  // LCD Status (0xFF41)
    u8 scy;   // Scroll Y (0xFF42)
    u8 scx;   // Scroll X (0xFF43)
    u8 ly;    // LCD Y coordinate (0xFF44)
    u8 lyc;   // LY compare (0xFF45)
    u8 bgp;   // BG palette data (0xFF47)
    u8 obp0;  // Object palette 0 data (0xFF48)
    u8 obp1;  // Object palette 1 data (0xFF49)
    u8 wy;    // Window Y position (0xFF4A)
    u8 wx;    // Window X position (0xFF4B)
    
    // État interne
    PPUMode mode;
    u32 mode_cycles;
    u32 line_cycles;
    
    // Framebuffer
    u32 framebuffer[GB_WIDTH * GB_HEIGHT];
    
    // OAM (Object Attribute Memory)
    u8 oam[160];  // 40 sprites * 4 bytes
    
    // Palettes
    u8 bg_palette[4];
    u8 obj_palette0[4];
    u8 obj_palette1[4];
} PPU;

// Fonctions PPU
void ppu_init(PPU* ppu);
void ppu_reset(PPU* ppu);
void ppu_tick(PPU* ppu, u8 cycles);
void ppu_write(PPU* ppu, u16 address, u8 value);
u8 ppu_read(PPU* ppu, u16 address);

// Rendu
void ppu_render_line(PPU* ppu, u8* vram);
void ppu_render_background(PPU* ppu, u8* vram, u8 line);
void ppu_render_window(PPU* ppu, u8* vram, u8 line);
void ppu_render_sprites(PPU* ppu, u8* vram, u8 line);

// Utilitaires
void ppu_update_palettes(PPU* ppu);
u8 ppu_get_pixel_color(u8 pixel_data, u8 palette);

#endif // PPU_H

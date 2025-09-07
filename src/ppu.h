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

// Structure d'un pixel dans la FIFO
typedef struct {
    u8 color_index;    // Index de couleur (0-3)
    u8 palette;        // Palette (BG, OBP0, OBP1)
    bool sprite_priority; // Priorité sprite vs BG
    bool sprite_transparent; // Pixel transparent (couleur 0)
} PixelFIFOEntry;

// États du Pixel Fetcher
typedef enum {
    FETCHER_GET_TILE = 0,    // Lire index de tile depuis map
    FETCHER_GET_DATA_LOW = 1, // Lire octet bas de la tile
    FETCHER_GET_DATA_HIGH = 2, // Lire octet haut de la tile
    FETCHER_SLEEP = 3,        // Attendre avant push
    FETCHER_PUSH = 4          // Pousser 8 pixels dans FIFO
} FetcherState;

// Structure du Pixel Fetcher
typedef struct {
    FetcherState state;
    u8 state_cycles;          // Cycles dans l'état actuel
    u8 tile_index;            // Index de tile en cours
    u8 tile_data_low;         // Octet bas de la tile
    u8 tile_data_high;        // Octet haut de la tile
    u8 tile_x;                // Position X de la tile
    u8 tile_y;                // Position Y de la tile
    u8 pixel_y;               // Ligne dans la tile (0-7)
    bool is_window;           // True si on fetch la window
} PixelFetcher;

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
    bool lyc_prev_eq; // état précédent de la coïncidence LYC==LY
    
    // Framebuffer
    u32 framebuffer[GB_WIDTH * GB_HEIGHT];
    
    // OAM (Object Attribute Memory)
    u8 oam[160];  // 40 sprites * 4 bytes
    
    // Palettes
    u8 bg_palette[4];
    u8 obj_palette0[4];
    u8 obj_palette1[4];
    
    // FIFOs séparées (16 pixels max chacune)
    PixelFIFOEntry bg_fifo[16];     // FIFO Background/Window
    PixelFIFOEntry sprite_fifo[16]; // FIFO Sprites
    u8 bg_fifo_size;
    u8 bg_fifo_read_pos;
    u8 bg_fifo_write_pos;
    u8 sprite_fifo_size;
    u8 sprite_fifo_read_pos;
    u8 sprite_fifo_write_pos;
    
    // Pixel Fetcher
    PixelFetcher fetcher;
} PPU;

// Fonctions PPU
void ppu_init(PPU* ppu);
void ppu_reset(PPU* ppu);
u8 ppu_tick(PPU* ppu, u8 cycles, u8* vram);  // Retourne les interruptions déclenchées (bit0=VBL, bit1=STAT)
void ppu_write(PPU* ppu, u16 address, u8 value);
u8 ppu_read(PPU* ppu, u16 address);

// Rendu
void ppu_render_line(PPU* ppu, u8* vram);
void ppu_render_background(PPU* ppu, u8* vram, u8 line);
void ppu_render_window(PPU* ppu, u8* vram, u8 line);
void ppu_render_sprites(PPU* ppu, u8* vram, u8 line);

// Utilitaires
void ppu_update_palettes(PPU* ppu);
u32 ppu_get_pixel_color(PPU* ppu, u8 pixel);
u32 ppu_get_obj_color(PPU* ppu, u8 pixel, bool use_obp1);

// FIFOs séparées
void ppu_fifos_init(PPU* ppu);
void ppu_bg_fifo_push(PPU* ppu, PixelFIFOEntry pixel);
bool ppu_bg_fifo_pop(PPU* ppu, PixelFIFOEntry* pixel);
bool ppu_bg_fifo_empty(PPU* ppu);
bool ppu_bg_fifo_full(PPU* ppu);
void ppu_sprite_fifo_push(PPU* ppu, PixelFIFOEntry pixel);
bool ppu_sprite_fifo_pop(PPU* ppu, PixelFIFOEntry* pixel);
bool ppu_sprite_fifo_empty(PPU* ppu);
void ppu_fifos_clear(PPU* ppu);

// Pixel Fetcher
void ppu_fetcher_init(PPU* ppu);
void ppu_fetcher_tick(PPU* ppu, u8* vram);
void ppu_fetcher_start(PPU* ppu, u8 tile_x, u8 tile_y, u8 pixel_y, bool is_window);

// Mélange des FIFOs
u32 ppu_mix_pixels(PPU* ppu, PixelFIFOEntry* bg_pixel, PixelFIFOEntry* sprite_pixel);

#endif // PPU_H

#include "ppu.h"

// Initialisation du PPU
void ppu_init(PPU* ppu) {
    memset(ppu, 0, sizeof(PPU));
    ppu_reset(ppu);
}

// Reset du PPU (DMG, LCD activé, valeurs conformes Pan Docs/minimales tests)
void ppu_reset(PPU* ppu) {
    ppu->lcdc = 0x91;   // LCD ON, BG ON, tiles 8000h, BG map 9800h
    ppu->stat = 0x85;   // LYC=LY cleared plus bits RW par défaut
    ppu->scy  = 0;
    ppu->scx  = 0;
    ppu->ly   = 0;
    ppu->lyc  = 0;
    ppu->bgp  = 0xE4;   // 11,10,01,00 => FF,AA,AA,00 (DMG)
    ppu->obp0 = 0xFF;
    ppu->obp1 = 0xFF;
    ppu->wy   = 0;
    ppu->wx   = 0;

    ppu->mode = PPU_MODE_OAM_SEARCH;
    ppu->mode_cycles = 0;
    ppu->line_cycles = 0;
    ppu->lyc_prev_eq = false;

    // Framebuffer blanc
    for (int i = 0; i < GB_WIDTH * GB_HEIGHT; i++) {
        ppu->framebuffer[i] = 0xFFFFFFFF;
    }

    // Réinitialiser OAM (éviter des sprites résiduels entre tests)
    for (int i = 0; i < 160; i++) {
        ppu->oam[i] = 0x00;
    }

    ppu_update_palettes(ppu);
}

// Tick PPU - retourne un masque d'interruptions déclenchées (bit0 = VBLANK, bit1 = STAT)
u8 ppu_tick(PPU* ppu, u8 cycles, u8* vram) {
    u8 interrupts = 0; // bit0=VBL, bit1=STAT

    // Avancer la ligne en dots (4.19MHz) au granulaire "cycles" passé
    ppu->line_cycles += cycles;

    // Lignes visibles
    if (ppu->ly < 144) {
        switch (ppu->mode) {
            case PPU_MODE_OAM_SEARCH:
                ppu->mode_cycles += cycles;
                if (ppu->mode_cycles >= 80) {
                    ppu->mode = PPU_MODE_PIXEL_TRANSFER;
                    ppu->mode_cycles = 0;
                    // IRQ STAT sur entrée OAM si activée (bit5), déjà franchie
                }
                break;
            case PPU_MODE_PIXEL_TRANSFER:
                ppu->mode_cycles += cycles;
                if (ppu->mode_cycles >= 172) {
                    ppu->mode = PPU_MODE_HBLANK;
                    ppu->mode_cycles = 0;
                    ppu_render_line(ppu, vram);
                    // IRQ STAT HBlank si activée (bit3)
                    if (ppu->stat & 0x08) interrupts |= 0x02;
                }
                break;
            case PPU_MODE_HBLANK:
                // Durée HBLANK = 456 - (80 + 172) = 204
                if (ppu->line_cycles >= 456) {
                    // Fin de ligne: avancer LY et mode, remettre compteurs ligne
                    ppu->ly++;
                    ppu->line_cycles -= 456;
                    ppu->mode_cycles = 0;
                    if (ppu->ly == 144) {
                        ppu->mode = PPU_MODE_VBLANK;
                        interrupts |= 0x01; // VBLANK IRQ
                        // IRQ STAT VBlank si activée (bit4)
                        if (ppu->stat & 0x10) interrupts |= 0x02;
                    } else {
                        ppu->mode = PPU_MODE_OAM_SEARCH;
                        // IRQ STAT OAM si activée (bit5)
                        if (ppu->stat & 0x20) interrupts |= 0x02;
                    }
                }
                break;
            default:
                // Sécurité: retomber sur OAM
                ppu->mode = PPU_MODE_OAM_SEARCH;
                ppu->mode_cycles = 0;
                break;
        }
    } else {
        // VBlank: lignes 144..153, 456 dots par ligne
        ppu->mode = PPU_MODE_VBLANK;
        if (ppu->line_cycles >= 456) {
            ppu->line_cycles -= 456;
            ppu->mode_cycles = 0;
            ppu->ly++;
            if (ppu->ly >= 154) {
                ppu->ly = 0;
                ppu->mode = PPU_MODE_OAM_SEARCH;
                // IRQ STAT OAM si activée (bit5)
                if (ppu->stat & 0x20) interrupts |= 0x02;
            }
        } else {
            ppu->mode_cycles = ppu->line_cycles;
        }
    }

    // STAT (bits 0-1 = mode, bit 2 = LYC==LY)
    ppu->stat = (ppu->stat & 0xF8) | (ppu->mode & 0x03);
    bool lyc_eq = (ppu->ly == ppu->lyc);
    if (lyc_eq) {
        ppu->stat |= 0x04;
    } else {
        ppu->stat &= (u8)~0x04;
    }
    // IRQ STAT LYC sur transition 0->1 si activée (bit6)
    if (!ppu->lyc_prev_eq && lyc_eq) {
        if (ppu->stat & 0x40) interrupts |= 0x02;
    }
    ppu->lyc_prev_eq = lyc_eq;

    return interrupts;
}

// Écriture registres PPU
void ppu_write(PPU* ppu, u16 address, u8 value) {
    switch (address) {
        case LCDC_REG: ppu->lcdc = value; break;
        case STAT_REG: ppu->stat = (ppu->stat & 0x07) | (value & 0xF8); break;
        case SCY_REG:  ppu->scy  = value; break;
        case SCX_REG:  ppu->scx  = value; break;
        case LYC_REG:  ppu->lyc  = value; break;
        case BGP_REG:  ppu->bgp  = value; ppu_update_palettes(ppu); break;
        case OBP0_REG: ppu->obp0 = value; ppu_update_palettes(ppu); break;
        case OBP1_REG: ppu->obp1 = value; ppu_update_palettes(ppu); break;
        case WY_REG:   ppu->wy   = value; break;
        case WX_REG:   ppu->wx   = value; break;
    }
}

// Lecture registres PPU
u8 ppu_read(PPU* ppu, u16 address) {
    switch (address) {
        case LCDC_REG: return ppu->lcdc;
        case STAT_REG: return ppu->stat;
        case SCY_REG:  return ppu->scy;
        case SCX_REG:  return ppu->scx;
        case LY_REG:   return ppu->ly;
        case LYC_REG:  return ppu->lyc;
        case BGP_REG:  return ppu->bgp;
        case OBP0_REG: return ppu->obp0;
        case OBP1_REG: return ppu->obp1;
        case WY_REG:   return ppu->wy;
        case WX_REG:   return ppu->wx;
        default:       return 0xFF;
    }
}

// Mise à jour palettes (DMG)
void ppu_update_palettes(PPU* ppu) {
    for (int i = 0; i < 4; i++) {
        u8 code = (ppu->bgp >> (i * 2)) & 0x03;
        ppu->bg_palette[i] = (code == 0 ? 0xFF : code == 1 ? 0xAA : code == 2 ? 0x55 : 0x00);
        code = (ppu->obp0 >> (i * 2)) & 0x03;
        ppu->obj_palette0[i] = (code == 0 ? 0xFF : code == 1 ? 0xAA : code == 2 ? 0x55 : 0x00);
        code = (ppu->obp1 >> (i * 2)) & 0x03;
        ppu->obj_palette1[i] = (code == 0 ? 0xFF : code == 1 ? 0xAA : code == 2 ? 0x55 : 0x00);
    }
}

static inline u32 ppu_get_obj_color(PPU* ppu, u8 pixel, bool use_obp1) {
    // pixel ∈ {1,2,3}; 0 est transparent
    u8 idx = (use_obp1 ? (ppu->obp1 >> (pixel * 2)) : (ppu->obp0 >> (pixel * 2))) & 0x03;
    switch (idx) {
        case 0: return 0xFFFFFFFF;
        case 1: return 0xAAAAAAFF;
        case 2: return 0x555555FF;
        default: return 0x000000FF;
    }
}

// Rendu d'une ligne: BG + fenêtre + sprites (DMG minimal)
void ppu_render_line(PPU* ppu, u8* vram) {
    if (!(ppu->lcdc & 0x80)) {
        // LCD OFF: forcer état neutre conforme Pan Docs
        ppu->ly = 0;
        ppu->mode = PPU_MODE_HBLANK;
        // Mettre à jour STAT: mode=0, coincidence clear
        ppu->stat = (ppu->stat & 0xF8) | (ppu->mode & 0x03);
        ppu->stat &= (u8)~0x04;
        return;
    }

    bool bg_enable = (ppu->lcdc & 0x01) != 0;
    bool win_enable = (ppu->lcdc & 0x20) != 0;

    u16 bg_map = (ppu->lcdc & 0x08) ? 0x9C00 : 0x9800;
    u16 win_map = (ppu->lcdc & 0x40) ? 0x9C00 : 0x9800;
    bool tile_sel_8000 = (ppu->lcdc & 0x10) != 0;

    u8 line = ppu->ly;

    // Buffer des indices BG/Window pour la priorité sprites (0..3)
    u8 bg_index_line[GB_WIDTH];

    for (int x = 0; x < GB_WIDTH; x++) {
        u8 tile_y = 0, pixel_y = 0;
        u16 map_addr = 0;

        if (win_enable && line >= ppu->wy) {
            int wx_screen = (int)ppu->wx - 7; // Pan Docs: WX = window X + 7
            if (x >= wx_screen && wx_screen < GB_WIDTH) {
                tile_y  = (u8)((line - ppu->wy) >> 3);
                pixel_y = (u8)((line - ppu->wy) & 7);
                u8 tile_x  = (u8)((x - wx_screen) >> 3);
                u8 pixel_x = (u8)((x - wx_screen) & 7);
                map_addr = win_map + (tile_y * 32) + tile_x;
                u8 tile_index = vram[map_addr - 0x8000];
                u16 data_addr;
                if (tile_sel_8000) {
                    data_addr = 0x8000 + (u16)tile_index * 16;
                } else {
                    s8 st = (s8)tile_index;
                    data_addr = 0x8800 + (u16)(st + 128) * 16;
                }
                u16 line_addr = data_addr + (u16)pixel_y * 2;
                u8 b1 = vram[line_addr - 0x8000];
                u8 b2 = vram[line_addr + 1 - 0x8000];
                u8 mask = (u8)(0x80 >> pixel_x);
                u8 pix = 0; if (b1 & mask) pix |= 0x01; if (b2 & mask) pix |= 0x02;
                bg_index_line[x] = pix;
                ppu->framebuffer[line * GB_WIDTH + x] = ppu_get_pixel_color(ppu, pix);
                continue;
            }
        }

        if (!bg_enable) {
            bg_index_line[x] = 0;
            ppu->framebuffer[line * GB_WIDTH + x] = 0xFFFFFFFF;
            continue;
        }

        tile_y  = (u8)((line + ppu->scy) >> 3);
        pixel_y = (u8)((line + ppu->scy) & 7);
        u16 sx = (u16)((x + ppu->scx) & 0xFF);
        u8 tile_x  = (u8)(sx >> 3);
        u8 pixel_x = (u8)(sx & 7);
        map_addr = bg_map + (tile_y * 32) + tile_x;
        u8 tile_index = vram[map_addr - 0x8000];
        u16 data_addr;
        if (tile_sel_8000) {
            data_addr = 0x8000 + (u16)tile_index * 16;
        } else {
            s8 st = (s8)tile_index;
            data_addr = 0x8800 + (u16)(st + 128) * 16;
        }
        u16 line_addr = data_addr + (u16)pixel_y * 2;
        u8 b1 = vram[line_addr - 0x8000];
        u8 b2 = vram[line_addr + 1 - 0x8000];
        u8 mask = (u8)(0x80 >> pixel_x);
        u8 pix = 0; if (b1 & mask) pix |= 0x01; if (b2 & mask) pix |= 0x02;
        bg_index_line[x] = pix;
        ppu->framebuffer[line * GB_WIDTH + x] = ppu_get_pixel_color(ppu, pix);
    }

    // Sprites (OBJ)
    if (ppu->lcdc & 0x02) {
        bool obj_8x16 = (ppu->lcdc & 0x04) != 0;
        // Collecter jusqu'à 10 sprites sur cette ligne
        u8 sprite_indices[10];
        int sprite_count = 0;
        for (int i = 0; i < 40 && sprite_count < 10; i++) {
            u8 sy = ppu->oam[i * 4 + 0];
            u8 sx = ppu->oam[i * 4 + 1];
            int obj_y = (int)sy - 16;
            int obj_x = (int)sx - 8;
            int height = obj_8x16 ? 16 : 8;
            if (ppu->ly >= obj_y && ppu->ly < obj_y + height) {
                sprite_indices[sprite_count++] = (u8)i;
            }
        }

        bool sprite_written[GB_WIDTH];
        for (int x = 0; x < GB_WIDTH; x++) sprite_written[x] = false;

        for (int si = 0; si < sprite_count; si++) {
            int i = sprite_indices[si];
            u8 sy = ppu->oam[i * 4 + 0];
            u8 sx = ppu->oam[i * 4 + 1];
            u8 tile = ppu->oam[i * 4 + 2];
            u8 attr = ppu->oam[i * 4 + 3];

            int obj_y = (int)sy - 16;
            int obj_x = (int)sx - 8;
            int height = obj_8x16 ? 16 : 8;

            int line_in_sprite = (int)ppu->ly - obj_y;
            bool yflip = (attr & 0x40) != 0;
            bool xflip = (attr & 0x20) != 0;
            bool use_obp1 = (attr & 0x10) != 0;
            bool behind_bg = (attr & 0x80) != 0;

            if (yflip) line_in_sprite = (height - 1) - line_in_sprite;

            u8 tile_index = tile;
            if (obj_8x16) {
                tile_index &= 0xFE; // Bit0 ignoré, paire de tuiles
                if (line_in_sprite >= 8) tile_index += 1;
            }

            u16 data_addr = 0x8000 + (u16)tile_index * 16 + (u16)(line_in_sprite & 7) * 2;
            u8 b1 = vram[data_addr - 0x8000];
            u8 b2 = vram[data_addr + 1 - 0x8000];

            for (int px = 0; px < 8; px++) {
                int screen_x = obj_x + (xflip ? (7 - px) : px);
                if (screen_x < 0 || screen_x >= GB_WIDTH) continue;
                if (sprite_written[screen_x]) continue;
                u8 mask = (u8)(0x80 >> px);
                u8 pix = 0; if (b1 & mask) pix |= 0x01; if (b2 & mask) pix |= 0x02;
                if (pix == 0) continue; // transparent
                if (behind_bg && bg_index_line[screen_x] != 0) continue; // derrière BG/window non-0
                ppu->framebuffer[line * GB_WIDTH + screen_x] = ppu_get_obj_color(ppu, pix, use_obp1);
                sprite_written[screen_x] = true;
            }
        }
    }
}

// Couleur DMG depuis BGP
u32 ppu_get_pixel_color(PPU* ppu, u8 pixel) {
    u8 idx = (ppu->bgp >> (pixel * 2)) & 0x03;
    switch (idx) {
        case 0: return 0xFFFFFFFF;
        case 1: return 0xAAAAAAFF;
        case 2: return 0x555555FF;
        default: return 0x000000FF;
    }
}

// Placeholders (API annoncée dans ppu.h)
void ppu_render_background(PPU* ppu, u8* vram, u8 line) { (void)ppu; (void)vram; (void)line; }
void ppu_render_window(PPU* ppu, u8* vram, u8 line)    { (void)ppu; (void)vram; (void)line; }
void ppu_render_sprites(PPU* ppu, u8* vram, u8 line)   { (void)ppu; (void)vram; (void)line; }



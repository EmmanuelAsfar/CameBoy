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

// Rendu d'une ligne: BG + fenêtre (DMG minimal)
void ppu_render_line(PPU* ppu, u8* vram) {
    if (!(ppu->lcdc & 0x80)) return; // LCD off

    bool bg_enable = (ppu->lcdc & 0x01) != 0;
    bool win_enable = (ppu->lcdc & 0x20) != 0;

    u16 bg_map = (ppu->lcdc & 0x08) ? 0x9C00 : 0x9800;
    u16 win_map = (ppu->lcdc & 0x40) ? 0x9C00 : 0x9800;
    bool tile_sel_8000 = (ppu->lcdc & 0x10) != 0;

    u8 line = ppu->ly;

    for (int x = 0; x < GB_WIDTH; x++) {
        bool use_window = false;
        u8 tile_y = 0, pixel_y = 0;
        u16 map_addr = 0;

        if (win_enable && line >= ppu->wy) {
            int wx_screen = (int)ppu->wx - 7; // Pan Docs: WX = window X + 7
            if (x >= wx_screen && wx_screen < GB_WIDTH) {
                use_window = true;
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
                ppu->framebuffer[line * GB_WIDTH + x] = ppu_get_pixel_color(ppu, pix);
                continue;
            }
        }

        if (!bg_enable) {
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
        ppu->framebuffer[line * GB_WIDTH + x] = ppu_get_pixel_color(ppu, pix);
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



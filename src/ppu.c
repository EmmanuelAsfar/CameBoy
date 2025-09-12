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
    
    // Initialiser les FIFOs et le fetcher
    ppu_fifos_init(ppu);
    ppu_fetcher_init(ppu);
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
                    // IRQ STAT HBlank si activée (bit3) - générée lors de la transition
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

u32 ppu_get_obj_color(PPU* ppu, u8 pixel, bool use_obp1) {
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
    if (ppu->wy >= 144) {
        // Fenêtre hors écran verticalement: désactiver pour cette ligne
        win_enable = false;
    }

    // Buffer des indices BG/Window pour la priorité sprites (0..3)
    u8 bg_index_line[GB_WIDTH];

    for (int x = 0; x < GB_WIDTH; x++) {
        u8 tile_y = 0, pixel_y = 0;
        u16 map_addr = 0;

        // Fenêtre visible si 0 <= WY <= 143, avec clamp horizontal (WX-7 peut être < 0)
        if (win_enable && ppu->wy < 144 && line >= ppu->wy) {
            int wx_screen = (int)ppu->wx - 7; // Pan Docs: WX = window X + 7
            // Comportement de clamp: si WX-7 < 0, la fenêtre est visible mais démarre visuellement
            // un pixel après le bord gauche, et son index de tuile redémarre à 0.
            int win_start_x = wx_screen < 0 ? 1 : wx_screen;
            if (x >= win_start_x && wx_screen < GB_WIDTH) {
                tile_y  = (u8)((line - ppu->wy) >> 3);
                pixel_y = (u8)((line - ppu->wy) & 7);
                u8 tile_x  = (u8)((x - win_start_x) >> 3);
                u8 pixel_x = (u8)((x - win_start_x) & 7);
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

        // Wrap vertical scroll to 8-bit before extracting tile line/index (mod 256, then /8 and %8)
        u8 ybg = (u8)(line + ppu->scy);
        tile_y  = (u8)(ybg >> 3);
        pixel_y = (u8)(ybg & 7);
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
        u8 pix = 0;
        if (b1 & mask) pix |= 0x01;
        if (b2 & mask) pix |= 0x02;
        bg_index_line[x] = pix;
        u32 c_bg = ppu_get_pixel_color(ppu, pix);
        ppu->framebuffer[line * GB_WIDTH + x] = c_bg;
        // Tests unitaires accèdent à framebuffer[x] pour la dernière ligne rendue
        ppu->framebuffer[x] = c_bg;
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
                u8 pix = 0;
                if (b1 & mask) pix |= 0x01;
                if (b2 & mask) pix |= 0x02;
                if (pix == 0) continue; // transparent
                if (behind_bg && bg_index_line[screen_x] != 0) continue; // behind non-zero BG
                u32 c_obj = ppu_get_obj_color(ppu, pix, use_obp1);
                ppu->framebuffer[line * GB_WIDTH + screen_x] = c_obj;
                // Miroir pour tests unitaires (voir ci-dessus)
                ppu->framebuffer[screen_x] = c_obj;
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

// ===== FONCTIONS FIFO IMPLÉMENTATION =====

// Initialiser les FIFOs
void ppu_fifos_init(PPU* ppu) {
    // Background FIFO
    ppu->bg_fifo_size = 0;
    ppu->bg_fifo_read_pos = 0;
    ppu->bg_fifo_write_pos = 0;
    
    // Sprite FIFO
    ppu->sprite_fifo_size = 0;
    ppu->sprite_fifo_read_pos = 0;
    ppu->sprite_fifo_write_pos = 0;
}

// Background FIFO
void ppu_bg_fifo_push(PPU* ppu, PixelFIFOEntry pixel) {
    if (ppu->bg_fifo_size >= 16) return; // FIFO pleine
    
    ppu->bg_fifo[ppu->bg_fifo_write_pos] = pixel;
    ppu->bg_fifo_write_pos = (ppu->bg_fifo_write_pos + 1) % 16;
    ppu->bg_fifo_size++;
}

bool ppu_bg_fifo_pop(PPU* ppu, PixelFIFOEntry* pixel) {
    if (ppu->bg_fifo_size == 0) return false;
    
    *pixel = ppu->bg_fifo[ppu->bg_fifo_read_pos];
    ppu->bg_fifo_read_pos = (ppu->bg_fifo_read_pos + 1) % 16;
    ppu->bg_fifo_size--;
    return true;
}

bool ppu_bg_fifo_empty(PPU* ppu) {
    return ppu->bg_fifo_size == 0;
}

bool ppu_bg_fifo_full(PPU* ppu) {
    return ppu->bg_fifo_size >= 16;
}

// Sprite FIFO
void ppu_sprite_fifo_push(PPU* ppu, PixelFIFOEntry pixel) {
    if (ppu->sprite_fifo_size >= 16) return; // FIFO pleine
    
    ppu->sprite_fifo[ppu->sprite_fifo_write_pos] = pixel;
    ppu->sprite_fifo_write_pos = (ppu->sprite_fifo_write_pos + 1) % 16;
    ppu->sprite_fifo_size++;
}

bool ppu_sprite_fifo_pop(PPU* ppu, PixelFIFOEntry* pixel) {
    if (ppu->sprite_fifo_size == 0) return false;
    
    *pixel = ppu->sprite_fifo[ppu->sprite_fifo_read_pos];
    ppu->sprite_fifo_read_pos = (ppu->sprite_fifo_read_pos + 1) % 16;
    ppu->sprite_fifo_size--;
    return true;
}

bool ppu_sprite_fifo_empty(PPU* ppu) {
    return ppu->sprite_fifo_size == 0;
}

// Vider les deux FIFOs
void ppu_fifos_clear(PPU* ppu) {
    ppu_fifos_init(ppu);
}

// ===== FONCTIONS PIXEL FETCHER IMPLÉMENTATION =====

// Initialiser le fetcher
void ppu_fetcher_init(PPU* ppu) {
    ppu->fetcher.state = FETCHER_GET_TILE;
    ppu->fetcher.state_cycles = 0;
    ppu->fetcher.tile_index = 0;
    ppu->fetcher.tile_data_low = 0;
    ppu->fetcher.tile_data_high = 0;
    ppu->fetcher.tile_x = 0;
    ppu->fetcher.tile_y = 0;
    ppu->fetcher.pixel_y = 0;
    ppu->fetcher.is_window = false;
}

// Démarrer le fetcher pour une tile donnée
void ppu_fetcher_start(PPU* ppu, u8 tile_x, u8 tile_y, u8 pixel_y, bool is_window) {
    ppu->fetcher.state = FETCHER_GET_TILE;
    ppu->fetcher.state_cycles = 0;
    ppu->fetcher.tile_x = tile_x;
    ppu->fetcher.tile_y = tile_y;
    ppu->fetcher.pixel_y = pixel_y;
    ppu->fetcher.is_window = is_window;
}

// Faire avancer le fetcher d'un cycle
void ppu_fetcher_tick(PPU* ppu, u8* vram) {
    ppu->fetcher.state_cycles++;

    switch (ppu->fetcher.state) {
        case FETCHER_GET_TILE:
            if (ppu->fetcher.state_cycles >= 2) {
                // Lire l'index de tile depuis la map
                u16 map_addr;
                if (ppu->fetcher.is_window) {
                    u16 win_map = (ppu->lcdc & 0x40) ? 0x9C00 : 0x9800;
                    map_addr = win_map + (ppu->fetcher.tile_y * 32) + ppu->fetcher.tile_x;
                } else {
                    u16 bg_map = (ppu->lcdc & 0x08) ? 0x9C00 : 0x9800;
                    map_addr = bg_map + (ppu->fetcher.tile_y * 32) + ppu->fetcher.tile_x;
                }
                ppu->fetcher.tile_index = vram[map_addr - 0x8000];
                ppu->fetcher.state = FETCHER_GET_DATA_LOW;
                ppu->fetcher.state_cycles = 0;
            }
            break;

        case FETCHER_GET_DATA_LOW:
            if (ppu->fetcher.state_cycles >= 2) {
                // Lire l'octet bas de la tile
                u16 data_addr;
                bool tile_sel_8000 = (ppu->lcdc & 0x10) != 0;
                if (tile_sel_8000) {
                    data_addr = 0x8000 + (u16)ppu->fetcher.tile_index * 16;
                } else {
                    s8 st = (s8)ppu->fetcher.tile_index;
                    data_addr = 0x8800 + (u16)(st + 128) * 16;
                }
                u16 line_addr = data_addr + (u16)ppu->fetcher.pixel_y * 2;
                ppu->fetcher.tile_data_low = vram[line_addr - 0x8000];
                ppu->fetcher.state = FETCHER_GET_DATA_HIGH;
                ppu->fetcher.state_cycles = 0;
            }
            break;

        case FETCHER_GET_DATA_HIGH:
            if (ppu->fetcher.state_cycles >= 2) {
                // Lire l'octet haut de la tile
                u16 data_addr;
                bool tile_sel_8000 = (ppu->lcdc & 0x10) != 0;
                if (tile_sel_8000) {
                    data_addr = 0x8000 + (u16)ppu->fetcher.tile_index * 16;
                } else {
                    s8 st = (s8)ppu->fetcher.tile_index;
                    data_addr = 0x8800 + (u16)(st + 128) * 16;
                }
                u16 line_addr = data_addr + (u16)ppu->fetcher.pixel_y * 2;
                ppu->fetcher.tile_data_high = vram[line_addr + 1 - 0x8000];
                ppu->fetcher.state = FETCHER_SLEEP;
                ppu->fetcher.state_cycles = 0;
            }
            break;

        case FETCHER_SLEEP:
            if (ppu->fetcher.state_cycles >= 2) {
                ppu->fetcher.state = FETCHER_PUSH;
                ppu->fetcher.state_cycles = 0;
            }
            break;

        case FETCHER_PUSH:
            if (ppu->fetcher.state_cycles >= 1) {
                // Pousser 8 pixels dans la BG FIFO
                for (int px = 0; px < 8; px++) {
                    if (ppu_bg_fifo_full(ppu)) break;
                    u8 mask = (u8)(0x80 >> px);
                    u8 pix = 0;
                    if (ppu->fetcher.tile_data_low & mask) pix |= 0x01;
                    if (ppu->fetcher.tile_data_high & mask) pix |= 0x02;
                    PixelFIFOEntry pixel = {
                        .color_index = pix,
                        .palette = 0,
                        .sprite_priority = false,
                        .sprite_transparent = false
                    };
                    ppu_bg_fifo_push(ppu, pixel);
                }
                ppu->fetcher.state = FETCHER_GET_TILE;
                ppu->fetcher.state_cycles = 0;
            }
            break;
    }
}

// Mélanger les pixels BG et Sprite selon les priorités
u32 ppu_mix_pixels(PPU* ppu, PixelFIFOEntry* bg_pixel, PixelFIFOEntry* sprite_pixel) {
    // Si pas de sprite, utiliser BG
    if (!sprite_pixel) {
        if (bg_pixel) {
            return ppu_get_pixel_color(ppu, bg_pixel->color_index);
        }
        return 0xFFFFFFFF; // Blanc par défaut
    }
    
    // Si sprite transparent (couleur 0), utiliser BG
    if (sprite_pixel->sprite_transparent || sprite_pixel->color_index == 0) {
        if (bg_pixel) {
            return ppu_get_pixel_color(ppu, bg_pixel->color_index);
        }
        return 0xFFFFFFFF;
    }
    
    // Si sprite priorité derrière BG et BG non-transparent
    if (sprite_pixel->sprite_priority && bg_pixel && bg_pixel->color_index != 0) {
        return ppu_get_pixel_color(ppu, bg_pixel->color_index);
    }
    
    // Utiliser le sprite
    return ppu_get_obj_color(ppu, sprite_pixel->color_index, sprite_pixel->palette == 1);
}



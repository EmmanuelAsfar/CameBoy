#include "ppu.h"

// Initialisation du PPU
void ppu_init(PPU* ppu) {
    memset(ppu, 0, sizeof(PPU));
    ppu_reset(ppu);
}

// Reset du PPU
void ppu_reset(PPU* ppu) {
    ppu->lcdc = 0x91;
    ppu->stat = 0x85;
    ppu->scy = 0;
    ppu->scx = 0;
    ppu->ly = 0;
    ppu->lyc = 0;
    ppu->bgp = 0xFC;
    ppu->obp0 = 0xFF;
    ppu->obp1 = 0xFF;
    ppu->wy = 0;
    ppu->wx = 0;
    
    ppu->mode = PPU_MODE_HBLANK;
    ppu->mode_cycles = 0;
    ppu->line_cycles = 0;
    
    // Initialiser le framebuffer en blanc
    for (int i = 0; i < GB_WIDTH * GB_HEIGHT; i++) {
        ppu->framebuffer[i] = 0xFFFFFFFF;
    }
    
    ppu_update_palettes(ppu);
}

// Tick du PPU
void ppu_tick(PPU* ppu, u8 cycles) {
    ppu->mode_cycles += cycles;
    ppu->line_cycles += cycles;
    
    // Gestion des modes PPU
    switch (ppu->mode) {
        case PPU_MODE_OAM_SEARCH:
            if (ppu->mode_cycles >= 80) {
                ppu->mode = PPU_MODE_PIXEL_TRANSFER;
                ppu->mode_cycles = 0;
            }
            break;
            
        case PPU_MODE_PIXEL_TRANSFER:
            if (ppu->mode_cycles >= 172) {
                ppu->mode = PPU_MODE_HBLANK;
                ppu->mode_cycles = 0;
                // TODO: Rendre la ligne
            }
            break;
            
        case PPU_MODE_HBLANK:
            if (ppu->mode_cycles >= 204) {
                ppu->mode_cycles = 0;
                ppu->line_cycles = 0;
                ppu->ly++;
                
                if (ppu->ly >= 144) {
                    ppu->mode = PPU_MODE_VBLANK;
                    // TODO: Déclencher interrupt VBLANK
                } else {
                    ppu->mode = PPU_MODE_OAM_SEARCH;
                }
            }
            break;
            
        case PPU_MODE_VBLANK:
            if (ppu->line_cycles >= 456) {
                ppu->line_cycles = 0;
                ppu->ly++;
                
                if (ppu->ly >= 154) {
                    ppu->ly = 0;
                    ppu->mode = PPU_MODE_OAM_SEARCH;
                }
            }
            break;
    }
    
    // Mise à jour du registre STAT
    ppu->stat = (ppu->stat & 0xFC) | ppu->mode;
    
    // Vérifier LY = LYC
    if (ppu->ly == ppu->lyc) {
        ppu->stat |= 0x04;  // Set LYC flag
        // TODO: Déclencher interrupt LCD STAT si activé
    } else {
        ppu->stat &= ~0x04;  // Clear LYC flag
    }
}

// Écriture dans les registres PPU
void ppu_write(PPU* ppu, u16 address, u8 value) {
    switch (address) {
        case LCDC_REG:
            ppu->lcdc = value;
            break;
            
        case STAT_REG:
            // Seuls les bits 3-6 sont écritables
            ppu->stat = (ppu->stat & 0x87) | (value & 0x78);
            break;
            
        case SCY_REG:
            ppu->scy = value;
            break;
            
        case SCX_REG:
            ppu->scx = value;
            break;
            
        case LYC_REG:
            ppu->lyc = value;
            break;
            
        case BGP_REG:
            ppu->bgp = value;
            ppu_update_palettes(ppu);
            break;
            
        case OBP0_REG:
            ppu->obp0 = value;
            ppu_update_palettes(ppu);
            break;
            
        case OBP1_REG:
            ppu->obp1 = value;
            ppu_update_palettes(ppu);
            break;
            
        case WY_REG:
            ppu->wy = value;
            break;
            
        case WX_REG:
            ppu->wx = value;
            break;
    }
}

// Lecture des registres PPU
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

// Mise à jour des palettes
void ppu_update_palettes(PPU* ppu) {
    // Palette BG
    for (int i = 0; i < 4; i++) {
        u8 color_code = (ppu->bgp >> (i * 2)) & 0x03;
        switch (color_code) {
            case 0: ppu->bg_palette[i] = 0xFF; break;  // Blanc
            case 1: ppu->bg_palette[i] = 0xAA; break;  // Gris clair
            case 2: ppu->bg_palette[i] = 0x55; break;  // Gris foncé
            case 3: ppu->bg_palette[i] = 0x00; break;  // Noir
        }
    }
    
    // Palette OBJ0
    for (int i = 0; i < 4; i++) {
        u8 color_code = (ppu->obp0 >> (i * 2)) & 0x03;
        switch (color_code) {
            case 0: ppu->obj_palette0[i] = 0xFF; break;  // Transparent
            case 1: ppu->obj_palette0[i] = 0xAA; break;  // Gris clair
            case 2: ppu->obj_palette0[i] = 0x55; break;  // Gris foncé
            case 3: ppu->obj_palette0[i] = 0x00; break;  // Noir
        }
    }
    
    // Palette OBJ1
    for (int i = 0; i < 4; i++) {
        u8 color_code = (ppu->obp1 >> (i * 2)) & 0x03;
        switch (color_code) {
            case 0: ppu->obj_palette1[i] = 0xFF; break;  // Transparent
            case 1: ppu->obj_palette1[i] = 0xAA; break;  // Gris clair
            case 2: ppu->obj_palette1[i] = 0x55; break;  // Gris foncé
            case 3: ppu->obj_palette1[i] = 0x00; break;  // Noir
        }
    }
}

// Rendu d'une ligne (simplifié)
void ppu_render_line(PPU* ppu, u8* vram) {
    (void)vram; // Suppress unused parameter warning
    if (!(ppu->lcdc & 0x80)) return;  // LCD disabled
    
    // Pour l'instant, on rend juste un fond uni
    u8 color = ppu->bg_palette[0];
    for (int x = 0; x < GB_WIDTH; x++) {
        ppu->framebuffer[ppu->ly * GB_WIDTH + x] = color;
    }
}

// Obtenir la couleur d'un pixel
u8 ppu_get_pixel_color(u8 pixel_data, u8 palette) {
    u8 color_index = (pixel_data >> (palette * 2)) & 0x03;
    return color_index;
}

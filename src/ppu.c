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
    ppu->bgp = 0xE4;  // Palette Blargg: blanc, gris clair, gris foncé, noir
    ppu->obp0 = 0xFF;
    ppu->obp1 = 0xFF;
    ppu->wy = 0;
    ppu->wx = 0;
    
    ppu->mode = PPU_MODE_OAM_SEARCH;
    ppu->mode_cycles = 0;
    ppu->line_cycles = 0;
    
    // Initialiser le framebuffer en blanc
    for (int i = 0; i < GB_WIDTH * GB_HEIGHT; i++) {
        ppu->framebuffer[i] = 0xFFFFFFFF;
    }
    
    ppu_update_palettes(ppu);
}

// Tick du PPU - Retourne les interruptions déclenchées
u8 ppu_tick(PPU* ppu, u8 cycles, u8* vram) {
    u8 interrupts = 0;
    ppu->mode_cycles += cycles;
    ppu->line_cycles += cycles;

    // Machine à états basée sur mode_cycles (80 / 172 / 204 / 456)
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
                ppu_render_line(ppu, vram);
            }
            break;
        case PPU_MODE_HBLANK:
            // Terminer la ligne strictement à 456 dots (conforme aux tests)
            if (ppu->line_cycles >= 456) {
                ppu->mode_cycles = 0;
                ppu->line_cycles = 0;
                ppu->ly++;
                if (ppu->ly == 144) {
                    ppu->mode = PPU_MODE_VBLANK;
                    interrupts |= 0x01;
                } else {
                    ppu->mode = PPU_MODE_OAM_SEARCH;
                }
            }
            break;
        case PPU_MODE_VBLANK:
            if (ppu->mode_cycles >= 456) {
                ppu->mode_cycles = 0;
                ppu->line_cycles = 0;
                ppu->ly++;
                if (ppu->ly >= 154) {
                    ppu->ly = 0;
                    ppu->mode = PPU_MODE_OAM_SEARCH;
                }
            }
            break;
    }
    
    // Mise à jour minimale du registre STAT: bits 0-1 (mode) et bit 2 (LYC==LY)
    ppu->stat = (ppu->stat & 0xF8) | (ppu->mode & 0x03);
    if (ppu->ly == ppu->lyc) {
        ppu->stat |= 0x04;
    } else {
        ppu->stat &= (u8)~0x04;
    }
    
    return interrupts;
}

// Écriture dans les registres PPU
void ppu_write(PPU* ppu, u16 address, u8 value) {
    switch (address) {
        case LCDC_REG:
            ppu->lcdc = value;
            break;
            
        case STAT_REG:
            // Bits 3-6 et 7 écrits par la CPU; bits 0-2 reflètent le mode/LYC
            ppu->stat = (ppu->stat & 0x07) | (value & 0xF8);
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

// Rendu d'une ligne (vrai rendu Game Boy)
void ppu_render_line(PPU* ppu, u8* vram) {
    if (!vram) return; // Pas de VRAM disponible
    
    if (!(ppu->lcdc & 0x80)) {
        // LCD désactivé - afficher un pattern de test
        for (int x = 0; x < GB_WIDTH; x++) {
            u32 color = 0x808080FF; // Gris
            ppu->framebuffer[ppu->ly * GB_WIDTH + x] = color;
        }
        return;
    }
    
    // Debug réduit - supprimé
    
    // Rendu normal Game Boy
    
    // Rendu du background
    if (ppu->lcdc & 0x01) { // Background enabled
        // Calculer la ligne de tuiles
        u8 tile_y = (ppu->ly + ppu->scy) / 8;
        u8 pixel_y = (ppu->ly + ppu->scy) % 8;
        
        // Adresse de la tile map
        u16 tile_map_addr = (ppu->lcdc & 0x08) ? 0x9C00 : 0x9800;
        
        for (int x = 0; x < GB_WIDTH; x++) {
            // Calculer la colonne de tuiles
            u8 tile_x = (x + ppu->scx) / 8;
            u8 pixel_x = (x + ppu->scx) % 8;
            
            // Lire l'index de la tuile
            u16 tile_addr = tile_map_addr + tile_y * 32 + tile_x;
            u8 tile_index = vram[tile_addr - 0x8000];
            
            // Debug réduit - supprimé
            
            // Adresse des données de la tuile
            u16 tile_data_addr;
            if (ppu->lcdc & 0x10) {
                // Mode 0x8000 : tuiles 0-255
                tile_data_addr = 0x8000 + tile_index * 16;
            } else {
                // Mode 0x8800 : tuiles -128 à 127
                s8 signed_tile = (s8)tile_index;
                tile_data_addr = 0x8800 + (signed_tile + 128) * 16;
            }
            
            // Lire les 2 octets de la ligne de la tuile
            u8 line_addr = tile_data_addr + pixel_y * 2;
            u8 byte1 = vram[line_addr - 0x8000];
            u8 byte2 = vram[line_addr + 1 - 0x8000];
            
            // Debug réduit - supprimé
            
            // Extraire le pixel (bit 7-x de byte1 et byte2)
            u8 pixel = 0;
            if (byte1 & (0x80 >> pixel_x)) pixel |= 0x01;
            if (byte2 & (0x80 >> pixel_x)) pixel |= 0x02;
            
            // Convertir en couleur selon la palette BGP
            u32 color = ppu_get_pixel_color(ppu, pixel);
            ppu->framebuffer[ppu->ly * GB_WIDTH + x] = color;
        }
    } else {
        // Background désactivé - pixels blancs
        for (int x = 0; x < GB_WIDTH; x++) {
            ppu->framebuffer[ppu->ly * GB_WIDTH + x] = 0xFFFFFFFF;
        }
    }
}

// Obtenir la couleur d'un pixel selon la palette BGP
u32 ppu_get_pixel_color(PPU* ppu, u8 pixel) {
    // Extraire la couleur de la palette BGP
    u8 color_index = (ppu->bgp >> (pixel * 2)) & 0x03;
    
    // Convertir en couleur RGBA
    switch (color_index) {
        case 0: return 0xFFFFFFFF; // Blanc
        case 1: return 0xAAAAAAFF; // Gris clair
        case 2: return 0x555555FF; // Gris foncé
        case 3: return 0x000000FF; // Noir
        default: return 0x000000FF;
    }
}

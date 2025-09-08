#include "mmu.h"
#include "timer.h"
#include "apu.h"
#include "joypad.h"
#include "ppu.h"

static inline bool mmu_vram_access_allowed(MMU* mmu) {
    if (!mmu->ppu) return true;
    PPUMode mode = ((PPU*)mmu->ppu)->mode;
    return mode != PPU_MODE_PIXEL_TRANSFER; // bloqué en mode 3
}

static inline bool mmu_oam_access_allowed(MMU* mmu) {
    if (mmu->dma.active) return false; // OAM bloqué pendant DMA
    if (!mmu->ppu) return true;
    PPUMode mode = ((PPU*)mmu->ppu)->mode;
    return !(mode == PPU_MODE_OAM_SEARCH || mode == PPU_MODE_PIXEL_TRANSFER); // bloqué en 2 et 3
}

// Initialisation de la MMU
void mmu_init(MMU* mmu) {
    memset(mmu, 0, sizeof(MMU));
    
    // Allouer la mémoire totale (64KB)
    mmu->memory = malloc(0x10000);
    if (!mmu->memory) {
        printf("Erreur: Impossible d'allouer la mémoire\n");
        exit(1);
    }
    // Initialiser à 0xFF comme état par défaut non-initialisé
    memset(mmu->memory, 0xFF, 0x10000);
    
    // Pointeurs vers les zones mémoire
    mmu->rom = &mmu->memory[0x0000];
    mmu->vram = &mmu->memory[0x8000];
    mmu->eram = &mmu->memory[0xA000];
    mmu->wram = &mmu->memory[0xC000];
    mmu->oam = &mmu->memory[0xFE00];
    mmu->io = &mmu->memory[0xFF00];
    mmu->hram = &mmu->memory[0xFF80];
    
    mmu->dma.active = false;
    mmu->dma.source_addr = 0;
    mmu->dma.index = 0;
    mmu->dma.cycles_accum = 0;
    
    mmu_reset(mmu);
}

// Nettoyage de la MMU
void mmu_cleanup(MMU* mmu) {
    if (mmu->memory) {
        free(mmu->memory);
        mmu->memory = NULL;
    }
    
    if (mmu->cart.rom_data) {
        free(mmu->cart.rom_data);
        mmu->cart.rom_data = NULL;
    }
    
    if (mmu->cart.ram_data) {
        free(mmu->cart.ram_data);
        mmu->cart.ram_data = NULL;
    }
}

// Reset de la MMU
void mmu_reset(MMU* mmu) {
    // Ré-initialiser toute la RAM à 0xFF (zones non écrites lues à 0xFF)
    memset(mmu->memory, 0xFF, 0x10000);

    // Initialiser les valeurs par défaut des registres IO
    mmu->memory[0xFF00] = 0xCF;  // P1
    mmu->memory[0xFF01] = 0x00;  // SB
    mmu->memory[0xFF02] = 0x7E;  // SC
    mmu->memory[0xFF04] = 0x00;  // DIV
    mmu->memory[0xFF05] = 0x00;  // TIMA
    mmu->memory[0xFF06] = 0x00;  // TMA
    mmu->memory[0xFF07] = 0x00;  // TAC
    mmu->memory[0xFF0F] = 0xE1;  // IF
    mmu->memory[0xFF40] = 0x91;  // LCDC
    mmu->memory[0xFF41] = 0x85;  // STAT
    mmu->memory[0xFF42] = 0x00;  // SCY
    mmu->memory[0xFF43] = 0x00;  // SCX
    mmu->memory[0xFF44] = 0x90;  // LY (valeur attendue par les tests Blargg)
    mmu->memory[0xFF45] = 0x00;  // LYC
    mmu->memory[0xFF46] = 0x00;  // DMA
    mmu->memory[0xFF47] = 0xFC;  // BGP
    mmu->memory[0xFF48] = 0xFF;  // OBP0
    mmu->memory[0xFF49] = 0xFF;  // OBP1
    mmu->memory[0xFF4A] = 0x00;  // WY
    mmu->memory[0xFF4B] = 0x00;  // WX
    // CGB registers defaults
    mmu->memory[KEY1_REG] = 0x00; // KEY1 (DMG would read as 0xFF)
    mmu->memory[0xFF50] = 0x01;  // BOOT ROM disable
    mmu->memory[0xFFFF] = 0x00;  // IE

    // CGB flags
    mmu->is_cgb = false;
    mmu->cgb_double_speed = false;
}

// Chargement d'une ROM
bool mmu_load_rom(MMU* mmu, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Erreur: Impossible d'ouvrir le fichier %s\n", filename);
        return false;
    }
    
    // Obtenir la taille du fichier
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size < 0x8000) {
        printf("Erreur: Fichier ROM trop petit\n");
        fclose(file);
        return false;
    }
    
    // Allouer et lire la ROM
    mmu->cart.rom_data = malloc(file_size);
    if (!mmu->cart.rom_data) {
        printf("Erreur: Impossible d'allouer la mémoire pour la ROM\n");
        fclose(file);
        return false;
    }
    
    size_t bytes_read = fread(mmu->cart.rom_data, 1, file_size, file);
    fclose(file);
    
    if ((long)bytes_read != file_size) {
        printf("Erreur: Lecture incomplète de la ROM\n");
        free(mmu->cart.rom_data);
        mmu->cart.rom_data = NULL;
        return false;
    }
    
    mmu->cart.rom_size = file_size;
    
    // Parser l'en-tête de la cartouche
    if (!cart_parse_header(&mmu->cart, mmu->cart.rom_data)) {
        printf("Erreur: En-tête de cartouche invalide\n");
        return false;
    }
    
    // Copier la ROM dans la mémoire (optionnel, utile pour tests simples)
    size_t copy_size = (file_size > 0x8000) ? 0x8000 : file_size;
    memcpy(mmu->rom, mmu->cart.rom_data, copy_size);
    
    // Allouer la RAM de cartouche si nécessaire
    if (mmu->cart.ram_size > 0) {
        mmu->cart.ram_data = calloc(mmu->cart.ram_size, 1);
        if (!mmu->cart.ram_data) {
            printf("Erreur: Impossible d'allouer la RAM de cartouche\n");
            return false;
        }
    }
    
    printf("ROM chargée: %s\n", mmu->cart.header.title);
    printf("Type: %s\n", cart_type_name(mmu->cart.type));
    printf("Taille ROM: %d KB\n", (int)(mmu->cart.rom_size / 1024));
    if (mmu->cart.ram_size > 0) {
        printf("Taille RAM: %d KB\n", (int)(mmu->cart.ram_size / 1024));
    }
    
    return true;
}

// Lecture d'un octet
u8 mmu_read8(MMU* mmu, u16 address) {
    // Pendant un DMA actif, seul l'accès HRAM (FF80-FFFE) est autorisé au CPU
    if (mmu->dma.active) {
        if (!((address >= 0xFF80 && address <= 0xFFFE) || (address == 0xFF00) || (address == 0xFF04) || (address == 0xFF05) || (address == 0xFF06) || (address == 0xFF07))) {
            // Bloquer les autres bus (renvoie 0xFF comme hardware généralement)
            // Exceptions: certains IO critiques peuvent rester lisibles, mais on reste simple ici
            // Sauf si PPU/Timer/APU lisent via MMU interne (non CPU). Ce chemin modélise accès CPU
            return 0xFF;
        }
    }
    if (address <= 0x7FFF) {
        // ROM (via MBC mapping). Fallback to raw memory if no ROM loaded (unit tests)
        if (mmu->cart.rom_data == NULL || mmu->cart.rom_size == 0) {
            return mmu->memory[address];
        }
        return mbc_read(mmu, address);
    } else if (address >= 0x8000 && address <= 0x9FFF) {
        // VRAM (bloquée en Mode 3)
        if (!mmu_vram_access_allowed(mmu)) return 0xFF;
        return mmu->vram[address - 0x8000];
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        // ERAM (External RAM via MBC)
        return mbc_read(mmu, address);
    } else if (address >= 0xC000 && address <= 0xDFFF) {
        // WRAM
        return mmu->wram[address - 0xC000];
    } else if (address >= 0xE000 && address <= 0xFDFF) {
        // Echo RAM (miroir de WRAM)
        return mmu->wram[address - 0xE000];
    } else if (address >= 0xFE00 && address <= 0xFE9F) {
        // OAM (bloquée en Mode 2/3 et pendant DMA)
        if (!mmu_oam_access_allowed(mmu)) return 0xFF;
        return mmu->oam[address - 0xFE00];
    } else if (address >= 0xFF00 && address <= 0xFF7F) {
        // IO
        if (address == 0xFF00 && mmu->joypad) {
            return joypad_read((Joypad*)mmu->joypad);
        }
        // CGB KEY1 read: on DMG, reads 0xFF
        if (address == KEY1_REG) {
            if (!mmu->is_cgb) return 0xFF;
            u8 key1 = mmu->io[KEY1_REG - 0xFF00] & 0x01; // bit0 prepare
            if (mmu->cgb_double_speed) key1 |= 0x80;     // bit7 current speed
            // Bits 1-6 typically return 1
            return key1 | 0x7E;
        }
        // Connecter les registres timer au timer
        if (address >= 0xFF04 && address <= 0xFF07) {
            return timer_read((Timer*)mmu->timer, address);
        }
        // Connecter les registres audio à l'APU
        if (address >= 0xFF10 && address <= 0xFF3F) {
            return apu_read((APU*)mmu->apu, address);
        }
        // Autres registres IO
        return mmu->io[address - 0xFF00];
    } else if (address >= 0xFF80 && address <= 0xFFFE) {
        // HRAM
        return mmu->hram[address - 0xFF80];
    } else if (address == 0xFFFF) {
        // IE
        return mmu->memory[0xFFFF];
    }
    
    return 0xFF;  // Valeur par défaut
}

// Lecture d'un mot (16 bits)
u16 mmu_read16(MMU* mmu, u16 address) {
    u8 low = mmu_read8(mmu, address);
    u8 high = mmu_read8(mmu, address + 1);
    return low | (high << 8);
}

// Écriture d'un octet
void mmu_write8(MMU* mmu, u16 address, u8 value) {
    // Pendant un DMA actif, le CPU ne peut écrire que HRAM (FF80-FFFE) et quelques IO; sinon ignore
    if (mmu->dma.active) {
        if (!(address >= 0xFF80 && address <= 0xFFFE) && address != 0xFF46 && !(address >= 0xFF04 && address <= 0xFF07)) {
            return;
        }
    }
    if (address <= 0x7FFF) {
        // ROM area - route vers MBC
        mbc_write(mmu, address, value);
    } else if (address >= 0x8000 && address <= 0x9FFF) {
        // VRAM (bloquée en Mode 3)
        if (!mmu_vram_access_allowed(mmu)) return;
        mmu->vram[address - 0x8000] = value;
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        // ERAM via MBC
        mbc_write(mmu, address, value);
    } else if (address >= 0xC000 && address <= 0xDFFF) {
        // WRAM
        mmu->wram[address - 0xC000] = value;
        // if (address == 0xC100) { printf("WRAM[0xC100] <= 0x%02X\n", value); }
    } else if (address >= 0xE000 && address <= 0xFDFF) {
        // Echo RAM (miroir de WRAM)
        mmu->wram[address - 0xE000] = value;
    } else if (address >= 0xFE00 && address <= 0xFE9F) {
        // OAM (bloquée en Mode 2/3 et pendant DMA)
        if (!mmu_oam_access_allowed(mmu)) return;
        mmu->oam[address - 0xFE00] = value;
    } else if (address >= 0xFF00 && address <= 0xFF7F) {
        // IO
        // Connecter les registres timer au timer
        if (address >= 0xFF04 && address <= 0xFF07) {
            timer_write((Timer*)mmu->timer, address, value);
            return; // Ne pas écrire dans mmu->io
        }
        // Connecter les registres audio à l'APU
        if (address >= 0xFF10 && address <= 0xFF3F) {
            apu_write((APU*)mmu->apu, address, value);
            return; // Ne pas écrire dans mmu->io
        }
        
        // Joypad P1 (sélection et lecture via struct Joypad)
        if (address == 0xFF00 && mmu->joypad) {
            joypad_write((Joypad*)mmu->joypad, value);
            mmu->io[address - 0xFF00] = (u8)((mmu->io[address - 0xFF00] & 0x0F) | (value & 0x30));
            return;
        }

        // OAM DMA
        if (address == 0xFF46) {
            mmu->io[address - 0xFF00] = value;
            u16 src = (u16)value << 8;
            // Démarrer un DMA temporisé (1 octet toutes les 4 cycles CPU, total 160 octets)
            mmu->dma.active = true;
            mmu->dma.source_addr = src;
            mmu->dma.index = 0;
            mmu->dma.cycles_accum = 0;
            return;
        }

        // Support du port série pour les tests
        if (address == 0xFF01) {  // SB - Serial Data
            mmu->io[address - 0xFF00] = value;
        } else if (address == 0xFF02) {  // SC - Serial Control
            mmu->io[address - 0xFF00] = value;
            // Si bit 7 est activé, transmettre le caractère
            if (value & 0x80) {
                u8 data = mmu->io[0xFF01 - 0xFF00];
                // Si un callback est défini (GUI), ne pas écrire sur stdout
                if (mmu->serial_cb) {
                    mmu->serial_cb(data);
                } else {
                    // Mode RUN: conserver la sortie console existante
                    printf("SERIAL: 0x%02X ('%c')\n", data, (data >= 32 && data <= 126) ? data : '.');
                    putchar((int)data);
                    fflush(stdout);
                }
                // Remettre le bit 7 à 0 après transmission
                mmu->io[address - 0xFF00] = 0x00;
            }
        } else {
            // KEY1 write handling (CGB only)
            if (address == KEY1_REG) {
                if (!mmu->is_cgb) return; // ignore on DMG
                // Only bit0 is writable (prepare speed switch)
                u8 v = value & 0x01;
                // preserve bit7 (current speed) in io mirror for readback convenience
                u8 cur = mmu->io[address - 0xFF00] & 0x80;
                mmu->io[address - 0xFF00] = cur | v;
                return;
            }
            // Autres registres IO
            mmu->io[address - 0xFF00] = value;
        }
    } else if (address >= 0xFF80 && address <= 0xFFFE) {
        // HRAM
        mmu->hram[address - 0xFF80] = value;
        // if (address == 0xFF80) { printf("HRAM[0xFF80] <= 0x%02X\n", value); }
    } else if (address == 0xFFFF) {
        // IE
        mmu->memory[0xFFFF] = value;
    }
}

// Écriture d'un mot (16 bits)
void mmu_write16(MMU* mmu, u16 address, u16 value) {
    mmu_write8(mmu, address, value & 0xFF);
    mmu_write8(mmu, address + 1, (value >> 8) & 0xFF);
}

// Fonctions MBC (simplifiées)
void mbc_write(MMU* mmu, u16 address, u8 value) {
    // Implement minimal MBC1 (common in Blargg ROMs)
    // 0000-1FFF: RAM enable
    // 2000-3FFF: ROM bank low 5 bits (bank 0 -> map to 1)
    // 4000-5FFF: Upper 2 bits of ROM bank or RAM bank (mode dependent)
    // 6000-7FFF: Banking mode select (0=ROM banking, 1=RAM banking)
    if (mmu->cart.type == CART_MBC1 || mmu->cart.type == CART_MBC1_RAM || mmu->cart.type == CART_MBC1_RAM_BATTERY) {
        if (address <= 0x1FFF) {
            mmu->cart.ram_enabled = ((value & 0x0F) == 0x0A);
        } else if (address >= 0x2000 && address <= 0x3FFF) {
            u8 bank = value & 0x1F;
            if (bank == 0) bank = 1; // bank 0 forbidden for 4000-7FFF
            mmu->cart.rom_bank = (mmu->cart.rom_bank & 0x60) | bank;
        } else if (address >= 0x4000 && address <= 0x5FFF) {
            u8 upper = (value & 0x03) << 5; // bits 5-6
            if (mmu->cart.rom_banking_mode) {
                mmu->cart.rom_bank = (mmu->cart.rom_bank & 0x1F) | upper;
            } else {
                mmu->cart.ram_bank = value & 0x03;
            }
        } else if (address >= 0x6000 && address <= 0x7FFF) {
            mmu->cart.rom_banking_mode = ((value & 0x01) != 0);
        } else if (address >= 0xA000 && address <= 0xBFFF) {
            if (mmu->cart.ram_enabled && mmu->cart.ram_data) {
                u32 ram_offset = (u32)mmu->cart.ram_bank * 0x2000 + (address - 0xA000);
                if (ram_offset < mmu->cart.ram_size) {
                    mmu->cart.ram_data[ram_offset] = value;
                }
            }
        }
        return;
    }
    // Other MBCs not implemented yet
}

u8 mbc_read(MMU* mmu, u16 address) {
    if (address <= 0x7FFF) {
        if (mmu->cart.type == CART_MBC1 || mmu->cart.type == CART_MBC1_RAM || mmu->cart.type == CART_MBC1_RAM_BATTERY) {
            if (address < 0x4000) {
                // Bank 0 (or upper bank in RAM banking mode)
                u32 base_bank = 0;
                if (!mmu->cart.rom_banking_mode) {
                    // RAM banking mode: 0000-3FFF may map to 0x00,0x20,0x40,0x60
                    base_bank = ((mmu->cart.rom_bank & 0x60) * 0x4000) / 0x20; // extract bits 5-6
                }
                u32 rom_address = base_bank + address;
                if (rom_address < mmu->cart.rom_size) return mmu->cart.rom_data[rom_address];
                return 0xFF;
            } else {
                u8 bank = mmu->cart.rom_bank;
                if ((bank & 0x1F) == 0) bank |= 0x01; // ensure not 0
                u32 rom_address = (u32)bank * 0x4000 + (address - 0x4000);
                if (rom_address < mmu->cart.rom_size) return mmu->cart.rom_data[rom_address];
                return 0xFF;
            }
        } else {
            // ROM only
            if (mmu->cart.rom_data && address < mmu->cart.rom_size) return mmu->cart.rom_data[address];
            // Fallback for unit tests where instructions are placed in mmu->memory
            return mmu->memory[address];
        }
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        if (mmu->cart.ram_enabled && mmu->cart.ram_data) {
            u32 ram_offset = (u32)mmu->cart.ram_bank * 0x2000 + (address - 0xA000);
            if (ram_offset < mmu->cart.ram_size) return mmu->cart.ram_data[ram_offset];
        }
        return 0xFF;
    }
    return 0xFF;
}

// Parsing de l'en-tête de cartouche
bool cart_parse_header(Cartridge* cart, u8* rom_data) {
    memcpy(&cart->header, &rom_data[0x100], sizeof(CartHeader));
    
    // Vérifier le logo Nintendo
    const u8 nintendo_logo[48] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
        0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
        0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };
    
    if (memcmp(cart->header.nintendo_logo, nintendo_logo, 48) != 0) {
        printf("Avertissement: Logo Nintendo invalide\n");
    }
    
    // Déterminer le type de cartouche
    cart->type = (CartType)cart->header.cart_type;
    
    // Déterminer la taille de la ROM
    u8 rom_size_code = cart->header.rom_size;
    if (rom_size_code <= 8) {
        cart->rom_size = 32 * 1024 * (1 << rom_size_code);  // 32KB * 2^code
    } else {
        printf("Erreur: Taille de ROM non supportée\n");
        return false;
    }
    
    // Déterminer la taille de la RAM
    u8 ram_size_code = cart->header.ram_size;
    switch (ram_size_code) {
        case 0x00: cart->ram_size = 0; break;
        case 0x01: cart->ram_size = 2 * 1024; break;      // 2KB
        case 0x02: cart->ram_size = 8 * 1024; break;      // 8KB
        case 0x03: cart->ram_size = 32 * 1024; break;     // 32KB
        case 0x04: cart->ram_size = 128 * 1024; break;    // 128KB
        case 0x05: cart->ram_size = 64 * 1024; break;     // 64KB
        default:
            printf("Erreur: Taille de RAM non supportée\n");
            return false;
    }
    
    return true;
}

// Nom du type de cartouche
const char* cart_type_name(CartType type) {
    switch (type) {
        case CART_ROM_ONLY: return "ROM Only";
        case CART_MBC1: return "MBC1";
        case CART_MBC1_RAM: return "MBC1 + RAM";
        case CART_MBC1_RAM_BATTERY: return "MBC1 + RAM + Battery";
        case CART_MBC2: return "MBC2";
        case CART_MBC2_BATTERY: return "MBC2 + Battery";
        case CART_ROM_RAM: return "ROM + RAM";
        case CART_ROM_RAM_BATTERY: return "ROM + RAM + Battery";
        case CART_MBC3: return "MBC3";
        case CART_MBC3_RAM: return "MBC3 + RAM";
        case CART_MBC3_RAM_BATTERY: return "MBC3 + RAM + Battery";
        case CART_MBC5: return "MBC5";
        case CART_MBC5_RAM: return "MBC5 + RAM";
        case CART_MBC5_RAM_BATTERY: return "MBC5 + RAM + Battery";
        default: return "Unknown";
    }
}

// Définir le callback série
void mmu_set_serial_callback(MMU* mmu, mmu_serial_cb_t callback) {
    mmu->serial_cb = callback;
}

void mmu_set_joypad(MMU* mmu, void* joypad) {
    mmu->joypad = joypad;
}

void mmu_set_ppu(MMU* mmu, void* ppu) {
    mmu->ppu = ppu;
}

void mmu_request_joypad_irq(MMU* mmu) {
    // Set IF bit 4
    u8 if_reg = mmu->memory[IF_REG];
    if_reg |= 0x10;
    mmu->memory[IF_REG] = if_reg;
}

// Tick DMA OAM: copie 1 octet toutes les 4 cycles jusqu'à 160 octets
void mmu_dma_tick(MMU* mmu, u16 cycles) {
    if (!mmu->dma.active) return;
    mmu->dma.cycles_accum += cycles;
    while (mmu->dma.active && mmu->dma.cycles_accum >= 4) {
        mmu->dma.cycles_accum -= 4;
        if (mmu->dma.index < 160) {
            // Lire la source en ignorant les verrous CPU (DMA matériel accède au bus directement)
            u16 addr = (u16)(mmu->dma.source_addr + mmu->dma.index);
            u8 b;
            if (addr <= 0x7FFF) {
                b = mbc_read(mmu, addr);
            } else if (addr >= 0x8000 && addr <= 0x9FFF) {
                b = mmu->vram[addr - 0x8000];
            } else if (addr >= 0xA000 && addr <= 0xBFFF) {
                b = mbc_read(mmu, addr);
            } else if (addr >= 0xC000 && addr <= 0xDFFF) {
                b = mmu->wram[addr - 0xC000];
            } else if (addr >= 0xE000 && addr <= 0xFDFF) {
                b = mmu->wram[addr - 0xE000];
            } else if (addr >= 0xFE00 && addr <= 0xFE9F) {
                b = mmu->oam[addr - 0xFE00];
            } else if (addr >= 0xFF80 && addr <= 0xFFFE) {
                b = mmu->hram[addr - 0xFF80];
            } else if (addr >= 0xFF00 && addr <= 0xFF7F) {
                b = mmu->io[addr - 0xFF00];
            } else if (addr == 0xFFFF) {
                b = mmu->memory[0xFFFF];
            } else {
                b = 0xFF;
            }
            mmu->oam[mmu->dma.index] = b;
            mmu->dma.index++;
        }
        if (mmu->dma.index >= 160) {
            mmu->dma.active = false;
        }
    }
}

#include "mmu.h"
#include "timer.h"

// Initialisation de la MMU
void mmu_init(MMU* mmu) {
    memset(mmu, 0, sizeof(MMU));
    
    // Allouer la mémoire totale (64KB)
    mmu->memory = calloc(0x10000, 1);
    if (!mmu->memory) {
        printf("Erreur: Impossible d'allouer la mémoire\n");
        exit(1);
    }
    
    // Pointeurs vers les zones mémoire
    mmu->rom = &mmu->memory[0x0000];
    mmu->vram = &mmu->memory[0x8000];
    mmu->eram = &mmu->memory[0xA000];
    mmu->wram = &mmu->memory[0xC000];
    mmu->oam = &mmu->memory[0xFE00];
    mmu->io = &mmu->memory[0xFF00];
    mmu->hram = &mmu->memory[0xFF80];
    
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
    mmu->memory[0xFF50] = 0x01;  // BOOT ROM disable
    mmu->memory[0xFFFF] = 0x00;  // IE
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
    
    // Copier la ROM dans la mémoire
    // Pour les ROMs simples, copier toute la ROM
    size_t copy_size = (file_size > 0x8000) ? 0x8000 : file_size;
    memcpy(mmu->rom, mmu->cart.rom_data, copy_size);
    
    // Pour les ROMs plus grandes, utiliser le MBC
    if (file_size > 0x8000) {
        // Copier les 32KB supplémentaires dans la zone MBC
        size_t remaining = file_size - 0x8000;
        if (remaining > 0x8000) remaining = 0x8000;  // Limiter à 64KB total
        memcpy(mmu->rom + 0x4000, mmu->cart.rom_data + 0x8000, remaining);
    }
    
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
    if (address <= 0x7FFF) {
        // ROM
        if (address < 0x4000) {
            return mmu->rom[address];
        } else {
            // Banking ROM
            return mbc_read(mmu, address);
        }
    } else if (address >= 0x8000 && address <= 0x9FFF) {
        // VRAM
        return mmu->vram[address - 0x8000];
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        // ERAM (External RAM)
        return mbc_read(mmu, address);
    } else if (address >= 0xC000 && address <= 0xDFFF) {
        // WRAM
        return mmu->wram[address - 0xC000];
    } else if (address >= 0xE000 && address <= 0xFDFF) {
        // Echo RAM (miroir de WRAM)
        return mmu->wram[address - 0xE000];
    } else if (address >= 0xFE00 && address <= 0xFE9F) {
        // OAM
        return mmu->oam[address - 0xFE00];
    } else if (address >= 0xFF00 && address <= 0xFF7F) {
        // IO
        // Connecter les registres timer au timer
        if (address >= 0xFF04 && address <= 0xFF07) {
            return timer_read((Timer*)mmu->timer, address);
        }
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
    if (address <= 0x7FFF) {
        // ROM - écriture vers MBC
        mbc_write(mmu, address, value);
    } else if (address >= 0x8000 && address <= 0x9FFF) {
        // VRAM
        mmu->vram[address - 0x8000] = value;
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        // ERAM - écriture vers MBC
        mbc_write(mmu, address, value);
    } else if (address >= 0xC000 && address <= 0xDFFF) {
        // WRAM
        mmu->wram[address - 0xC000] = value;
    } else if (address >= 0xE000 && address <= 0xFDFF) {
        // Echo RAM (miroir de WRAM)
        mmu->wram[address - 0xE000] = value;
    } else if (address >= 0xFE00 && address <= 0xFE9F) {
        // OAM
        mmu->oam[address - 0xFE00] = value;
    } else if (address >= 0xFF00 && address <= 0xFF7F) {
        // IO
        mmu->io[address - 0xFF00] = value;
        
        // Support du port série pour les tests
        if (address == 0xFF01) {  // SB - Serial Data
            // Juste stocker la valeur
        } else if (address == 0xFF02) {  // SC - Serial Control
            // Si bit 7 est activé, transmettre le caractère
            if (value & 0x80) {
                u8 data = mmu->io[0xFF01 - 0xFF00];
                printf("SERIAL: 0x%02X ('%c')\n", data, (data >= 32 && data <= 126) ? data : '.');
                fflush(stdout);
                // Remettre le bit 7 à 0 après transmission
                mmu->io[address - 0xFF00] = 0x00;
            }
        }
        
        // Connecter les registres timer au timer
        if (address >= 0xFF04 && address <= 0xFF07) {
            timer_write((Timer*)mmu->timer, address, value);
        }
    } else if (address >= 0xFF80 && address <= 0xFFFE) {
        // HRAM
        mmu->hram[address - 0xFF80] = value;
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
    (void)mmu;     // Suppress unused parameter warning
    (void)address; // Suppress unused parameter warning
    (void)value;   // Suppress unused parameter warning
    // Pour l'instant, on ne gère que les ROMs sans MBC
    // Les MBC seront implémentés plus tard
}

u8 mbc_read(MMU* mmu, u16 address) {
    if (address >= 0x4000 && address <= 0x7FFF) {
        // Pour les ROMs de 32KB, mapper la zone 0x4000-0x7FFF vers 0x4000-0x7FFF de la ROM
        u32 rom_address = address;
        if (rom_address < mmu->cart.rom_size) {
            return mmu->cart.rom_data[rom_address];
        }
        // Si on dépasse la taille de la ROM, retourner 0xFF
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

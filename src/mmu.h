#ifndef MMU_H
#define MMU_H

#include "common.h"

// Types de cartouche
typedef enum {
    CART_ROM_ONLY = 0x00,
    CART_MBC1 = 0x01,
    CART_MBC1_RAM = 0x02,
    CART_MBC1_RAM_BATTERY = 0x03,
    CART_MBC2 = 0x05,
    CART_MBC2_BATTERY = 0x06,
    CART_ROM_RAM = 0x08,
    CART_ROM_RAM_BATTERY = 0x09,
    CART_MMM01 = 0x0B,
    CART_MMM01_RAM = 0x0C,
    CART_MMM01_RAM_BATTERY = 0x0D,
    CART_MBC3_TIMER_BATTERY = 0x0F,
    CART_MBC3_TIMER_RAM_BATTERY = 0x10,
    CART_MBC3 = 0x11,
    CART_MBC3_RAM = 0x12,
    CART_MBC3_RAM_BATTERY = 0x13,
    CART_MBC5 = 0x19,
    CART_MBC5_RAM = 0x1A,
    CART_MBC5_RAM_BATTERY = 0x1B,
    CART_MBC5_RUMBLE = 0x1C,
    CART_MBC5_RUMBLE_RAM = 0x1D,
    CART_MBC5_RUMBLE_RAM_BATTERY = 0x1E,
    CART_POCKET_CAMERA = 0xFC,
    CART_BANDAI_TAMA5 = 0xFD,
    CART_HUC3 = 0xFE,
    CART_HUC1_RAM_BATTERY = 0xFF
} CartType;

// En-tête de cartouche
typedef struct {
    u8 entry_point[4];
    u8 nintendo_logo[48];
    char title[16];
    u16 new_licensee_code;
    u8 sgb_flag;
    u8 cart_type;
    u8 rom_size;
    u8 ram_size;
    u8 destination_code;
    u8 old_licensee_code;
    u8 version;
    u8 header_checksum;
    u16 global_checksum;
} CartHeader;

// Structure de cartouche
typedef struct {
    u8* rom_data;
    u32 rom_size;
    u8* ram_data;
    u32 ram_size;
    CartType type;
    CartHeader header;
    
    // MBC state
    u8 rom_bank;
    u8 ram_bank;
    bool ram_enabled;
    bool rom_banking_mode;  // true = ROM banking, false = RAM banking
} Cartridge;

// Structure MMU
typedef struct {
    u8* memory;
    u8* rom;
    u8* vram;
    u8* eram;
    u8* wram;
    u8* oam;
    u8* io;
    u8* hram;
    
    Cartridge cart;
    bool boot_rom_enabled;
} MMU;

// Fonctions MMU
void mmu_init(MMU* mmu);
void mmu_cleanup(MMU* mmu);
bool mmu_load_rom(MMU* mmu, const char* filename);
void mmu_reset(MMU* mmu);

// Accès mémoire
u8 mmu_read8(MMU* mmu, u16 address);
u16 mmu_read16(MMU* mmu, u16 address);
void mmu_write8(MMU* mmu, u16 address, u8 value);
void mmu_write16(MMU* mmu, u16 address, u16 value);

// Fonctions MBC
void mbc_write(MMU* mmu, u16 address, u8 value);
u8 mbc_read(MMU* mmu, u16 address);

// Parsing de cartouche
bool cart_parse_header(Cartridge* cart, u8* rom_data);
const char* cart_type_name(CartType type);

#endif // MMU_H

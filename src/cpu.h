#ifndef CPU_H
#define CPU_H

#include "common.h"
#include "mmu.h"

// Constantes des flags (selon Pan Docs - LR35902)
#define FLAG_Z 0x80  // Zero flag (bit 7) - Résultat = 0
#define FLAG_N 0x40  // Subtract flag (bit 6) - Dernière opération était une soustraction
#define FLAG_H 0x20  // Half carry flag (bit 5) - Carry du bit 3 vers bit 4
#define FLAG_C 0x10  // Carry flag (bit 4) - Carry ou borrow

// Constantes des interruptions (selon Pan Docs - registre IE/IF)
#define VBLANK_INT 0x01   // INT $40 - VBlank interrupt (priorité la plus haute)
#define LCD_STAT_INT 0x02 // INT $48 - STAT interrupt  
#define TIMER_INT 0x04    // INT $50 - Timer interrupt
#define SERIAL_INT 0x08   // INT $58 - Serial interrupt
#define JOYPAD_INT 0x10   // INT $60 - Joypad interrupt (priorité la plus basse)

// CPU LR35902 (Game Boy CPU - dérivé du Z80)
typedef struct {
    // Registres principaux 16-bit (accès 8-bit via get/set_reg_x)
    u16 af;  // A (accumulator) + F (flags) - A=bits 15-8, F=bits 7-0
    u16 bc;  // B + C registers - B=bits 15-8, C=bits 7-0  
    u16 de;  // D + E registers - D=bits 15-8, E=bits 7-0
    u16 hl;  // H + L registers - H=bits 15-8, L=bits 7-0
    
    // Registres spéciaux 16-bit
    u16 sp;  // Stack Pointer - pointe vers le haut de la pile
    u16 pc;  // Program Counter - adresse de la prochaine instruction
    
    // État selon Pan Docs
    bool halted;
    bool ime;  // Interrupt Master Enable
    bool ei_pending;  // EI prend effet après l'instruction suivante
    bool halt_bug;  // HALT bug : PC n'incrémente pas dans certaines conditions
    bool branch_taken; // Indique si la dernière condition a été prise (pour cycles)
} CPU;

// Structure d'instruction - représente un opcode Game Boy
typedef struct {
    const char* mnemonic;    // Nom assembleur (ex: "LD A,B")
    u8 length;               // Taille en octets (1, 2 ou 3)
    u8 cycles;               // Cycles d'horloge normaux (multiples de 4)
    u8 cycles_cond;          // Cycles si condition vraie (pour sauts/calls)
    void (*execute)(CPU* cpu, MMU* mmu); // Fonction d'exécution
} Instruction;

// Fonctions CPU
void cpu_init(CPU* cpu);
void cpu_reset(CPU* cpu);
u8 cpu_step(CPU* cpu, MMU* mmu);
void cpu_interrupt(CPU* cpu, MMU* mmu, u8 interrupt);

// Gestion des registres
u8 get_reg_a(CPU* cpu);
u8 get_reg_b(CPU* cpu);
u8 get_reg_c(CPU* cpu);
u8 get_reg_d(CPU* cpu);
u8 get_reg_e(CPU* cpu);
u8 get_reg_h(CPU* cpu);
u8 get_reg_l(CPU* cpu);
u8 get_reg_f(CPU* cpu);

void set_reg_a(CPU* cpu, u8 value);
void set_reg_b(CPU* cpu, u8 value);
void set_reg_c(CPU* cpu, u8 value);
void set_reg_d(CPU* cpu, u8 value);
void set_reg_e(CPU* cpu, u8 value);
void set_reg_h(CPU* cpu, u8 value);
void set_reg_l(CPU* cpu, u8 value);
void set_reg_f(CPU* cpu, u8 value);

// Gestion des flags
void set_flags(CPU* cpu, u8 flags);
u8 get_flags(CPU* cpu);
bool get_flag(CPU* cpu, u8 flag);
void set_flag(CPU* cpu, u8 flag, bool value);

// Instructions de base
void inst_nop(CPU* cpu, MMU* mmu);
void inst_halt(CPU* cpu, MMU* mmu);
void inst_cb_prefix(CPU* cpu, MMU* mmu);
void inst_stop(CPU* cpu, MMU* mmu);
void inst_illegal(CPU* cpu, MMU* mmu);
void inst_di(CPU* cpu, MMU* mmu);
void inst_ei(CPU* cpu, MMU* mmu);

// Instructions de chargement
void inst_ld_r8_r8(CPU* cpu, MMU* mmu);
void inst_ld_r8_n8(CPU* cpu, MMU* mmu);
void inst_ld_r16_n16(CPU* cpu, MMU* mmu);
void inst_ld_a_hl(CPU* cpu, MMU* mmu);
void inst_ld_hl_a(CPU* cpu, MMU* mmu);
void inst_ld_a_bc(CPU* cpu, MMU* mmu);
void inst_ld_a_de(CPU* cpu, MMU* mmu);
void inst_ld_bc_a(CPU* cpu, MMU* mmu);
void inst_ld_de_a(CPU* cpu, MMU* mmu);
void inst_ld_hl_plus_a(CPU* cpu, MMU* mmu);
void inst_ld_hl_minus_a(CPU* cpu, MMU* mmu);
void inst_ld_a_hl_plus(CPU* cpu, MMU* mmu);
void inst_ld_a_hl_minus(CPU* cpu, MMU* mmu);
void inst_ld_sp_n16(CPU* cpu, MMU* mmu);
void inst_ld_hl_sp_e8(CPU* cpu, MMU* mmu);
void inst_ld_sp_hl(CPU* cpu, MMU* mmu);
void inst_ld_nn_sp(CPU* cpu, MMU* mmu);

// Instructions arithmétiques
void inst_inc_r8(CPU* cpu, MMU* mmu);
void inst_dec_r8(CPU* cpu, MMU* mmu);
void inst_inc_r16(CPU* cpu, MMU* mmu);
void inst_dec_r16(CPU* cpu, MMU* mmu);
void inst_add_a_r8(CPU* cpu, MMU* mmu);
void inst_add_a_n8(CPU* cpu, MMU* mmu);
void inst_add_a_hl(CPU* cpu, MMU* mmu);
void inst_adc_a_r8(CPU* cpu, MMU* mmu);
void inst_adc_a_n8(CPU* cpu, MMU* mmu);
void inst_adc_a_hl(CPU* cpu, MMU* mmu);
void inst_sub_a_r8(CPU* cpu, MMU* mmu);
void inst_sub_a_n8(CPU* cpu, MMU* mmu);
void inst_sub_a_hl(CPU* cpu, MMU* mmu);
void inst_sbc_a_r8(CPU* cpu, MMU* mmu);
void inst_sbc_a_n8(CPU* cpu, MMU* mmu);
void inst_sbc_a_hl(CPU* cpu, MMU* mmu);
void inst_and_a_r8(CPU* cpu, MMU* mmu);
void inst_and_a_n8(CPU* cpu, MMU* mmu);
void inst_and_a_hl(CPU* cpu, MMU* mmu);
void inst_xor_a_r8(CPU* cpu, MMU* mmu);
void inst_xor_a_n8(CPU* cpu, MMU* mmu);
void inst_xor_a_hl(CPU* cpu, MMU* mmu);
void inst_or_a_r8(CPU* cpu, MMU* mmu);
void inst_or_a_n8(CPU* cpu, MMU* mmu);
void inst_or_a_hl(CPU* cpu, MMU* mmu);
void inst_cp_a_r8(CPU* cpu, MMU* mmu);
void inst_cp_a_n8(CPU* cpu, MMU* mmu);
void inst_cp_a_hl(CPU* cpu, MMU* mmu);
void inst_add_hl_r16(CPU* cpu, MMU* mmu);
void inst_add_hl_sp(CPU* cpu, MMU* mmu);
void inst_add_sp_e8(CPU* cpu, MMU* mmu);

// Instructions de saut
void inst_jp_n16(CPU* cpu, MMU* mmu);
void inst_jp_hl(CPU* cpu, MMU* mmu);
void inst_jr_e8(CPU* cpu, MMU* mmu);
void inst_jr_nz_e8(CPU* cpu, MMU* mmu);
void inst_jr_z_e8(CPU* cpu, MMU* mmu);
void inst_jr_nc_e8(CPU* cpu, MMU* mmu);
void inst_jr_c_e8(CPU* cpu, MMU* mmu);
void inst_jp_nz_n16(CPU* cpu, MMU* mmu);
void inst_jp_z_n16(CPU* cpu, MMU* mmu);
void inst_jp_nc_n16(CPU* cpu, MMU* mmu);
void inst_jp_c_n16(CPU* cpu, MMU* mmu);
void inst_call_n16(CPU* cpu, MMU* mmu);
void inst_call_nz_n16(CPU* cpu, MMU* mmu);
void inst_call_z_n16(CPU* cpu, MMU* mmu);
void inst_call_nc_n16(CPU* cpu, MMU* mmu);
void inst_call_c_n16(CPU* cpu, MMU* mmu);
void inst_ret(CPU* cpu, MMU* mmu);
void inst_ret_nz(CPU* cpu, MMU* mmu);
void inst_ret_z(CPU* cpu, MMU* mmu);
void inst_ret_nc(CPU* cpu, MMU* mmu);
void inst_ret_c(CPU* cpu, MMU* mmu);
void inst_reti(CPU* cpu, MMU* mmu);
void inst_rst_00h(CPU* cpu, MMU* mmu);
void inst_rst_08h(CPU* cpu, MMU* mmu);
void inst_rst_10h(CPU* cpu, MMU* mmu);
void inst_rst_18h(CPU* cpu, MMU* mmu);
void inst_rst_20h(CPU* cpu, MMU* mmu);
void inst_rst_28h(CPU* cpu, MMU* mmu);
void inst_rst_30h(CPU* cpu, MMU* mmu);
void inst_rst_38h(CPU* cpu, MMU* mmu);

// Instructions de pile
void inst_push_af(CPU* cpu, MMU* mmu);
void inst_push_bc(CPU* cpu, MMU* mmu);
void inst_push_de(CPU* cpu, MMU* mmu);
void inst_push_hl(CPU* cpu, MMU* mmu);
void inst_pop_af(CPU* cpu, MMU* mmu);
void inst_pop_bc(CPU* cpu, MMU* mmu);
void inst_pop_de(CPU* cpu, MMU* mmu);
void inst_pop_hl(CPU* cpu, MMU* mmu);

// Instructions de rotation et décalage
void inst_rlca(CPU* cpu, MMU* mmu);
void inst_rla(CPU* cpu, MMU* mmu);
void inst_rrca(CPU* cpu, MMU* mmu);
void inst_rra(CPU* cpu, MMU* mmu);
void inst_rlc_r8(CPU* cpu, MMU* mmu);
void inst_rl_r8(CPU* cpu, MMU* mmu);
void inst_rrc_r8(CPU* cpu, MMU* mmu);
void inst_rr_r8(CPU* cpu, MMU* mmu);
void inst_sla_r8(CPU* cpu, MMU* mmu);
void inst_sra_r8(CPU* cpu, MMU* mmu);
void inst_srl_r8(CPU* cpu, MMU* mmu);
void inst_swap_r8(CPU* cpu, MMU* mmu);

// Instructions de bits
void inst_bit_0_r8(CPU* cpu, MMU* mmu);
void inst_bit_1_r8(CPU* cpu, MMU* mmu);
void inst_bit_2_r8(CPU* cpu, MMU* mmu);
void inst_bit_3_r8(CPU* cpu, MMU* mmu);
void inst_bit_4_r8(CPU* cpu, MMU* mmu);
void inst_bit_5_r8(CPU* cpu, MMU* mmu);
void inst_bit_6_r8(CPU* cpu, MMU* mmu);
void inst_bit_7_r8(CPU* cpu, MMU* mmu);
void inst_bit_0_hl(CPU* cpu, MMU* mmu);
void inst_bit_1_hl(CPU* cpu, MMU* mmu);
void inst_bit_2_hl(CPU* cpu, MMU* mmu);
void inst_bit_3_hl(CPU* cpu, MMU* mmu);
void inst_bit_4_hl(CPU* cpu, MMU* mmu);
void inst_bit_5_hl(CPU* cpu, MMU* mmu);
void inst_bit_6_hl(CPU* cpu, MMU* mmu);
void inst_bit_7_hl(CPU* cpu, MMU* mmu);
void inst_set_0_r8(CPU* cpu, MMU* mmu);
void inst_set_1_r8(CPU* cpu, MMU* mmu);
void inst_set_2_r8(CPU* cpu, MMU* mmu);
void inst_set_3_r8(CPU* cpu, MMU* mmu);
void inst_set_4_r8(CPU* cpu, MMU* mmu);
void inst_set_5_r8(CPU* cpu, MMU* mmu);
void inst_set_6_r8(CPU* cpu, MMU* mmu);
void inst_set_7_r8(CPU* cpu, MMU* mmu);
void inst_set_0_hl(CPU* cpu, MMU* mmu);
void inst_set_1_hl(CPU* cpu, MMU* mmu);
void inst_set_2_hl(CPU* cpu, MMU* mmu);
void inst_set_3_hl(CPU* cpu, MMU* mmu);
void inst_set_4_hl(CPU* cpu, MMU* mmu);
void inst_set_5_hl(CPU* cpu, MMU* mmu);
void inst_set_6_hl(CPU* cpu, MMU* mmu);
void inst_set_7_hl(CPU* cpu, MMU* mmu);
void inst_res_0_r8(CPU* cpu, MMU* mmu);
void inst_res_1_r8(CPU* cpu, MMU* mmu);
void inst_res_2_r8(CPU* cpu, MMU* mmu);
void inst_res_3_r8(CPU* cpu, MMU* mmu);
void inst_res_4_r8(CPU* cpu, MMU* mmu);
void inst_res_5_r8(CPU* cpu, MMU* mmu);
void inst_res_6_r8(CPU* cpu, MMU* mmu);
void inst_res_7_r8(CPU* cpu, MMU* mmu);
void inst_res_0_hl(CPU* cpu, MMU* mmu);
void inst_res_1_hl(CPU* cpu, MMU* mmu);
void inst_res_2_hl(CPU* cpu, MMU* mmu);
void inst_res_3_hl(CPU* cpu, MMU* mmu);
void inst_res_4_hl(CPU* cpu, MMU* mmu);
void inst_res_5_hl(CPU* cpu, MMU* mmu);
void inst_res_6_hl(CPU* cpu, MMU* mmu);
void inst_res_7_hl(CPU* cpu, MMU* mmu);

// Instructions spéciales
void inst_daa(CPU* cpu, MMU* mmu);
void inst_cpl(CPU* cpu, MMU* mmu);
void inst_scf(CPU* cpu, MMU* mmu);
void inst_ccf(CPU* cpu, MMU* mmu);

// Instructions LDH (High memory access)
void inst_ldh_a_imm8(CPU* cpu, MMU* mmu);
void inst_ldh_imm8_a(CPU* cpu, MMU* mmu);
void inst_ldh_a_c(CPU* cpu, MMU* mmu);
void inst_ldh_c_a(CPU* cpu, MMU* mmu);

// Instructions LD avec adresses 16-bit
void inst_ld_a_nn(CPU* cpu, MMU* mmu);
void inst_ld_nn_a(CPU* cpu, MMU* mmu);

// Tables d'instructions
extern const Instruction opcodes[256];
extern const Instruction opcodes_cb[256];

#endif // CPU_H

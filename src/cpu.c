// ============================================================================
// CPU GAME BOY LR35902 - IMPLÉMENTATION COMPLÈTE ET UNIFIÉE
// ============================================================================
// 
// Émulateur Game Boy conforme aux spécifications Pan Docs
// CPU Sharp LR35902 (dérivé du Z80)
//
// ============================================================================

#include "mmu.h"
#include "cpu.h"
#include "common.h"
#include <stdio.h>

// ============================================================================
// FONCTIONS UTILITAIRES - GESTION DES REGISTRES
// ============================================================================

// Accesseurs registres 8-bit (depuis registres 16-bit)
u8 get_reg_b(CPU* cpu) { return (cpu->bc >> 8) & 0xFF; }
u8 get_reg_c(CPU* cpu) { return cpu->bc & 0xFF; }
u8 get_reg_d(CPU* cpu) { return (cpu->de >> 8) & 0xFF; }
u8 get_reg_e(CPU* cpu) { return cpu->de & 0xFF; }
u8 get_reg_h(CPU* cpu) { return (cpu->hl >> 8) & 0xFF; }
u8 get_reg_l(CPU* cpu) { return cpu->hl & 0xFF; }
u8 get_reg_a(CPU* cpu) { return (cpu->af >> 8) & 0xFF; }
u8 get_reg_f(CPU* cpu) { return cpu->af & 0xFF; }

// Modificateurs registres 8-bit
void set_reg_b(CPU* cpu, u8 value) { cpu->bc = (cpu->bc & 0x00FF) | (value << 8); }
void set_reg_c(CPU* cpu, u8 value) { cpu->bc = (cpu->bc & 0xFF00) | value; }
void set_reg_d(CPU* cpu, u8 value) { cpu->de = (cpu->de & 0x00FF) | (value << 8); }
void set_reg_e(CPU* cpu, u8 value) { cpu->de = (cpu->de & 0xFF00) | value; }
void set_reg_h(CPU* cpu, u8 value) { cpu->hl = (cpu->hl & 0x00FF) | (value << 8); }
void set_reg_l(CPU* cpu, u8 value) { cpu->hl = (cpu->hl & 0xFF00) | value; }
void set_reg_a(CPU* cpu, u8 value) { cpu->af = (cpu->af & 0x00FF) | (value << 8); }
void set_reg_f(CPU* cpu, u8 value) { cpu->af = (cpu->af & 0xFF00) | value; }

// ============================================================================
// FONCTIONS UTILITAIRES - GESTION DES FLAGS
// ============================================================================

u8 get_flags(CPU* cpu) {
    return get_reg_f(cpu);
}

void set_flags(CPU* cpu, u8 flags) {
    set_reg_f(cpu, flags);
}

bool get_flag(CPU* cpu, u8 flag) {
    return (cpu->af & flag) != 0;
}

void set_flag(CPU* cpu, u8 flag, bool value) {
    if (value) {
        cpu->af |= flag;
    } else {
        cpu->af &= ~flag;
    }
}

// ============================================================================
// INSTRUCTIONS DE BASE (CONTROL FLOW)
// ============================================================================

void inst_nop(CPU* cpu, MMU* mmu) {
    (void)mmu;  // Paramètre non utilisé
    cpu->pc += 1;
}

void inst_halt(CPU* cpu, MMU* mmu) {
    (void)mmu;  // Paramètre non utilisé
    cpu->halted = true;
    cpu->pc += 1;
}

void inst_stop(CPU* cpu, MMU* mmu) {
    (void)mmu;  // Paramètre non utilisé
    cpu->halted = true;  // STOP équivalent à HALT pour simplifier
    cpu->pc += 2;  // STOP est sur 2 octets
}

void inst_di(CPU* cpu, MMU* mmu) {
    (void)mmu;  // Paramètre non utilisé
    cpu->ime = false;
    cpu->ei_pending = false;
    cpu->pc += 1;
}

void inst_ei(CPU* cpu, MMU* mmu) {
    (void)mmu;  // Paramètre non utilisé
    cpu->ei_pending = true;  // IME activé après l'instruction suivante
    cpu->pc += 1;
}

// ============================================================================
// INSTRUCTIONS DE CHARGEMENT (LOAD/STORE)
// ============================================================================

void inst_ld_r8_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 dst = (opcode >> 3) & 0x07;
    u8 src = opcode & 0x07;
    u8 value = 0;
    
    // Lire la valeur source
    switch (src) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    // Écrire dans la destination
    switch (dst) {
        case 0: set_reg_b(cpu, value); break;
        case 1: set_reg_c(cpu, value); break;
        case 2: set_reg_d(cpu, value); break;
        case 3: set_reg_e(cpu, value); break;
        case 4: set_reg_h(cpu, value); break;
        case 5: set_reg_l(cpu, value); break;
        case 6: mmu->memory[cpu->hl] = value; break;  // (HL)
        case 7: set_reg_a(cpu, value); break;
    }
    
    cpu->pc += 1;
}

void inst_ld_r8_n8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = (opcode >> 3) & 0x07;
    u8 value = mmu->memory[cpu->pc + 1];
    
    switch (reg) {
        case 0: set_reg_b(cpu, value); break;
        case 1: set_reg_c(cpu, value); break;
        case 2: set_reg_d(cpu, value); break;
        case 3: set_reg_e(cpu, value); break;
        case 4: set_reg_h(cpu, value); break;
        case 5: set_reg_l(cpu, value); break;
        case 6: mmu->memory[cpu->hl] = value; break;  // (HL)
        case 7: set_reg_a(cpu, value); break;
    }
    
    cpu->pc += 2;
}

void inst_ld_r16_n16(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = (opcode >> 4) & 0x03;
    u16 value = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
    
    switch (reg) {
        case 0: cpu->bc = value; break;
        case 1: cpu->de = value; break;
        case 2: cpu->hl = value; break;
        case 3: cpu->sp = value; break;
    }
    
    cpu->pc += 3;
}

void inst_ld_a_hl(CPU* cpu, MMU* mmu) {
    set_reg_a(cpu, mmu->memory[cpu->hl]);
    cpu->pc += 1;
}

void inst_ld_hl_a(CPU* cpu, MMU* mmu) {
    mmu->memory[cpu->hl] = get_reg_a(cpu);
    cpu->pc += 1;
}

void inst_ld_a_bc(CPU* cpu, MMU* mmu) {
    set_reg_a(cpu, mmu->memory[cpu->bc]);
    cpu->pc += 1;
}

void inst_ld_a_de(CPU* cpu, MMU* mmu) {
    set_reg_a(cpu, mmu->memory[cpu->de]);
    cpu->pc += 1;
}

void inst_ld_bc_a(CPU* cpu, MMU* mmu) {
    mmu->memory[cpu->bc] = get_reg_a(cpu);
    cpu->pc += 1;
}

void inst_ld_de_a(CPU* cpu, MMU* mmu) {
    mmu->memory[cpu->de] = get_reg_a(cpu);
    cpu->pc += 1;
}

void inst_ld_hl_plus_a(CPU* cpu, MMU* mmu) {
    mmu->memory[cpu->hl] = get_reg_a(cpu);
    cpu->hl++;
    cpu->pc += 1;
}

void inst_ld_hl_minus_a(CPU* cpu, MMU* mmu) {
    mmu->memory[cpu->hl] = get_reg_a(cpu);
    cpu->hl--;
    cpu->pc += 1;
}

void inst_ld_a_hl_plus(CPU* cpu, MMU* mmu) {
    set_reg_a(cpu, mmu->memory[cpu->hl]);
    cpu->hl++;
    cpu->pc += 1;
}

void inst_ld_a_hl_minus(CPU* cpu, MMU* mmu) {
    set_reg_a(cpu, mmu->memory[cpu->hl]);
    cpu->hl--;
    cpu->pc += 1;
}

void inst_ld_sp_n16(CPU* cpu, MMU* mmu) {
    cpu->sp = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
    cpu->pc += 3;
}

void inst_ld_sp_hl(CPU* cpu, MMU* mmu) {
    (void)mmu;
    cpu->sp = cpu->hl;
    cpu->pc += 1;
}

void inst_ld_nn_sp(CPU* cpu, MMU* mmu) {
    u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
    mmu->memory[addr] = cpu->sp & 0xFF;
    mmu->memory[addr + 1] = (cpu->sp >> 8) & 0xFF;
    cpu->pc += 3;
}

// ============================================================================
// INSTRUCTIONS ARITHMÉTIQUES - ADD/ADC
// ============================================================================

void inst_add_a_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    u8 a = get_reg_a(cpu);
    u16 result = a + value;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, (a & 0x0F) + (value & 0x0F) > 0x0F);
    set_flag(cpu, FLAG_C, result > 0xFF);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 1;
}

void inst_add_a_n8(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->pc + 1];
    u8 a = get_reg_a(cpu);
    u16 result = a + value;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, (a & 0x0F) + (value & 0x0F) > 0x0F);
    set_flag(cpu, FLAG_C, result > 0xFF);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 2;
}

void inst_add_a_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    u8 a = get_reg_a(cpu);
    u16 result = a + value;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, (a & 0x0F) + (value & 0x0F) > 0x0F);
    set_flag(cpu, FLAG_C, result > 0xFF);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 1;
}

// ============================================================================
// INSTRUCTIONS DE SAUT (JUMP/CALL/RETURN)
// ============================================================================

void inst_jp_n16(CPU* cpu, MMU* mmu) {
    u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
    cpu->pc = addr;
}

void inst_jp_hl(CPU* cpu, MMU* mmu) {
    (void)mmu;  // Paramètre non utilisé
    cpu->pc = cpu->hl;
}

void inst_jr_e8(CPU* cpu, MMU* mmu) {
    s8 offset = (s8)mmu->memory[cpu->pc + 1];
    cpu->pc += 2;  // Avancer d'abord
    cpu->pc += offset;  // Puis appliquer l'offset
}

void inst_call_n16(CPU* cpu, MMU* mmu) {
    u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
    
    // Avancer PC avant de le sauvegarder
    cpu->pc += 3;
    
    // Push PC sur la pile
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = cpu->pc & 0xFF;
    mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
    
    // Saut vers l'adresse
    cpu->pc = addr;
}

void inst_ret(CPU* cpu, MMU* mmu) {
    // Pop PC depuis la pile
    cpu->pc = mmu->memory[cpu->sp] | (mmu->memory[cpu->sp + 1] << 8);
    cpu->sp += 2;
}

// ============================================================================
// INSTRUCTIONS DE PILE (PUSH/POP)
// ============================================================================

void inst_push_bc(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = get_reg_c(cpu);
    mmu->memory[cpu->sp + 1] = get_reg_b(cpu);
    cpu->pc += 1;
}

void inst_push_de(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = get_reg_e(cpu);
    mmu->memory[cpu->sp + 1] = get_reg_d(cpu);
    cpu->pc += 1;
}

void inst_push_hl(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = get_reg_l(cpu);
    mmu->memory[cpu->sp + 1] = get_reg_h(cpu);
    cpu->pc += 1;
}

void inst_push_af(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = get_reg_f(cpu);
    mmu->memory[cpu->sp + 1] = get_reg_a(cpu);
    cpu->pc += 1;
}

void inst_pop_bc(CPU* cpu, MMU* mmu) {
    set_reg_c(cpu, mmu->memory[cpu->sp]);
    set_reg_b(cpu, mmu->memory[cpu->sp + 1]);
    cpu->sp += 2;
    cpu->pc += 1;
}

void inst_pop_de(CPU* cpu, MMU* mmu) {
    set_reg_e(cpu, mmu->memory[cpu->sp]);
    set_reg_d(cpu, mmu->memory[cpu->sp + 1]);
    cpu->sp += 2;
    cpu->pc += 1;
}

void inst_pop_hl(CPU* cpu, MMU* mmu) {
    set_reg_l(cpu, mmu->memory[cpu->sp]);
    set_reg_h(cpu, mmu->memory[cpu->sp + 1]);
    cpu->sp += 2;
    cpu->pc += 1;
}

void inst_pop_af(CPU* cpu, MMU* mmu) {
    set_reg_f(cpu, mmu->memory[cpu->sp]);
    set_reg_a(cpu, mmu->memory[cpu->sp + 1]);
    cpu->sp += 2;
    cpu->pc += 1;
}

// ============================================================================
// SAUTS CONDITIONNELS
// ============================================================================

void inst_jr_nz_e8(CPU* cpu, MMU* mmu) {
    s8 offset = (s8)mmu->memory[cpu->pc + 1];
    
    printf("JR NZ: offset=%d, Z=%d, PC avant=0x%04X", offset, get_flag(cpu, FLAG_Z), cpu->pc);
    
    if (!get_flag(cpu, FLAG_Z)) {
        cpu->pc += 2 + offset;
        cpu->branch_taken = true;
        printf(" -> SAUT vers 0x%04X\n", cpu->pc);
    } else {
        cpu->pc += 2;
        cpu->branch_taken = false;
        printf(" -> PAS DE SAUT, PC=0x%04X\n", cpu->pc);
    }
}

void inst_jr_z_e8(CPU* cpu, MMU* mmu) {
    s8 offset = (s8)mmu->memory[cpu->pc + 1];
    
    if (get_flag(cpu, FLAG_Z)) {
        cpu->pc += 2 + offset;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 2;
        cpu->branch_taken = false;
    }
}

void inst_jr_nc_e8(CPU* cpu, MMU* mmu) {
    s8 offset = (s8)mmu->memory[cpu->pc + 1];
    
    if (!get_flag(cpu, FLAG_C)) {
        cpu->pc += 2 + offset;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 2;
        cpu->branch_taken = false;
    }
}

void inst_jr_c_e8(CPU* cpu, MMU* mmu) {
    s8 offset = (s8)mmu->memory[cpu->pc + 1];
    
    if (get_flag(cpu, FLAG_C)) {
        cpu->pc += 2 + offset;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 2;
        cpu->branch_taken = false;
    }
}

// ============================================================================
// INSTRUCTIONS SPÉCIALES - COMPARAISON
// ============================================================================

void inst_cp_a_n8(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->pc + 1];
    u8 a = get_reg_a(cpu);
    
    u16 result = a - value;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, true);
    set_flag(cpu, FLAG_H, (a & 0x0F) < (value & 0x0F));
    set_flag(cpu, FLAG_C, a < value);
    
    cpu->pc += 2;
}

// ============================================================================
// GESTION DES INTERRUPTIONS
// ============================================================================

void cpu_interrupt(CPU* cpu, MMU* mmu, u8 interrupt) {
    if (!cpu->ime) return;
    
    // Désactiver les interruptions
    cpu->ime = false;
    cpu->halted = false;  // Sortir de HALT
    
    // Push PC sur la pile
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = cpu->pc & 0xFF;
    mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
    
    // Saut vers le vecteur d'interruption
    switch (interrupt) {
        case VBLANK_INT:  cpu->pc = 0x40; break;
        case LCD_STAT_INT: cpu->pc = 0x48; break;
        case TIMER_INT:    cpu->pc = 0x50; break;
        case SERIAL_INT:   cpu->pc = 0x58; break;
        case JOYPAD_INT:   cpu->pc = 0x60; break;
        default: cpu->pc = 0x00; break;  // Fallback
    }
}

void inst_reti(CPU* cpu, MMU* mmu) {
    // Équivalent à RET + EI
    inst_ret(cpu, mmu);
    cpu->ime = true;  // Réactiver immédiatement les interruptions
}

// ============================================================================
// INSTRUCTIONS LDH (HIGH MEMORY ACCESS) - POUR PORT SÉRIE
// ============================================================================

void inst_ldh_imm8_a(CPU* cpu, MMU* mmu) {
    u8 offset = mmu->memory[cpu->pc + 1];
    mmu_write8(mmu, 0xFF00 + offset, get_reg_a(cpu));  // Utilise mmu_write8 !
    cpu->pc += 2;
}

void inst_ldh_a_imm8(CPU* cpu, MMU* mmu) {
    u8 offset = mmu->memory[cpu->pc + 1];
    set_reg_a(cpu, mmu_read8(mmu, 0xFF00 + offset));  // Utilise mmu_read8 !
    cpu->pc += 2;
}

void inst_ldh_c_a(CPU* cpu, MMU* mmu) {
    mmu_write8(mmu, 0xFF00 + get_reg_c(cpu), get_reg_a(cpu));
    cpu->pc += 1;
}

void inst_ldh_a_c(CPU* cpu, MMU* mmu) {
    set_reg_a(cpu, mmu_read8(mmu, 0xFF00 + get_reg_c(cpu)));
    cpu->pc += 1;
}

// ============================================================================
// GESTION CB-PREFIX (INSTRUCTIONS ÉTENDUES)
// ============================================================================

void inst_cb_prefix(CPU* cpu, MMU* mmu) {
    u8 cb_opcode = mmu->memory[cpu->pc + 1];
    
    // Tables déclarées dans cpu_tables_cb.c
    extern const Instruction opcodes_cb[256];
    const Instruction* cb_inst = &opcodes_cb[cb_opcode];
    
    if (cb_inst->execute == NULL) {
        printf("CB Opcode non implémenté: 0xCB 0x%02X à PC=0x%04X\n", 
               cb_opcode, cpu->pc);
        cpu->pc += 2; // Avancer quand même
        return;
    }
    
    // Exécuter l'instruction CB
    cb_inst->execute(cpu, mmu);
}

// ============================================================================
// BOUCLE PRINCIPALE D'EXÉCUTION
// ============================================================================

u8 cpu_step(CPU* cpu, MMU* mmu) {
    
    if (cpu->halted) {
        return 4;  // Cycle de HALT
    }
    
    // Gestion du délai EI (prend effet après l'instruction suivante)
    if (cpu->ei_pending) {
        cpu->ei_pending = false;
        cpu->ime = true;
    }
    
    u8 opcode = mmu->memory[cpu->pc];
    const Instruction* inst = &opcodes[opcode];
    
    if (inst->execute == NULL) {
        printf("Opcode non implémenté: 0x%02X à PC=0x%04X\n", opcode, cpu->pc);
        cpu->pc += 1;
        return 4;
    }
    
    // Exécuter l'instruction
    inst->execute(cpu, mmu);
    
    // Retourner les cycles (avec cycles conditionnels si applicable)
    return cpu->branch_taken ? inst->cycles_cond : inst->cycles;
}

// ============================================================================
// FONCTIONS D'INITIALISATION
// ============================================================================

void cpu_init(CPU* cpu) {
    // Valeurs d'initialisation Game Boy (après boot ROM)
    cpu->af = 0x01B0;  // A=0x01, F=0xB0 (Z=1, N=0, H=1, C=1)
    cpu->bc = 0x0013;
    cpu->de = 0x00D8;
    cpu->hl = 0x014D;
    cpu->sp = 0xFFFE;  // Stack pointer au sommet de HRAM
    cpu->pc = 0x0100;  // Point d'entrée standard après boot ROM
    
    // État initial selon Pan Docs
    cpu->halted = false;
    cpu->ime = true;            // Interruptions activées pour les tests
    cpu->ei_pending = false;    // Pas de EI en attente
    cpu->halt_bug = false;      // Pas de HALT bug actif
    cpu->branch_taken = false;  // Pas de saut conditionnel pris
}

void cpu_reset(CPU* cpu) {
    // Reset à l'état initial (équivalent à cpu_init)
    cpu_init(cpu);
}

// ============================================================================
// INSTRUCTIONS ARITHMÉTIQUES - ADC/SUB/SBC/AND/XOR/OR/CP
// ============================================================================

void inst_adc_a_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    u8 a = get_reg_a(cpu);
    u8 carry = get_flag(cpu, FLAG_C) ? 1 : 0;
    u16 result = a + value + carry;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, (a & 0x0F) + (value & 0x0F) + carry > 0x0F);
    set_flag(cpu, FLAG_C, result > 0xFF);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 1;
}

void inst_adc_a_hl(CPU* cpu, MMU* mmu) {
    u8 a = get_reg_a(cpu);
    u8 value = mmu->memory[cpu->hl];
    u8 carry = get_flag(cpu, FLAG_C) ? 1 : 0;
    u16 result = a + value + carry;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, (a & 0x0F) + (value & 0x0F) + carry > 0x0F);
    set_flag(cpu, FLAG_C, result > 0xFF);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 1;
}

void inst_sub_a_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    u8 a = get_reg_a(cpu);
    u16 result = a - value;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, true);
    set_flag(cpu, FLAG_H, (a & 0x0F) < (value & 0x0F));
    set_flag(cpu, FLAG_C, a < value);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 1;
}

void inst_sub_a_hl(CPU* cpu, MMU* mmu) {
    u8 a = get_reg_a(cpu);
    u8 value = mmu->memory[cpu->hl];
    u16 result = a - value;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, true);
    set_flag(cpu, FLAG_H, (a & 0x0F) < (value & 0x0F));
    set_flag(cpu, FLAG_C, a < value);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 1;
}

void inst_sbc_a_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    u8 a = get_reg_a(cpu);
    u8 carry = get_flag(cpu, FLAG_C) ? 1 : 0;
    u16 result = a - value - carry;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, true);
    set_flag(cpu, FLAG_H, (a & 0x0F) < (value & 0x0F) + carry);
    set_flag(cpu, FLAG_C, a < value + carry);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 1;
}

void inst_sbc_a_hl(CPU* cpu, MMU* mmu) {
    u8 a = get_reg_a(cpu);
    u8 value = mmu->memory[cpu->hl];
    u8 carry = get_flag(cpu, FLAG_C) ? 1 : 0;
    u16 result = a - value - carry;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, true);
    set_flag(cpu, FLAG_H, (a & 0x0F) < (value & 0x0F) + carry);
    set_flag(cpu, FLAG_C, a < value + carry);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 1;
}

void inst_and_a_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    u8 result = get_reg_a(cpu) & value;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);  // AND met toujours H=1
    set_flag(cpu, FLAG_C, false);
    
    set_reg_a(cpu, result);
    cpu->pc += 1;
}

void inst_and_a_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    u8 result = get_reg_a(cpu) & value;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    set_flag(cpu, FLAG_C, false);
    
    set_reg_a(cpu, result);
    cpu->pc += 1;
}

void inst_xor_a_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    u8 result = get_reg_a(cpu) ^ value;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, false);
    
    set_reg_a(cpu, result);
    cpu->pc += 1;
}

void inst_xor_a_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    u8 result = get_reg_a(cpu) ^ value;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, false);
    
    set_reg_a(cpu, result);
    cpu->pc += 1;
}

void inst_or_a_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    u8 result = get_reg_a(cpu) | value;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, false);
    
    set_reg_a(cpu, result);
    cpu->pc += 1;
}

void inst_or_a_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    u8 result = get_reg_a(cpu) | value;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, false);
    
    set_reg_a(cpu, result);
    cpu->pc += 1;
}

void inst_cp_a_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    u8 a = get_reg_a(cpu);
    u16 result = a - value;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, true);
    set_flag(cpu, FLAG_H, (a & 0x0F) < (value & 0x0F));
    set_flag(cpu, FLAG_C, a < value);
    
    // CP ne modifie PAS le registre A
    cpu->pc += 1;
}

void inst_cp_a_hl(CPU* cpu, MMU* mmu) {
    u8 a = get_reg_a(cpu);
    u8 value = mmu->memory[cpu->hl];
    u16 result = a - value;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, true);
    set_flag(cpu, FLAG_H, (a & 0x0F) < (value & 0x0F));
    set_flag(cpu, FLAG_C, a < value);
    
    cpu->pc += 1;
}

void inst_adc_a_n8(CPU* cpu, MMU* mmu) {
    u8 a = get_reg_a(cpu);
    u8 value = mmu->memory[cpu->pc + 1];
    u8 carry = get_flag(cpu, FLAG_C) ? 1 : 0;
    u16 result = a + value + carry;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, (a & 0x0F) + (value & 0x0F) + carry > 0x0F);
    set_flag(cpu, FLAG_C, result > 0xFF);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 2;
}

void inst_sub_a_n8(CPU* cpu, MMU* mmu) {
    u8 a = get_reg_a(cpu);
    u8 value = mmu->memory[cpu->pc + 1];
    u16 result = a - value;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, true);
    set_flag(cpu, FLAG_H, (a & 0x0F) < (value & 0x0F));
    set_flag(cpu, FLAG_C, a < value);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 2;
}

void inst_sbc_a_n8(CPU* cpu, MMU* mmu) {
    u8 a = get_reg_a(cpu);
    u8 value = mmu->memory[cpu->pc + 1];
    u8 carry = get_flag(cpu, FLAG_C) ? 1 : 0;
    u16 result = a - value - carry;
    
    set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
    set_flag(cpu, FLAG_N, true);
    set_flag(cpu, FLAG_H, (a & 0x0F) < (value & 0x0F) + carry);
    set_flag(cpu, FLAG_C, a < value + carry);
    
    set_reg_a(cpu, result & 0xFF);
    cpu->pc += 2;
}

void inst_and_a_n8(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->pc + 1];
    u8 result = get_reg_a(cpu) & value;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    set_flag(cpu, FLAG_C, false);
    
    set_reg_a(cpu, result);
    cpu->pc += 2;
}

void inst_xor_a_n8(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->pc + 1];
    u8 result = get_reg_a(cpu) ^ value;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, false);
    
    set_reg_a(cpu, result);
    cpu->pc += 2;
}

void inst_or_a_n8(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->pc + 1];
    u8 result = get_reg_a(cpu) | value;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, false);
    
    set_reg_a(cpu, result);
    cpu->pc += 2;
}

void inst_add_hl_sp(CPU* cpu, MMU* mmu) {
    (void)mmu;
    u32 result = cpu->hl + cpu->sp;
    
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, (cpu->hl & 0x0FFF) + (cpu->sp & 0x0FFF) > 0x0FFF);
    set_flag(cpu, FLAG_C, result > 0xFFFF);
    
    cpu->hl = result & 0xFFFF;
    cpu->pc += 1;
}

void inst_add_sp_e8(CPU* cpu, MMU* mmu) {
    s8 offset = (s8)mmu->memory[cpu->pc + 1];
    u32 result = cpu->sp + offset;
    
    set_flag(cpu, FLAG_Z, false);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, (cpu->sp & 0x0F) + (offset & 0x0F) > 0x0F);
    set_flag(cpu, FLAG_C, (cpu->sp & 0xFF) + (offset & 0xFF) > 0xFF);
    
    cpu->sp = result & 0xFFFF;
    cpu->pc += 2;
}

// ============================================================================
// INSTRUCTIONS DE SAUT CONDITIONNELLES
// ============================================================================

void inst_jp_nz_n16(CPU* cpu, MMU* mmu) {
    if (!get_flag(cpu, FLAG_Z)) {
        u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 3;
        cpu->branch_taken = false;
    }
}

void inst_jp_z_n16(CPU* cpu, MMU* mmu) {
    if (get_flag(cpu, FLAG_Z)) {
        u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 3;
        cpu->branch_taken = false;
    }
}

void inst_jp_nc_n16(CPU* cpu, MMU* mmu) {
    if (!get_flag(cpu, FLAG_C)) {
        u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 3;
        cpu->branch_taken = false;
    }
}

void inst_jp_c_n16(CPU* cpu, MMU* mmu) {
    if (get_flag(cpu, FLAG_C)) {
        u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 3;
        cpu->branch_taken = false;
    }
}

void inst_call_nz_n16(CPU* cpu, MMU* mmu) {
    if (!get_flag(cpu, FLAG_Z)) {
        u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
        cpu->sp -= 2;
        mmu->memory[cpu->sp] = cpu->pc & 0xFF;
        mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 3;
        cpu->branch_taken = false;
    }
}

void inst_call_z_n16(CPU* cpu, MMU* mmu) {
    if (get_flag(cpu, FLAG_Z)) {
        u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
        cpu->sp -= 2;
        mmu->memory[cpu->sp] = cpu->pc & 0xFF;
        mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 3;
        cpu->branch_taken = false;
    }
}

void inst_call_nc_n16(CPU* cpu, MMU* mmu) {
    if (!get_flag(cpu, FLAG_C)) {
        u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
        cpu->sp -= 2;
        mmu->memory[cpu->sp] = cpu->pc & 0xFF;
        mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 3;
        cpu->branch_taken = false;
    }
}

void inst_call_c_n16(CPU* cpu, MMU* mmu) {
    if (get_flag(cpu, FLAG_C)) {
        u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
        cpu->sp -= 2;
        mmu->memory[cpu->sp] = cpu->pc & 0xFF;
        mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 3;
        cpu->branch_taken = false;
    }
}

void inst_ret_nz(CPU* cpu, MMU* mmu) {
    if (!get_flag(cpu, FLAG_Z)) {
        u16 addr = mmu->memory[cpu->sp] | (mmu->memory[cpu->sp + 1] << 8);
        cpu->sp += 2;
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 1;
        cpu->branch_taken = false;
    }
}

void inst_ret_z(CPU* cpu, MMU* mmu) {
    if (get_flag(cpu, FLAG_Z)) {
        u16 addr = mmu->memory[cpu->sp] | (mmu->memory[cpu->sp + 1] << 8);
        cpu->sp += 2;
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 1;
        cpu->branch_taken = false;
    }
}

void inst_ret_nc(CPU* cpu, MMU* mmu) {
    if (!get_flag(cpu, FLAG_C)) {
        u16 addr = mmu->memory[cpu->sp] | (mmu->memory[cpu->sp + 1] << 8);
        cpu->sp += 2;
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 1;
        cpu->branch_taken = false;
    }
}

void inst_ret_c(CPU* cpu, MMU* mmu) {
    if (get_flag(cpu, FLAG_C)) {
        u16 addr = mmu->memory[cpu->sp] | (mmu->memory[cpu->sp + 1] << 8);
        cpu->sp += 2;
        cpu->pc = addr;
        cpu->branch_taken = true;
    } else {
        cpu->pc += 1;
        cpu->branch_taken = false;
    }
}

// ============================================================================
// INSTRUCTIONS RST (RESTART)
// ============================================================================

void inst_rst_00h(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = cpu->pc & 0xFF;
    mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
    cpu->pc = 0x0000;
}

void inst_rst_08h(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = cpu->pc & 0xFF;
    mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
    cpu->pc = 0x0008;
}

void inst_rst_10h(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = cpu->pc & 0xFF;
    mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
    cpu->pc = 0x0010;
}

void inst_rst_18h(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = cpu->pc & 0xFF;
    mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
    cpu->pc = 0x0018;
}

void inst_rst_20h(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = cpu->pc & 0xFF;
    mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
    cpu->pc = 0x0020;
}

void inst_rst_28h(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = cpu->pc & 0xFF;
    mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
    cpu->pc = 0x0028;
}

void inst_rst_30h(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = cpu->pc & 0xFF;
    mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
    cpu->pc = 0x0030;
}

void inst_rst_38h(CPU* cpu, MMU* mmu) {
    cpu->sp -= 2;
    mmu->memory[cpu->sp] = cpu->pc & 0xFF;
    mmu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
    cpu->pc = 0x0038;
}

// ============================================================================
// INSTRUCTIONS LD MANQUANTES
// ============================================================================

void inst_ld_hl_sp_e8(CPU* cpu, MMU* mmu) {
    s8 offset = (s8)mmu->memory[cpu->pc + 1];
    u32 result = cpu->sp + offset;
    
    set_flag(cpu, FLAG_Z, false);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, (cpu->sp & 0x0F) + (offset & 0x0F) > 0x0F);
    set_flag(cpu, FLAG_C, (cpu->sp & 0xFF) + (offset & 0xFF) > 0xFF);
    
    cpu->hl = result & 0xFFFF;
    cpu->pc += 2;
}

void inst_ld_imm16_a(CPU* cpu, MMU* mmu) {
    u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
    mmu->memory[addr] = get_reg_a(cpu);
    cpu->pc += 3;
}

void inst_ld_a_imm16(CPU* cpu, MMU* mmu) {
    u16 addr = mmu->memory[cpu->pc + 1] | (mmu->memory[cpu->pc + 2] << 8);
    set_reg_a(cpu, mmu->memory[addr]);
    cpu->pc += 3;
}

// ============================================================================
// INSTRUCTIONS INC/DEC
// ============================================================================

void inst_inc_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = (opcode >> 3) & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break; // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    u8 result = value + 1;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, (value & 0x0F) == 0x0F);
    
    // Debug pour INC E
    if (reg == 3) { // E
        printf("INC E: %d -> %d, Z=%d\n", value, result, result == 0 ? 1 : 0);
    }
    
    switch (reg) {
        case 0: set_reg_b(cpu, result); break;
        case 1: set_reg_c(cpu, result); break;
        case 2: set_reg_d(cpu, result); break;
        case 3: set_reg_e(cpu, result); break;
        case 4: set_reg_h(cpu, result); break;
        case 5: set_reg_l(cpu, result); break;
        case 6: mmu->memory[cpu->hl] = result; break; // (HL)
        case 7: set_reg_a(cpu, result); break;
    }
    
    cpu->pc += 1;
}

void inst_dec_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = (opcode >> 3) & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break; // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    u8 result = value - 1;
    
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, true);
    set_flag(cpu, FLAG_H, (value & 0x0F) == 0);
    
    switch (reg) {
        case 0: set_reg_b(cpu, result); break;
        case 1: set_reg_c(cpu, result); break;
        case 2: set_reg_d(cpu, result); break;
        case 3: set_reg_e(cpu, result); break;
        case 4: set_reg_h(cpu, result); break;
        case 5: set_reg_l(cpu, result); break;
        case 6: mmu->memory[cpu->hl] = result; break; // (HL)
        case 7: set_reg_a(cpu, result); break;
    }
    
    cpu->pc += 1;
}

void inst_inc_r16(CPU* cpu, MMU* mmu) {
    (void)mmu;
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg_pair = (opcode >> 4) & 0x03;
    
    switch (reg_pair) {
        case 0: cpu->bc++; break;
        case 1: cpu->de++; break;
        case 2: cpu->hl++; break;
        case 3: cpu->sp++; break;
    }
    
    cpu->pc += 1;
}

void inst_dec_r16(CPU* cpu, MMU* mmu) {
    (void)mmu;
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg_pair = (opcode >> 4) & 0x03;
    
    switch (reg_pair) {
        case 0: cpu->bc--; break;
        case 1: cpu->de--; break;
        case 2: cpu->hl--; break;
        case 3: cpu->sp--; break;
    }
    
    cpu->pc += 1;
}

void inst_add_hl_r16(CPU* cpu, MMU* mmu) {
    (void)mmu;
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg_pair = (opcode >> 4) & 0x03;
    u16 value = 0;
    
    switch (reg_pair) {
        case 0: value = cpu->bc; break;
        case 1: value = cpu->de; break;
        case 2: value = cpu->hl; break;
        case 3: value = cpu->sp; break;
    }
    
    u32 result = cpu->hl + value;
    
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, (cpu->hl & 0x0FFF) + (value & 0x0FFF) > 0x0FFF);
    set_flag(cpu, FLAG_C, result > 0xFFFF);
    
    cpu->hl = result & 0xFFFF;
    cpu->pc += 1;
}

// ============================================================================
// INSTRUCTIONS DE ROTATION (REGISTRE A)
// ============================================================================

void inst_rlca(CPU* cpu, MMU* mmu) {
    (void)mmu;
    u8 a = get_reg_a(cpu);
    u8 carry = (a & 0x80) ? 1 : 0;
    
    a = (a << 1) | carry;
    
    set_reg_a(cpu, a);
    set_flag(cpu, FLAG_Z, false);  // RLCA n'affecte jamais Z
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, carry);
    
    cpu->pc += 1;
}

void inst_rrca(CPU* cpu, MMU* mmu) {
    (void)mmu;
    u8 a = get_reg_a(cpu);
    u8 carry = a & 0x01;
    
    a = (a >> 1) | (carry << 7);
    
    set_reg_a(cpu, a);
    set_flag(cpu, FLAG_Z, false);  // RRCA n'affecte jamais Z
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, carry);
    
    cpu->pc += 1;
}

void inst_rla(CPU* cpu, MMU* mmu) {
    (void)mmu;
    u8 a = get_reg_a(cpu);
    u8 old_carry = get_flag(cpu, FLAG_C) ? 1 : 0;
    u8 new_carry = (a & 0x80) ? 1 : 0;
    
    a = (a << 1) | old_carry;
    
    set_reg_a(cpu, a);
    set_flag(cpu, FLAG_Z, false);  // RLA n'affecte jamais Z
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, new_carry);
    
    cpu->pc += 1;
}

void inst_rra(CPU* cpu, MMU* mmu) {
    (void)mmu;
    u8 a = get_reg_a(cpu);
    u8 old_carry = get_flag(cpu, FLAG_C) ? 1 : 0;
    u8 new_carry = a & 0x01;
    
    a = (a >> 1) | (old_carry << 7);
    
    set_reg_a(cpu, a);
    set_flag(cpu, FLAG_Z, false);  // RRA n'affecte jamais Z
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, new_carry);
    
    cpu->pc += 1;
}

// ============================================================================
// INSTRUCTIONS SPÉCIALES
// ============================================================================

void inst_daa(CPU* cpu, MMU* mmu) {
    (void)mmu;
    // DAA (Decimal Adjust Accumulator) - complexe, placeholder pour l'instant
    printf("DAA pas encore implémenté\n");
    cpu->pc += 1;
}

void inst_cpl(CPU* cpu, MMU* mmu) {
    (void)mmu;
    u8 a = get_reg_a(cpu);
    set_reg_a(cpu, ~a);
    
    set_flag(cpu, FLAG_N, true);
    set_flag(cpu, FLAG_H, true);
    
    cpu->pc += 1;
}

void inst_scf(CPU* cpu, MMU* mmu) {
    (void)mmu;
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, true);
    
    cpu->pc += 1;
}

void inst_ccf(CPU* cpu, MMU* mmu) {
    (void)mmu;
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, !get_flag(cpu, FLAG_C));
    
    cpu->pc += 1;
}

// ============================================================================
// INSTRUCTIONS CB-PREFIX PLACEHOLDERS
// ============================================================================

// Ces fonctions seront ajoutées au fur et à mesure des besoins
// Pour l'instant, on a l'infrastructure de base pour le port série

void inst_srl_r8(CPU* cpu, MMU* mmu) {
    (void)mmu; // Suppression du warning
    
    u8 opcode = mmu->memory[cpu->pc + 1];
    u8 reg = opcode & 0x07;
    
    u8 value = 0;
    u8 result = 0;
    
    // Lire la valeur du registre
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    result = value >> 1;
    
    // Mettre à jour les flags
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, 0);
    set_flag(cpu, FLAG_H, 0);
    set_flag(cpu, FLAG_C, value & 0x01);
    
    // Mettre à jour le registre
    switch (reg) {
        case 0: set_reg_b(cpu, result); break;
        case 1: set_reg_c(cpu, result); break;
        case 2: set_reg_d(cpu, result); break;
        case 3: set_reg_e(cpu, result); break;
        case 4: set_reg_h(cpu, result); break;
        case 5: set_reg_l(cpu, result); break;
        case 6: mmu->memory[cpu->hl] = result; break;  // (HL)
        case 7: set_reg_a(cpu, result); break;
    }
    
    cpu->pc += 2;
}

void inst_rr_r8(CPU* cpu, MMU* mmu) {
    (void)mmu; // Suppression du warning
    
    u8 opcode = mmu->memory[cpu->pc + 1];
    u8 reg = opcode & 0x07;
    
    u8 value = 0;
    u8 result = 0;
    
    // Lire la valeur du registre
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    u8 carry_in = get_flag(cpu, FLAG_C) ? 1 : 0;
    result = (value >> 1) | (carry_in << 7);
    
    // Mettre à jour les flags
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, 0);
    set_flag(cpu, FLAG_H, 0);
    set_flag(cpu, FLAG_C, value & 0x01);
    
    // Mettre à jour le registre
    switch (reg) {
        case 0: set_reg_b(cpu, result); break;
        case 1: set_reg_c(cpu, result); break;
        case 2: set_reg_d(cpu, result); break;
        case 3: set_reg_e(cpu, result); break;
        case 4: set_reg_h(cpu, result); break;
        case 5: set_reg_l(cpu, result); break;
        case 6: mmu->memory[cpu->hl] = result; break;  // (HL)
        case 7: set_reg_a(cpu, result); break;
    }
    
    cpu->pc += 2;
}

void inst_rlc_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    u8 result = 0;
    
    // Lire la valeur selon le registre
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    // RLC : rotation gauche circulaire
    u8 carry = (value & 0x80) ? 1 : 0;
    result = (value << 1) | carry;
    
    // Écrire le résultat
    switch (reg) {
        case 0: set_reg_b(cpu, result); break;
        case 1: set_reg_c(cpu, result); break;
        case 2: set_reg_d(cpu, result); break;
        case 3: set_reg_e(cpu, result); break;
        case 4: set_reg_h(cpu, result); break;
        case 5: set_reg_l(cpu, result); break;
        case 6: mmu->memory[cpu->hl] = result; break;  // (HL)
        case 7: set_reg_a(cpu, result); break;
    }
    
    // Flags
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, carry);
    
    cpu->pc += 1;
}

void inst_rl_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    u8 result = 0;
    
    // Lire la valeur selon le registre
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    // RL : rotation gauche avec carry
    u8 old_carry = get_flag(cpu, FLAG_C) ? 1 : 0;
    u8 new_carry = (value & 0x80) ? 1 : 0;
    result = (value << 1) | old_carry;
    
    // Écrire le résultat
    switch (reg) {
        case 0: set_reg_b(cpu, result); break;
        case 1: set_reg_c(cpu, result); break;
        case 2: set_reg_d(cpu, result); break;
        case 3: set_reg_e(cpu, result); break;
        case 4: set_reg_h(cpu, result); break;
        case 5: set_reg_l(cpu, result); break;
        case 6: mmu->memory[cpu->hl] = result; break;  // (HL)
        case 7: set_reg_a(cpu, result); break;
    }
    
    // Flags
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, new_carry);
    
    cpu->pc += 1;
}

void inst_rrc_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    u8 result = 0;
    
    // Lire la valeur selon le registre
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    // RRC : rotation droite circulaire
    u8 carry = value & 0x01;
    result = (value >> 1) | (carry << 7);
    
    // Écrire le résultat
    switch (reg) {
        case 0: set_reg_b(cpu, result); break;
        case 1: set_reg_c(cpu, result); break;
        case 2: set_reg_d(cpu, result); break;
        case 3: set_reg_e(cpu, result); break;
        case 4: set_reg_h(cpu, result); break;
        case 5: set_reg_l(cpu, result); break;
        case 6: mmu->memory[cpu->hl] = result; break;  // (HL)
        case 7: set_reg_a(cpu, result); break;
    }
    
    // Flags
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, carry);
    
    cpu->pc += 1;
}

void inst_sla_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    u8 result = 0;
    
    // Lire la valeur selon le registre
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    // SLA : shift left arithmetic (même que SLL)
    u8 carry = (value & 0x80) ? 1 : 0;
    result = value << 1;
    
    // Écrire le résultat
    switch (reg) {
        case 0: set_reg_b(cpu, result); break;
        case 1: set_reg_c(cpu, result); break;
        case 2: set_reg_d(cpu, result); break;
        case 3: set_reg_e(cpu, result); break;
        case 4: set_reg_h(cpu, result); break;
        case 5: set_reg_l(cpu, result); break;
        case 6: mmu->memory[cpu->hl] = result; break;  // (HL)
        case 7: set_reg_a(cpu, result); break;
    }
    
    // Flags
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, carry);
    
    cpu->pc += 1;
}

void inst_sra_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    u8 result = 0;
    
    // Lire la valeur selon le registre
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    // SRA : shift right arithmetic (garde le bit de signe)
    u8 carry = value & 0x01;
    u8 sign_bit = value & 0x80;
    result = (value >> 1) | sign_bit;
    
    // Écrire le résultat
    switch (reg) {
        case 0: set_reg_b(cpu, result); break;
        case 1: set_reg_c(cpu, result); break;
        case 2: set_reg_d(cpu, result); break;
        case 3: set_reg_e(cpu, result); break;
        case 4: set_reg_h(cpu, result); break;
        case 5: set_reg_l(cpu, result); break;
        case 6: mmu->memory[cpu->hl] = result; break;  // (HL)
        case 7: set_reg_a(cpu, result); break;
    }
    
    // Flags
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, carry);
    
    cpu->pc += 1;
}

void inst_swap_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    u8 result = 0;
    
    // Lire la valeur selon le registre
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;  // (HL)
        case 7: value = get_reg_a(cpu); break;
    }
    
    // SWAP : échange les nibbles (bits 7-4 <-> bits 3-0)
    result = ((value & 0x0F) << 4) | ((value & 0xF0) >> 4);
    
    // Écrire le résultat
    switch (reg) {
        case 0: set_reg_b(cpu, result); break;
        case 1: set_reg_c(cpu, result); break;
        case 2: set_reg_d(cpu, result); break;
        case 3: set_reg_e(cpu, result); break;
        case 4: set_reg_h(cpu, result); break;
        case 5: set_reg_l(cpu, result); break;
        case 6: mmu->memory[cpu->hl] = result; break;  // (HL)
        case 7: set_reg_a(cpu, result); break;
    }
    
    // Flags
    set_flag(cpu, FLAG_Z, result == 0);
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, false);
    set_flag(cpu, FLAG_C, false);
    
    cpu->pc += 1;
}

// ============================================================================
// INSTRUCTIONS DE MANIPULATION DE BITS (48)
// ============================================================================

// BIT instructions (8) - Test de bit
void inst_bit_0_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;
        case 7: value = get_reg_a(cpu); break;
    }
    
    set_flag(cpu, FLAG_Z, !(value & 0x01));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    
    cpu->pc += 1;
}

void inst_bit_1_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;
        case 7: value = get_reg_a(cpu); break;
    }
    
    set_flag(cpu, FLAG_Z, !(value & 0x02));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    
    cpu->pc += 1;
}

void inst_bit_2_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;
        case 7: value = get_reg_a(cpu); break;
    }
    
    set_flag(cpu, FLAG_Z, !(value & 0x04));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    
    cpu->pc += 1;
}

void inst_bit_3_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;
        case 7: value = get_reg_a(cpu); break;
    }
    
    set_flag(cpu, FLAG_Z, !(value & 0x08));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    
    cpu->pc += 1;
}

void inst_bit_4_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;
        case 7: value = get_reg_a(cpu); break;
    }
    
    set_flag(cpu, FLAG_Z, !(value & 0x10));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    
    cpu->pc += 1;
}

void inst_bit_5_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;
        case 7: value = get_reg_a(cpu); break;
    }
    
    set_flag(cpu, FLAG_Z, !(value & 0x20));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    
    cpu->pc += 1;
}

void inst_bit_6_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;
        case 7: value = get_reg_a(cpu); break;
    }
    
    set_flag(cpu, FLAG_Z, !(value & 0x40));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    
    cpu->pc += 1;
}

void inst_bit_7_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); break;
        case 1: value = get_reg_c(cpu); break;
        case 2: value = get_reg_d(cpu); break;
        case 3: value = get_reg_e(cpu); break;
        case 4: value = get_reg_h(cpu); break;
        case 5: value = get_reg_l(cpu); break;
        case 6: value = mmu->memory[cpu->hl]; break;
        case 7: value = get_reg_a(cpu); break;
    }
    
    set_flag(cpu, FLAG_Z, !(value & 0x80));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    
    cpu->pc += 1;
}

// BIT HL instructions (8) - Test de bit sur (HL)
void inst_bit_0_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    set_flag(cpu, FLAG_Z, !(value & 0x01));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    cpu->pc += 1;
}

void inst_bit_1_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    set_flag(cpu, FLAG_Z, !(value & 0x02));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    cpu->pc += 1;
}

void inst_bit_2_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    set_flag(cpu, FLAG_Z, !(value & 0x04));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    cpu->pc += 1;
}

void inst_bit_3_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    set_flag(cpu, FLAG_Z, !(value & 0x08));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    cpu->pc += 1;
}

void inst_bit_4_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    set_flag(cpu, FLAG_Z, !(value & 0x10));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    cpu->pc += 1;
}

void inst_bit_5_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    set_flag(cpu, FLAG_Z, !(value & 0x20));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    cpu->pc += 1;
}

void inst_bit_6_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    set_flag(cpu, FLAG_Z, !(value & 0x40));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    cpu->pc += 1;
}

void inst_bit_7_hl(CPU* cpu, MMU* mmu) {
    u8 value = mmu->memory[cpu->hl];
    set_flag(cpu, FLAG_Z, !(value & 0x80));
    set_flag(cpu, FLAG_N, false);
    set_flag(cpu, FLAG_H, true);
    cpu->pc += 1;
}

// SET instructions (8) - Mettre un bit à 1
void inst_set_0_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value | 0x01); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value | 0x01); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value | 0x01); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value | 0x01); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value | 0x01); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value | 0x01); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value | 0x01; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value | 0x01); break;
    }
    cpu->pc += 1;
}

void inst_set_1_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value | 0x02); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value | 0x02); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value | 0x02); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value | 0x02); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value | 0x02); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value | 0x02); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value | 0x02; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value | 0x02); break;
    }
    cpu->pc += 1;
}

void inst_set_2_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value | 0x04); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value | 0x04); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value | 0x04); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value | 0x04); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value | 0x04); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value | 0x04); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value | 0x04; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value | 0x04); break;
    }
    cpu->pc += 1;
}

void inst_set_3_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value | 0x08); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value | 0x08); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value | 0x08); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value | 0x08); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value | 0x08); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value | 0x08); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value | 0x08; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value | 0x08); break;
    }
    cpu->pc += 1;
}

void inst_set_4_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value | 0x10); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value | 0x10); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value | 0x10); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value | 0x10); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value | 0x10); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value | 0x10); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value | 0x10; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value | 0x10); break;
    }
    cpu->pc += 1;
}

void inst_set_5_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value | 0x20); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value | 0x20); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value | 0x20); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value | 0x20); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value | 0x20); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value | 0x20); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value | 0x20; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value | 0x20); break;
    }
    cpu->pc += 1;
}

void inst_set_6_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value | 0x40); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value | 0x40); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value | 0x40); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value | 0x40); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value | 0x40); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value | 0x40); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value | 0x40; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value | 0x40); break;
    }
    cpu->pc += 1;
}

void inst_set_7_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value | 0x80); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value | 0x80); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value | 0x80); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value | 0x80); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value | 0x80); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value | 0x80); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value | 0x80; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value | 0x80); break;
    }
    cpu->pc += 1;
}

// SET HL instructions (8) - Mettre un bit à 1 sur (HL)
void inst_set_0_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] |= 0x01; cpu->pc += 1; }
void inst_set_1_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] |= 0x02; cpu->pc += 1; }
void inst_set_2_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] |= 0x04; cpu->pc += 1; }
void inst_set_3_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] |= 0x08; cpu->pc += 1; }
void inst_set_4_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] |= 0x10; cpu->pc += 1; }
void inst_set_5_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] |= 0x20; cpu->pc += 1; }
void inst_set_6_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] |= 0x40; cpu->pc += 1; }
void inst_set_7_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] |= 0x80; cpu->pc += 1; }

// RES instructions (8) - Mettre un bit à 0
void inst_res_0_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value & ~0x01); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value & ~0x01); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value & ~0x01); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value & ~0x01); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value & ~0x01); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value & ~0x01); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value & ~0x01; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value & ~0x01); break;
    }
    cpu->pc += 1;
}

void inst_res_1_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value & ~0x02); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value & ~0x02); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value & ~0x02); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value & ~0x02); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value & ~0x02); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value & ~0x02); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value & ~0x02; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value & ~0x02); break;
    }
    cpu->pc += 1;
}

void inst_res_2_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value & ~0x04); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value & ~0x04); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value & ~0x04); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value & ~0x04); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value & ~0x04); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value & ~0x04); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value & ~0x04; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value & ~0x04); break;
    }
    cpu->pc += 1;
}

void inst_res_3_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value & ~0x08); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value & ~0x08); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value & ~0x08); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value & ~0x08); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value & ~0x08); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value & ~0x08); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value & ~0x08; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value & ~0x08); break;
    }
    cpu->pc += 1;
}

void inst_res_4_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value & ~0x10); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value & ~0x10); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value & ~0x10); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value & ~0x10); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value & ~0x10); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value & ~0x10); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value & ~0x10; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value & ~0x10); break;
    }
    cpu->pc += 1;
}

void inst_res_5_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value & ~0x20); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value & ~0x20); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value & ~0x20); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value & ~0x20); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value & ~0x20); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value & ~0x20); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value & ~0x20; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value & ~0x20); break;
    }
    cpu->pc += 1;
}

void inst_res_6_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value & ~0x40); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value & ~0x40); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value & ~0x40); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value & ~0x40); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value & ~0x40); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value & ~0x40); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value & ~0x40; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value & ~0x40); break;
    }
    cpu->pc += 1;
}

void inst_res_7_r8(CPU* cpu, MMU* mmu) {
    u8 opcode = mmu->memory[cpu->pc];
    u8 reg = opcode & 0x07;
    u8 value = 0;
    
    switch (reg) {
        case 0: value = get_reg_b(cpu); set_reg_b(cpu, value & ~0x80); break;
        case 1: value = get_reg_c(cpu); set_reg_c(cpu, value & ~0x80); break;
        case 2: value = get_reg_d(cpu); set_reg_d(cpu, value & ~0x80); break;
        case 3: value = get_reg_e(cpu); set_reg_e(cpu, value & ~0x80); break;
        case 4: value = get_reg_h(cpu); set_reg_h(cpu, value & ~0x80); break;
        case 5: value = get_reg_l(cpu); set_reg_l(cpu, value & ~0x80); break;
        case 6: value = mmu->memory[cpu->hl]; mmu->memory[cpu->hl] = value & ~0x80; break;
        case 7: value = get_reg_a(cpu); set_reg_a(cpu, value & ~0x80); break;
    }
    cpu->pc += 1;
}

// RES HL instructions (8) - Mettre un bit à 0 sur (HL)
void inst_res_0_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] &= ~0x01; cpu->pc += 1; }
void inst_res_1_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] &= ~0x02; cpu->pc += 1; }
void inst_res_2_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] &= ~0x04; cpu->pc += 1; }
void inst_res_3_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] &= ~0x08; cpu->pc += 1; }
void inst_res_4_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] &= ~0x10; cpu->pc += 1; }
void inst_res_5_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] &= ~0x20; cpu->pc += 1; }
void inst_res_6_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] &= ~0x40; cpu->pc += 1; }
void inst_res_7_hl(CPU* cpu, MMU* mmu) { mmu->memory[cpu->hl] &= ~0x80; cpu->pc += 1; }

// ============================================================================
// NOTES DE DÉVELOPPEMENT
// ============================================================================

/*
 * Ce fichier contient l'implémentation unifiée du CPU Game Boy LR35902.
 * 
 * Structure organisée par catégories :
 * - Utilitaires (registres, flags)
 * - Instructions de base (NOP, HALT, DI, EI)
 * - Instructions de chargement (LD variants)
 * - Instructions arithmétiques (ADD, etc.)
 * - Instructions de saut (JP, JR, CALL, RET)
 * - Instructions de pile (PUSH, POP)
 * - Instructions conditionnelles
 * - Instructions spéciales (CP, etc.)
 * - Gestion des interruptions
 * - Instructions LDH (pour port série)
 * - Gestion CB-prefix
 * - Boucle principale d'exécution
 * - Fonctions d'initialisation
 * 
 * Les instructions manquantes seront ajoutées progressivement selon les
 * besoins révélés par les tests (Blargg, etc.)
 */


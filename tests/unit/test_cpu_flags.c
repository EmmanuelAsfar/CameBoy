#include "../../src/common.h"
#include "../../src/cpu.h"
#include "../../src/mmu.h"
#include <assert.h>
#include <stdio.h>

static void reset_cpu(CPU* cpu, MMU* mmu) {
    cpu_init(cpu);
    mmu_init(mmu);
    cpu->pc = 0x0100;
}

static void load_code(MMU* mmu, const u8* code, size_t n) {
    for (size_t i = 0; i < n; i++) {
        mmu->memory[0x0100 + i] = code[i];
    }
}

static void step_n(CPU* cpu, MMU* mmu, int n) {
    for (int i = 0; i < n; i++) {
        cpu_step(cpu, mmu);
    }
}

static void test_add_flags(void) {
    CPU cpu; MMU mmu; reset_cpu(&cpu, &mmu);
    // LD A,00 ; ADD A,00 ; LD A,0F ; ADD A,01 ; LD A,FF ; ADD A,01 ; HALT
    const u8 prog[] = {
        0x3E, 0x00, 0xC6, 0x00,
        0x3E, 0x0F, 0xC6, 0x01,
        0x3E, 0xFF, 0xC6, 0x01,
        0x76
    };
    load_code(&mmu, prog, sizeof(prog));

    // LD A,00 ; ADD A,00
    step_n(&cpu, &mmu, 2);
    assert(cpu.af >> 8 == 0x00);
    {
        u8 f = (u8)(cpu.af & 0xFF);
        assert((f & 0x80) != 0); // Z
        assert((f & 0x40) == 0); // N
        assert((f & 0x20) == 0); // H
        assert((f & 0x10) == 0); // C
    }

    // LD A,0F ; ADD A,01
    step_n(&cpu, &mmu, 2);
    assert(cpu.af >> 8 == 0x10);
    {
        u8 f = (u8)(cpu.af & 0xFF);
        assert((f & 0x80) == 0);
        assert((f & 0x40) == 0);
        assert((f & 0x20) != 0); // H
        assert((f & 0x10) == 0);
    }

    // LD A,FF ; ADD A,01
    step_n(&cpu, &mmu, 2);
    assert(cpu.af >> 8 == 0x00);
    {
        u8 f = (u8)(cpu.af & 0xFF);
        assert((f & 0x80) != 0);
        assert((f & 0x10) != 0); // C
    }
}

static void test_sub_and_or_xor_cp(void) {
    CPU cpu; MMU mmu; reset_cpu(&cpu, &mmu);
    // LD A,10 ; SUB 01 ; LD A,F0 ; AND 0F ; LD A,F0 ; OR 0F ; LD A,FF ; XOR FF ; LD A,3C ; CP 2F ; HALT
    const u8 prog[] = {
        0x3E, 0x10, 0xD6, 0x01,
        0x3E, 0xF0, 0xE6, 0x0F,
        0x3E, 0xF0, 0xF6, 0x0F,
        0x3E, 0xFF, 0xEE, 0xFF,
        0x3E, 0x3C, 0xFE, 0x2F,
        0x76
    };
    load_code(&mmu, prog, sizeof(prog));

    // SUB
    step_n(&cpu, &mmu, 2);
    assert(cpu.af >> 8 == 0x0F);
    {
        u8 f = (u8)(cpu.af & 0xFF);
        assert((f & 0x40) != 0); // N
        assert((f & 0x20) != 0); // H borrow
        assert((f & 0x10) == 0);
    }

    // AND
    step_n(&cpu, &mmu, 2);
    assert(cpu.af >> 8 == 0x00);
    {
        u8 f = (u8)(cpu.af & 0xFF);
        assert((f & 0x80) != 0); // Z
        assert((f & 0x40) == 0);
        assert((f & 0x20) != 0); // H=1 after AND
        assert((f & 0x10) == 0);
    }

    // OR
    step_n(&cpu, &mmu, 2);
    assert(cpu.af >> 8 == 0xFF);
    {
        u8 f = (u8)(cpu.af & 0xFF);
        assert((f & 0x80) == 0);
        assert((f & 0x40) == 0);
        assert((f & 0x20) == 0);
        assert((f & 0x10) == 0);
    }

    // XOR
    step_n(&cpu, &mmu, 2);
    assert(cpu.af >> 8 == 0x00);
    {
        u8 f = (u8)(cpu.af & 0xFF);
        assert((f & 0x80) != 0);
        assert((f & 0x40) == 0);
        assert((f & 0x20) == 0);
        assert((f & 0x10) == 0);
    }

    // CP
    step_n(&cpu, &mmu, 2);
    assert(cpu.af >> 8 == 0x3C); // unchanged
    {
        u8 f = (u8)(cpu.af & 0xFF);
        assert((f & 0x80) == 0);
        assert((f & 0x40) != 0); // N
        assert((f & 0x20) != 0); // H borrow
        assert((f & 0x10) == 0); // no carry
    }
}

int main(void) {
    printf("=== TEST CPU FLAGS ===\n");
    test_add_flags();
    test_sub_and_or_xor_cp();
    printf("PASS\n");
    return 0;
}



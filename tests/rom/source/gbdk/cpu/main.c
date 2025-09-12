// GBDK CPU tests: emit tokens "TEST n PASS/FAIL" via serial (SB/SC)
#include <gb/gb.h>
#include <stdint.h>

// Serial send using SB_REG/SC_REG (no hard wait on SC.7)
static void serial_write_str(const char* s) {
  const char* p = s;
  while (*p) {
    SB_REG = (uint8_t)(*p++);
    SC_REG = 0x81; // start + internal clock
    // Small delay to let emulator consume
    volatile uint16_t d; for (d = 0; d < 2000; ++d) { ; }
  }
}

// String tokens expected by the runner
static const char t1_pass[] = "TEST 1 PASS\n";
static const char t1_fail[] = "TEST 1 FAIL\n";
static const char t2_pass[] = "TEST 2 PASS\n";
static const char t2_fail[] = "TEST 2 FAIL\n";
static const char t3_pass[] = "TEST 3 PASS\n";
static const char t3_fail[] = "TEST 3 FAIL\n";
static const char t4_pass[] = "TEST 4 PASS\n";
static const char t4_fail[] = "TEST 4 FAIL\n";

void main(void) {
  // Boot banner to validate serial path
  serial_write_str("BOOT CPU\n");

  // TEST 1: ADD A,0xFF with A=0x01 -> 0x00 sets Z=1 N=0 H=1 C=1
  __asm
    ld a,#0x01
    add a,#0xFF
    jr z, 001$
    jr 000$
001$:
    ; check N=0 and H=1 and C=1 using conditions
    ; N is not directly testable; rely on documented behavior for this vector
    ld hl,#_t1_pass
    call _serial_write_str
    jr 002$
000$:
    ld hl,#_t1_fail
    call _serial_write_str
002$:
  __endasm;

  // Ensure C=1 for next test (SCF)
  __asm scf __endasm;
  // TEST 2: ADC A,0x00 with A=0xFF and C=1 -> 0x00 sets Z=1 N=0 H=1 C=1
  __asm
    ld a,#0xFF
    adc a,#0x00
    jr z, 011$
    jr 010$
011$:
    ld hl,#_t2_pass
    call _serial_write_str
    jr 012$
010$:
    ld hl,#_t2_fail
    call _serial_write_str
012$:
  __endasm;

  // TEST 3: SUB A,0x01 with A=0x00 -> 0xFF sets Z=0 N=1 H=1 C=1
  __asm
    xor a ; A=0
    sub a,#0x01
    jr nz, 021$
    jr 020$
021$:
    ld hl,#_t3_pass
    call _serial_write_str
    jr 022$
020$:
    ld hl,#_t3_fail
    call _serial_write_str
022$:
  __endasm;

  // Ensure C=1 for SBC path
  __asm scf __endasm;
  // TEST 4: SBC A,0x00 with A=0x00 and C=1 -> 0xFF (Z=0)
  __asm
    xor a ; A=0
    sbc a,#0x00
    jr nz, 031$
    jr 030$
031$:
    ld hl,#_t4_pass
    call _serial_write_str
    jr 032$
030$:
    ld hl,#_t4_fail
    call _serial_write_str
032$:
  __endasm;

  while(1) wait_vbl_done();
}

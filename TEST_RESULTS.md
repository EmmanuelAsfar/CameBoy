# Résultats des Tests Unitaires - CameBoy 
 
**Date:** 08/09/2025 12:48:41,62 
**Status:** ❌ CERTAINS TESTS ONT ECHOUE 
 
## 📊 Synthèse Globale 
 
| Métrique | Valeur | 
|---------|--------| 
| **Total des tests** | 8 | 
| **Tests réussis** | 7 | 
| **Tests échoués** | 1 | 
 
## 📋 Résultats par Rubrique 
 
| Rubrique | Tests | Réussis | Échecs | Status | 
|----------|-------|---------|--------|--------| 
| **cpu** | 25 | 25 | 0 | ✅ | 
| **mmu** | 10 | 10 | 0 | ✅ | 
| **ppu** | 27 | 26 | 1 | ❌ | 
| **timer** | 8 | 8 | 0 | ✅ | 
| **interrupt** | 8 | 8 | 0 | ✅ | 
| **joypad** | 7 | 7 | 0 | ✅ | 
| **joypad_irq** | 1 | 1 | 0 | ✅ | 
| **cpu_flags** | 1 | 1 | 0 | ✅ | 
 
## 🔍 Détails des Tests 
 
### test_cpu 
 
**Status:** 
✅ **PASSED** 
 
**Log:** 
``` 
=== TESTS UNITAIRES CPU LR35902 ===

Test 1: CPU Initialisation... PASS
Test 2: CPU Reset... PASS
Test 3: CPU Flags... PASS
Test 4: CPU Registers... PASS
Test 5: Arithmetic ADD... PASS
Test 6: Arithmetic SUB... PASS
Test 7: Arithmetic ADC... PASS
Test 8: Arithmetic SBC... PASS
Test 9: Logical AND... PASS
Test 10: Logical OR... PASS
Test 11: Logical XOR... PASS
Test 12: Logical CP... PASS
Test 13: Load LD r8,r8... PASS
Test 14: Load LD r8,n8... PASS
Test 15: Load LD r16,n16... PASS
Test 16: Jumps JR NZ... PASS
Test 17: Jumps JR Z... PASS
Test 18: Jumps JR NC... PASS
Test 19: Jumps JR C... PASS
Test 20: Stack PUSH/POP... PASS
Test 21: Interrupts... PASS
Test 22: EI Delay... PASS
Test 23: HALT bug... PASS
Test 24: DAA cases... PASS
Test 25: STOP + KEY1 speed switch (CGB)... PASS

=== RÉSULTATS ===
Tests passés: 25/25
✅ TOUS LES TESTS SONT PASSÉS !
``` 
 
### test_mmu 
 
**Status:** 
✅ **PASSED** 
 
**Log:** 
``` 
=== TESTS UNITAIRES MMU ===

Test 1: MMU Initialisation... PASS
Test 2: MMU Reset... PASS
Test 3: MMU Memory Mapping... PASS
Test 4: MMU Cart Parsing... Avertissement: Logo Nintendo invalide
PASS
Test 5: MMU Read/Write 8-bit... PASS
Test 6: MMU Read/Write 16-bit... PASS
Test 7: MMU Echo RAM... PASS
Test 8: MMU VRAM/OAM Restrictions... PASS
Test 9: MMU DMA OAM Copy... PASS
Test 10: MMU DMA Timed & BusLock... PASS

=== RÉSULTATS ===
Tests passés: 10/10
✅ TOUS LES TESTS SONT PASSÉS !
``` 
 
### test_ppu 
 
**Status:** 
❌ **FAILED** 
 
**Log:** 
``` 
=== TESTS UNITAIRES PPU ===

Test 1: PPU Initialisation... PASS
Test 2: PPU Reset... PASS
Test 3: PPU Registers... PASS
Test 4: PPU Modes... PASS
Test 5: PPU Pixel Transfer... PASS
Test 6: PPU HBlank... PASS
Test 7: PPU VBlank... PASS
Test 8: PPU Render Line... PASS
Test 9: PPU Palettes... PASS
Test 10: PPU STAT IRQ HBlank... PASS
Test 11: PPU STAT IRQ VBlank... PASS
Test 12: PPU STAT IRQ OAM... PASS
Test 13: PPU STAT IRQ LYC... PASS
Test 14: PPU Window Basic... PASS
Test 15: PPU Sprites Basic... PASS
Test 16: PPU Sprites Priority vs BG... PASS
Test 17: PPU Sprites Limit 10... PASS
Test 18: PPU Sprites Flip XY... PASS
Test 19: PPU LCDC Off/On... PASS
Test 20: PPU Window WX Clamp... PASS
Test 21: PPU Fine Scroll SCX... PASS
Test 22: PPU FIFO Basic... PASS
Test 23: PPU FIFO Overflow... PASS
Test 24: PPU Mode Timings... PASS
Test 25: PPU STAT IRQ Transitions... PASS
Test 26: PPU Window Edge Cases... PASS
Test 27: PPU Sprite Priority Detailed... Assertion failed: ppu.framebuffer[12] == ppu_get_pixel_color(&ppu, 1), file tests\unit\test_ppu.c, line 918
``` 
 

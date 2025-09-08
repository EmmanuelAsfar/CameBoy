# CPU (LR35902) a" SpAcifications d'implAmentation

Retour: [Index specs](./README.md) A [Architecture](../architecture.md) A [Tests](../testing.md)

## 1) Principe de fonctionnement (Game Boy)

Le LR35902 est un CPU 8a'bit (hybride 8080/Z80) cadencA A  4.194304 MHz.
- Registres 8a'bit: A, B, C, D, E, H, L (F = flags)
- Registres 16a'bit: AF, BC, DE, HL, SP, PC
- Flags: Z (zAro), N (soustraction), H (demia'retenue), C (retenue)
- Interruptions: VBLANK (0x40), LCD STAT (0x48), TIMER (0x50), SERIAL (0x58), JOYPAD (0x60)

### Cycle dainstruction

```
flowchart TD
  F[Fetch opcode @PC] --> D[Decode via tables std/CB]
  D --> E[Execute]
  E --> U[MA J registres/flags/PC]
  U --> C[Compter cycles]
  C --> F
```

- EI: laactivation des interruptions prend effet aprAs lainstruction suivante
- HALT: CPU saarrAte jusquaA  interruption; HALT bug si IME=0 et IRQ en attente a' PC peut ne pas saincrAmenter

```
sequenceDiagram
  participant CPU
  participant MMU
  CPU->>MMU: read IE/IF
  alt IME=0 & (IE&IF)!=0
    CPU->>CPU: HALT bug (PC n'avance pas)
  else IME=1 & IRQ en attente
    CPU->>CPU: push PC; IME=0; PC=handler
  end
```

RAfs Pan Docs: CPU, Registers/Flags, Instruction Set, HALT

---

## 2) Logique daimplAmentation (CameBoy)

### Structure
- Registres regroupAs: `u16 af, bc, de, hl, sp, pc` avec accesseurs 8a'bit (A/B/C/D/E/H/L/F)
- Atat CPU: `halted`, `ime`, `ei_pending` (EI retardA), `halt_bug`, `branch_taken`
- Tables daopcodes `opcodes[]` et `opcodes_cb[]` (dAclarAes dans `cpu_tables*.c`)

### ContrAle de flux et interruptions
- EI: `inst_ei` pose `ei_pending`; `cpu_step` active `ime` aprAs lainstruction suivante (conforme EI delay)
- HALT: 
  - si `IME=0` et `IE&IF!=0` a' `halt_bug=true`, `halted=false`, `pc` pas incrAmentA (bug HALT)
  - sinon `halted=true` et `pc += 1`; `cpu_step` reste A  4 cycles tant quaaucune IRQ en attente
- Interruptions: `cpu_interrupt` pousse PC, met `IME=0`, `halted=false`, et redirige vers le vecteur; laintAgration est pilotAe par laextArieur (tests/appelle `cpu_interrupt` directement)

### Instructions et flags
- ArithmAtiques/logiques: Z/N/H/C mis A  jour selon Pan Docs
- DAA: implAmentation basAe sur `N/H/C` et nibbles de A; assez fidAle pour usage courant (piste daajustement fine possible)
- Rotations RLCA/RRCA/RLA/RRA: Z=0 toujours (conforme), mise A  jour de C selon bit dAcalA
- LDH: accAs `0xFF00+imm` ou `0xFF00+C` via `mmu_read/write`
- Sauts conditionnels: `branch_taken` dAtermine `cycles_cond` au retour `cpu_step`

### HypothAses/choix
- `cpu_step` ne traite pas luia'mAme la sAlection daIRQ selon IE/IF; caest la couche dainterruptions/appelant qui invoque `cpu_interrupt` (cohArent avec la structure actuelle du projet)
- Valeurs dainitialisation (post boot ROM): `AF=0x01B0`, `BC=0x0013`, `DE=0x00D8`, `HL=0x014D`, `SP=0xFFFE`, `PC=0x0100`

---

## 3) StratAgie de test (unitaires)

Tests dans `tests/unit/test_cpu.c`:

- Initialisation/Reset
  - `test_cpu_init`, `test_cpu_reset`
- Flags et registres
  - `test_cpu_flags`, `test_cpu_registers`
- ArithmAtiques/logiques
  - `test_cpu_arithmetic_add/sub/adc/sbc`, `test_cpu_logical_and/or/xor/cp`
- Chargements et sauts
  - `test_cpu_load_ld_r8_r8/n8`, `test_cpu_load_ld_r16_n16`, `test_cpu_jumps_jr_*`
- Pile
  - `test_cpu_stack_push_pop`
- Interruptions
  - `test_cpu_interrupts`
- Nouveaux tests (prAcision timing/BCD)
  - `test_cpu_ei_delay`: vArifie laactivation diffArAe de `IME` (EI prend effet aprAs lainstruction suivante)
  - `test_cpu_halt_bug`: vArifie que `HALT` avec `IME=0` et `IE&IF!=0` naincrAmente pas `PC` et active `halt_bug`
  - `test_cpu_daa_cases`: couvre plusieurs cas daajustement dAcimal (N/H/C et nibbles) pour renforcer la conformitA de `DAA`

```
sequenceDiagram
  participant T as Test
  participant CPU
  participant MMU
  T->>CPU: EI @PC
  T->>CPU: cpu_step() (EI posA)
  CPU-->>T: IME==false
  T->>CPU: cpu_step() (instr suivante)
  CPU-->>T: IME==true
```

RAfArences: Pan Docs (sections EI delay, HALT, DAA)

### STOP et double-vitesse CGB (KEY1)

STOP a deux comportements selon la preparation et le modele:
- DMG ou CGB sans preparation: STOP met le CPU en veille basse consommation (reveil via joypad). Pour simplifier dans un cadre pedagogique, on peut traiter ceci comme un HALT prolonge.
- CGB avec preparation: si `KEY1` (`0xFF4D`) a `bit0=1` (preparation), l'execution de STOP bascule la vitesse CPU (bit7 de `KEY1` reflete l'etat: 0=normal, 1=double). La bascule efface `bit0` et l'execution continue sans entrer en veille.

Details `KEY1` (CGB uniquement):
- `bit7` (lecture seule): vitesse actuelle (0 = normal, 1 = double)
- `bit0` (lecture/ecriture): preparation de bascule (ecrire 1 avant STOP)
- bits 1-6: lecture a 1

Impact: la vitesse double affecte le rythme des sous-systemes synchronises au CPU (timers, APU, certaines durees PPU cote hote). Dans ce projet, on maintient la logique a frequence CPU fixe et on traite le double-speed comme un facteur d'horloge global a propager au besoin dans les modules concernes.

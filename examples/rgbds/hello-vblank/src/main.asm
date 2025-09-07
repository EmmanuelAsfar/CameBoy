; RGBDS hello (VBlank) – installe une ISR VBlank et incrémente un compteur

DEF IE  EQU $FFFF
DEF IF_REG  EQU $FF0F
DEF LCDC EQU $FF40
DEF LY  EQU $FF44

SECTION "Entry", ROM0[$0100]
    nop
    jp Start

SECTION "Code", ROM0

Start:
    di
    ld sp, $FFFE
    call LcdOff
    call InstallVBlank
    call LcdOn
.loop:
    halt
    jr .loop

LcdOff:
    ld a, [LCDC]
    res 7, a
    ld [LCDC], a
.wait:
    ld a, [LY]
    cp 144
    jr nz, .wait
    ret

LcdOn:
    ld a, [LCDC]
    set 7, a
    ld [LCDC], a
    ret

InstallVBlank:
    ; Activer IRQ VBlank
    ld a, 1
    ld [IE], a
    xor a
    ld [IF_REG], a
    ei
    ret

SECTION "VBlank ISR", ROM0[$0040]
    push af
    push hl
    ld hl, Counter
    inc [hl]
    ld a, [IF_REG]
    res 0, a
    ld [IF_REG], a
    pop hl
    pop af
    reti

SECTION "WRAM Vars", WRAM0
Counter: DS 1



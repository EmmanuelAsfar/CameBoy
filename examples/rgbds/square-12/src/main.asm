; RGBDS: carré centré 12x12 pixels (BG), via 4 tuiles partielles

DEF LCDC EQU $FF40
DEF BGP  EQU $FF47

SECTION "Entry", ROM0[$0100]
    nop
    jp Start

SECTION "Code", ROM0

Start:
    di
    ld sp, $FFFE

    ; LCD OFF et palette DMG
    xor a
    ld [LCDC], a
    ld a, $E4
    ld [BGP], a

    ; Copier tuiles (4 tuiles = 64 octets) vers $8000
    ld hl, Tiles
    ld de, $8000
    ld bc, 64
.copy_tiles:
    ld a, b
    or c
    jr z, .tiles_done
    ld a, [hl+]
    ld [de], a
    inc de
    dec bc
    jr .copy_tiles
.tiles_done:

    ; Placer tuiles dans la BG map: (9,8)->1, (10,8)->2, (9,9)->3, (10,9)->4
    ld hl, $9800 + (8*32) + 9
    ld a, 1
    ld [hl], a
    inc hl
    ld a, 2
    ld [hl], a

    ld hl, $9800 + (9*32) + 9
    ld a, 3
    ld [hl], a
    inc hl
    ld a, 4
    ld [hl], a

    ; LCD ON (BG ON, tiles 8000h, map 9800h)
    ld a, $91
    ld [LCDC], a

.loop:
    halt
    jr .loop

SECTION "Data", ROM0

; Tuile (9,8) haut-gauche: lignes 2..7, colonnes 0..5 => 0xFC
Tiles:
    db $00,$00, $00,$00, $FC,$FC, $FC,$FC, $FC,$FC, $FC,$FC, $FC,$FC, $FC,$FC
; Tuile (10,8) haut-droite: lignes 2..7, colonnes 2..7 => 0x3F
    db $00,$00, $00,$00, $3F,$3F, $3F,$3F, $3F,$3F, $3F,$3F, $3F,$3F, $3F,$3F
; Tuile (9,9) bas-gauche: lignes 0..5, colonnes 0..5 => 0xFC
    db $FC,$FC, $FC,$FC, $FC,$FC, $FC,$FC, $FC,$FC, $FC,$FC, $00,$00, $00,$00
; Tuile (10,9) bas-droite: lignes 0..5, colonnes 2..7 => 0x3F
    db $3F,$3F, $3F,$3F, $3F,$3F, $3F,$3F, $3F,$3F, $3F,$3F, $00,$00, $00,$00

; ROM de test simple pour CameBoy
; Affiche un pattern de couleurs sur l'écran

; Définitions des adresses importantes
LCDC_REG     EQU $FF40  ; LCD Control
STAT_REG     EQU $FF41  ; LCD Status
SCY_REG      EQU $FF42  ; Scroll Y
SCX_REG      EQU $FF43  ; Scroll X
LY_REG       EQU $FF44  ; LCD Y Coordinate
LYC_REG      EQU $FF45  ; LCD Y Compare
DMA_REG      EQU $FF46  ; DMA Transfer
BGP_REG      EQU $FF47  ; Background Palette
OBP0_REG     EQU $FF48  ; Object Palette 0
OBP1_REG     EQU $FF49  ; Object Palette 1
WY_REG       EQU $FF4A  ; Window Y
WX_REG       EQU $FF4B  ; Window X

; Adresses VRAM
VRAM_START   EQU $8000
TILE_DATA    EQU $8000
TILE_MAP     EQU $9800

; Point d'entrée
SECTION "ROM", ROM0[$0100]
    nop
    jp start

; Header Nintendo
SECTION "Header", ROM0[$0104]
    DB "TEST PATTERN"  ; Title (11 bytes)
    DS 5               ; Manufacturer code
    DB $00             ; CGB flag
    DB $00             ; New licensee code
    DB $00             ; SGB flag
    DB $00             ; Cartridge type
    DB $00             ; ROM size
    DB $00             ; RAM size
    DB $00             ; Destination code
    DB $00             ; Old licensee code
    DB $00             ; Mask ROM version
    DB $00             ; Header checksum
    DW $0000           ; Global checksum

start:
    ; Désactiver le LCD pendant l'initialisation
    ld a, $00
    ld (LCDC_REG), a
    
    ; Attendre que le LCD soit prêt
    call wait_vblank
    
    ; Initialiser les registres PPU
    ld a, $00
    ld (SCY_REG), a    ; Scroll Y = 0
    ld (SCX_REG), a    ; Scroll X = 0
    
    ; Palette de couleurs (blanc, gris clair, gris foncé, noir)
    ld a, $E4          ; 11 10 01 00
    ld (BGP_REG), a
    
    ; Créer des tiles de test
    call create_test_tiles
    
    ; Remplir la tile map avec nos tiles
    call fill_tile_map
    
    ; Activer le LCD
    ld a, $91          ; LCD on, BG on, tiles à $8000
    ld (LCDC_REG), a
    
    ; Boucle infinie
forever:
    halt
    nop
    jr forever

; Créer des tiles de test avec différents patterns
create_test_tiles:
    ld hl, TILE_DATA
    
    ; Tile 0: Tout blanc
    ld b, 16
tile0_loop:
    ld (hl), $00
    inc hl
    dec b
    jr nz, tile0_loop
    
    ; Tile 1: Rayures verticales
    ld b, 8
tile1_loop:
    ld (hl), $AA       ; 10101010
    inc hl
    ld (hl), $55       ; 01010101
    inc hl
    dec b
    jr nz, tile1_loop
    
    ; Tile 2: Damier
    ld b, 8
tile2_loop:
    ld (hl), $FF       ; 11111111
    inc hl
    ld (hl), $00       ; 00000000
    inc hl
    dec b
    jr nz, tile2_loop
    
    ; Tile 3: Tout noir
    ld b, 16
tile3_loop:
    ld (hl), $FF
    inc hl
    dec b
    jr nz, tile3_loop
    
    ret

; Remplir la tile map avec un pattern
fill_tile_map:
    ld hl, TILE_MAP
    ld b, 18           ; 18 lignes
map_y_loop:
    ld c, 20           ; 20 colonnes
map_x_loop:
    ; Calculer le tile index basé sur la position
    ld a, b
    add a, c
    and $03            ; Modulo 4 (0-3)
    ld (hl), a
    inc hl
    dec c
    jr nz, map_x_loop
    dec b
    jr nz, map_y_loop
    ret

; Attendre VBlank
wait_vblank:
    ld a, (LY_REG)
    cp 144
    jr c, wait_vblank
    ret

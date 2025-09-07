; RGBDS hello (cross) – affiche une croix géante sur l'écran

DEF LCDC EQU $FF40
DEF STAT EQU $FF41
DEF SCY  EQU $FF42
DEF SCX  EQU $FF43
DEF LY   EQU $FF44
DEF LYC  EQU $FF45
DEF BGP  EQU $FF47
DEF OBP0 EQU $FF48
DEF OBP1 EQU $FF49
DEF WY   EQU $FF4A
DEF WX   EQU $FF4B

SECTION "Entry", ROM0[$0100]
    nop
    jp Start

SECTION "Code", ROM0

Start:
    di
    ld sp, $FFFE
    
    ; Éteindre le LCD
    call LcdOff
    
    ; Initialiser les palettes
    ld a, $E4  ; Palette: 11 10 01 00 (blanc, gris clair, gris foncé, noir)
    ld [BGP], a
    
    ; Effacer la tilemap
    call ClearTilemap
    
    ; Dessiner la croix
    call DrawCross
    
    ; Activer le LCD
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

ClearTilemap:
    ld hl, $9800  ; Tilemap
    ld bc, $0400  ; 32x32 = 1024 tiles
    ld a, 0       ; Tile vide
.clear_loop:
    ld [hl+], a
    dec bc
    ld a, b
    or c
    jr nz, .clear_loop
    ret

DrawCross:
    ; Dessiner la diagonale principale (de haut-gauche à bas-droite)
    ld hl, $9800  ; Début de la tilemap
    ld b, 20      ; Hauteur de la croix
    ld c, 0       ; Colonne de départ
.draw_main_diag:
    ; Calculer l'adresse: $9800 + (b-1) * 32 + (b-1)
    ld a, b
    dec a
    ld d, a       ; d = b-1 (ligne)
    ld e, a       ; e = b-1 (colonne)
    
    ; Calculer offset: d * 32 + e
    sla d         ; d * 2
    sla d         ; d * 4
    sla d         ; d * 8
    sla d         ; d * 16
    sla d         ; d * 32
    ld a, d
    add a, e      ; d * 32 + e
    
    ; Adresse finale
    ld e, a
    ld d, 0
    add hl, de
    
    ; Dessiner le pixel (tile 1 = carré noir)
    ld a, 1
    ld [hl], a
    
    ; Retour à la position de départ
    ld hl, $9800
    dec b
    jr nz, .draw_main_diag
    
    ; Dessiner la diagonale secondaire (de haut-droite à bas-gauche)
    ld hl, $9800  ; Début de la tilemap
    ld b, 20      ; Hauteur de la croix
    ld c, 19      ; Colonne de départ (droite)
.draw_sec_diag:
    ; Calculer l'adresse: $9800 + (b-1) * 32 + (19-(b-1))
    ld a, b
    dec a
    ld d, a       ; d = b-1 (ligne)
    ld e, 19
    ld a, e
    sub a, d      ; e = 19 - (b-1) (colonne)
    ld e, a
    
    ; Calculer offset: d * 32 + e
    sla d         ; d * 2
    sla d         ; d * 4
    sla d         ; d * 8
    sla d         ; d * 16
    sla d         ; d * 32
    ld a, d
    add a, e      ; d * 32 + e
    
    ; Adresse finale
    ld e, a
    ld d, 0
    add hl, de
    
    ; Dessiner le pixel (tile 1 = carré noir)
    ld a, 1
    ld [hl], a
    
    ; Retour à la position de départ
    ld hl, $9800
    dec b
    jr nz, .draw_sec_diag
    
    ret

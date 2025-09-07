; RGBDS hello (serial) – envoie une chaîne sur le port série puis boucle

DEF SB EQU $FF01
DEF SC EQU $FF02

SECTION "Entry", ROM0[$0100]
    nop
    jp Start

SECTION "Code", ROM0

Start:
    di
    ld sp, $FFFE
    
    ; Envoyer chaque caractère directement
    ld a, 'H'
    call SendChar
    call Delay
    
    ld a, 'e'
    call SendChar
    call Delay
    
    ld a, 'l'
    call SendChar
    call Delay
    
    ld a, 'l'
    call SendChar
    call Delay
    
    ld a, 'o'
    call SendChar
    call Delay
    
    ld a, ','
    call SendChar
    call Delay
    
    ld a, ' '
    call SendChar
    call Delay
    
    ld a, 'R'
    call SendChar
    call Delay
    
    ld a, 'G'
    call SendChar
    call Delay
    
    ld a, 'B'
    call SendChar
    call Delay
    
    ld a, 'D'
    call SendChar
    call Delay
    
    ld a, 'S'
    call SendChar
    call Delay
    
    ld a, '!'
    call SendChar
    call Delay
    
    ld a, 10  ; \n
    call SendChar
    call Delay

.loop:
    jr .loop

; A = octet à envoyer via lien série interne
SendChar:
    ld [SB], a
    ld a, $81 ; start transfer: internal clock
    ld [SC], a
.busy:
    ld a, [SC]
    bit 7, a
    jr nz, .busy
    ret

; Délai simple
Delay:
    ld bc, $1000
.delay_loop:
    dec bc
    ld a, b
    or c
    jr nz, .delay_loop
    ret



; RGBDS hello (serial debug) – envoie un caractère à la fois avec délai

DEF SB EQU $FF01
DEF SC EQU $FF02

SECTION "Entry", ROM0[$0100]
    nop
    jp Start

SECTION "Code", ROM0

Start:
    di
    ld sp, $FFFE
    
    ; Envoyer 'H'
    ld a, 'H'
    call SendChar
    call Delay
    
    ; Envoyer 'e'
    ld a, 'e'
    call SendChar
    call Delay
    
    ; Envoyer 'l'
    ld a, 'l'
    call SendChar
    call Delay
    
    ; Envoyer 'l'
    ld a, 'l'
    call SendChar
    call Delay
    
    ; Envoyer 'o'
    ld a, 'o'
    call SendChar
    call Delay
    
    ; Envoyer '\n'
    ld a, 10
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

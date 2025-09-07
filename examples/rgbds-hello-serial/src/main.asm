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
    call SendString
.loop:
    jr .loop

; HL -> chaîne terminée par 0x00
SendString:
    ld hl, Message
.next:
    ld a, [hl]
    or a
    jr z, .done
    call SendChar
    inc hl
    jr .next
.done:
    ret

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

Message:
    db "Hello, RGBDS!", 10, 0



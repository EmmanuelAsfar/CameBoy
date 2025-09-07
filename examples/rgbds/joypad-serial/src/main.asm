; RGBDS joypad (serial) – affiche les touches appuyées sur le port série

DEF SB EQU $FF01
DEF SC EQU $FF02
DEF P1 EQU $FF00
DEF IE EQU $FFFF
DEF IF_REG EQU $FF0F

SECTION "Entry", ROM0[$0100]
    nop
    jp Start

SECTION "Code", ROM0

Start:
    di
    ld sp, $FFFE
    
    ; Activer les interruptions
    ld a, 1
    ld [IE], a
    xor a
    ld [IF_REG], a
    ei
    
    ; Message de démarrage
    ld hl, WelcomeMsg
    call SendString
    
    ; Boucle principale
.main_loop:
    call CheckJoypad
    call Delay
    jr .main_loop

; Vérifie l'état du joypad et affiche les touches pressées
CheckJoypad:
    ; Lire les boutons directionnels
    ld a, $20        ; Sélectionner boutons directionnels
    ld [P1], a
    ld a, [P1]       ; Lire plusieurs fois pour stabiliser
    ld a, [P1]
    ld a, [P1]
    cpl              ; Inverser les bits (0 = pressé)
    and $0F          ; Garder seulement les 4 bits directionnels
    ld b, a
    
    ; Lire les boutons d'action
    ld a, $10        ; Sélectionner boutons d'action
    ld [P1], a
    ld a, [P1]       ; Lire plusieurs fois pour stabiliser
    ld a, [P1]
    ld a, [P1]
    cpl              ; Inverser les bits
    and $0F          ; Garder seulement les 4 bits d'action
    sla a            ; Décaler vers la gauche
    sla a
    sla a
    sla a
    or b             ; Combiner avec les directionnels
    ld c, a          ; Sauvegarder l'état complet
    
    ; Vérifier si quelque chose a changé
    ld a, [LastJoypad]
    cp c
    ret z            ; Pas de changement
    
    ; Sauvegarder le nouvel état
    ld a, c
    ld [LastJoypad], a
    
    ; Afficher l'état
    ld a, c
    call SendJoypadState
    ret

; Affiche l'état du joypad sur le port série
; A = état du joypad (bits: Down, Up, Left, Right, Start, Select, B, A)
SendJoypadState:
    push af
    push bc
    push hl
    
    ; Afficher "Joypad: "
    ld hl, JoypadPrefix
    call SendString
    
    ; Afficher les boutons directionnels
    ld a, [LastJoypad]
    and $0F
    ld b, a
    
    ; Down
    bit 3, b
    jr z, .no_down
    ld a, 'D'
    call SendChar
    jr .check_up
.no_down:
    ld a, '-'
    call SendChar
.check_up:
    bit 2, b
    jr z, .no_up
    ld a, 'U'
    call SendChar
    jr .check_left
.no_up:
    ld a, '-'
    call SendChar
.check_left:
    bit 1, b
    jr z, .no_left
    ld a, 'L'
    call SendChar
    jr .check_right
.no_left:
    ld a, '-'
    call SendChar
.check_right:
    bit 0, b
    jr z, .no_right
    ld a, 'R'
    call SendChar
    jr .check_buttons
.no_right:
    ld a, '-'
    call SendChar
    
.check_buttons:
    ld a, ' '
    call SendChar
    
    ; Afficher les boutons d'action
    ld a, [LastJoypad]
    and $F0
    ld b, a
    
    ; Start
    bit 7, b
    jr z, .no_start
    ld a, 'S'
    call SendChar
    jr .check_select
.no_start:
    ld a, '-'
    call SendChar
.check_select:
    bit 6, b
    jr z, .no_select
    ld a, 's'
    call SendChar
    jr .check_b
.no_select:
    ld a, '-'
    call SendChar
.check_b:
    bit 5, b
    jr z, .no_b
    ld a, 'B'
    call SendChar
    jr .check_a
.no_b:
    ld a, '-'
    call SendChar
.check_a:
    bit 4, b
    jr z, .no_a
    ld a, 'A'
    call SendChar
    jr .end_buttons
.no_a:
    ld a, '-'
    call SendChar
    
.end_buttons:
    ld a, 10  ; \n
    call SendChar
    
    pop hl
    pop bc
    pop af
    ret

; Envoie une chaîne de caractères via le port série
; HL = adresse de la chaîne (terminée par 0)
SendString:
    ld a, [hl]
    or a
    ret z
    call SendChar
    inc hl
    jr SendString

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

SECTION "Data", ROM0
WelcomeMsg:
    db "Joypad Test Ready!", 10, 0
JoypadPrefix:
    db "Joypad: ", 0

SECTION "WRAM Vars", WRAM0
LastJoypad: DS 1

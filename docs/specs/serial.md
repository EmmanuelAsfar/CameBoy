# Port SArie a" SpAcifications d'implAmentation

Retour: [Index specs](./README.md) A [Architecture](../architecture.md) A [Utilisation](../usage.md)

## Vue d'ensemble

Le port sArie de la Game Boy permet la communication entre deux Game Boy ou avec des pAriphAriques externes. C'est un composant essentiel pour les jeux multijoueurs.

### Pourquoi un port sArie ?

La Game Boy a AtA conAue pour le multijoueur :
- **Communication** : Achanger des donnAes entre Game Boy
- **PAriphAriques** : Imprimante, camAra, etc.
- **SimplicitA** : Protocole simple et fiable
- **Aconomie** : CoAt rAduit par rapport A  des solutions plus complexes

## Registres du port sArie

### SB (0xFF01) - Serial Data
```c
#define SB_REG 0xFF01

// Registre de donnAes sArie (8 bits)
u8 serial_data;
```

**Pourquoi 8 bits ?** C'est la taille standard pour les communications sArie de l'Apoque.

### SC (0xFF02) - Serial Control
```c
#define SC_REG 0xFF02

// Bits de contrAle
#define SC_INTERNAL_CLOCK 0x80  // Utiliser l'horloge interne
#define SC_CLOCK_SPEED    0x02  // Vitesse (0 = normal, 1 = rapide)
#define SC_TRANSFER_START 0x80  // DAmarrer le transfert
#define SC_TRANSFER_FLAG  0x02  // Transfert en cours
```

**Pourquoi ces bits ?** Chaque bit contrAle un aspect de la communication :
- **INTERNAL_CLOCK** : Utiliser l'horloge interne (vs externe)
- **CLOCK_SPEED** : Vitesse de transfert
- **TRANSFER_START** : DAmarrer un transfert
- **TRANSFER_FLAG** : Indiquer qu'un transfert est en cours

## Protocole de communication

### SAquence de transfert
`````mermaid`r`nsequenceDiagram
    participant GB1 as Game Boy 1
    participant GB2 as Game Boy 2
    
    GB1->>GB1: Acrire donnAes dans SB
    GB1->>GB1: Configurer SC (START + CLOCK)
    GB1->>GB2: Envoyer bit par bit
    GB2->>GB2: Recevoir et stocker dans SB
    GB1->>GB1: Transfert terminA a' IRQ
    GB2->>GB2: Transfert terminA a' IRQ
```

**Pourquoi bit par bit ?** C'est le protocole sArie standard. Les donnAes sont envoyAes un bit A  la fois.

### Vitesses de transfert
```c
// Vitesses de transfert (en Hz)
#define SERIAL_SPEED_NORMAL 8192   // 4.194304 MHz / 512
#define SERIAL_SPEED_FAST   262144 // 4.194304 MHz / 16

// Calcul de la vitesse
u32 get_serial_speed(u8 sc_reg) {
    if (sc_reg & SC_CLOCK_SPEED) {
        return SERIAL_SPEED_FAST;
    } else {
        return SERIAL_SPEED_NORMAL;
    }
}
```

**Pourquoi ces vitesses ?** Elles sont calculAes A  partir de la frAquence CPU (4.194304 MHz) et des diviseurs matAriels.

## Gestion du transfert

### DAmarrage d'un transfert
```c
void serial_start_transfer(MMU* mmu) {
    u8 sc = mmu_read8(mmu, SC_REG);
    
    // VArifier si le transfert est dAjA  en cours
    if (sc & SC_TRANSFER_FLAG) {
        return;  // Transfert dAjA  en cours
    }
    
    // DAmarrer le transfert
    sc |= SC_TRANSFER_START | SC_TRANSFER_FLAG;
    mmu_write8(mmu, SC_REG, sc);
    
    // Initialiser le compteur de bits
    mmu->serial_bit_count = 0;
    mmu->serial_cycles = 0;
}
```

**Pourquoi vArifier le flag ?** Avite de dAmarrer plusieurs transferts simultanAs.

### Mise A  jour du transfert
```c
void serial_tick(MMU* mmu, u8 cycles) {
    u8 sc = mmu_read8(mmu, SC_REG);
    
    // VArifier si un transfert est en cours
    if (!(sc & SC_TRANSFER_FLAG)) {
        return;
    }
    
    // Mettre A  jour le compteur de cycles
    mmu->serial_cycles += cycles;
    
    // Calculer la vitesse de transfert
    u32 speed = get_serial_speed(sc);
    u32 cycles_per_bit = 4194304 / speed;
    
    // VArifier si on doit envoyer le bit suivant
    if (mmu->serial_cycles >= cycles_per_bit) {
        mmu->serial_cycles -= cycles_per_bit;
        serial_send_bit(mmu);
    }
}
```

**Pourquoi compter les cycles ?** Pour respecter la vitesse de transfert exacte.

### Envoi d'un bit
```c
void serial_send_bit(MMU* mmu) {
    u8 sb = mmu_read8(mmu, SB_REG);
    u8 bit = (sb >> (7 - mmu->serial_bit_count)) & 1;
    
    // Envoyer le bit (simulation)
    serial_send_bit_to_external(bit);
    
    // Passer au bit suivant
    mmu->serial_bit_count++;
    
    // VArifier si le transfert est terminA
    if (mmu->serial_bit_count >= 8) {
        serial_complete_transfer(mmu);
    }
}
```

**Pourquoi 8 bits ?** C'est la taille standard d'un octet.

### Fin du transfert
```c
void serial_complete_transfer(MMU* mmu) {
    u8 sc = mmu_read8(mmu, SC_REG);
    
    // Effacer les flags de transfert
    sc &= ~(SC_TRANSFER_START | SC_TRANSFER_FLAG);
    mmu_write8(mmu, SC_REG, sc);
    
    // DAclencher l'interruption sArie
    request_interrupt(mmu, IRQ_SERIAL);
    
    // RAinitialiser le compteur
    mmu->serial_bit_count = 0;
    mmu->serial_cycles = 0;
}
```

**Pourquoi dAclencher une interruption ?** Pour notifier le CPU que le transfert est terminA.

## Communication externe

### Simulation de la communication
```c
typedef struct {
    u8 data;
    u8 bit_count;
    bool connected;
} SerialConnection;

void serial_send_bit_to_external(u8 bit) {
    // Simulation : stocker le bit reAu
    if (serial_connection.connected) {
        serial_connection.data |= (bit << (7 - serial_connection.bit_count));
        serial_connection.bit_count++;
        
        if (serial_connection.bit_count >= 8) {
            // Octet complet reAu
            serial_connection.bit_count = 0;
            serial_connection.data = 0;
        }
    }
}
```

**Pourquoi simuler ?** Dans un Amulateur, on ne peut pas vraiment connecter deux Game Boy physiques.

### Gestion des connexions
```c
void serial_connect(bool connected) {
    serial_connection.connected = connected;
    serial_connection.data = 0;
    serial_connection.bit_count = 0;
}

bool serial_is_connected() {
    return serial_connection.connected;
}
```

**Pourquoi gArer les connexions ?** Pour simuler la prAsence/absence d'une Game Boy connectAe.

## IntAgration avec la MMU

### Lecture du port sArie
```c
u8 mmu_read_serial(MMU* mmu, u16 addr) {
    switch (addr) {
        case SB_REG:
            return mmu->serial_data;
        case SC_REG:
            return mmu->serial_control;
        default:
            return 0xFF;
    }
}
```

**Pourquoi passer par la MMU ?** Le CPU accAde au port sArie via le bus mAmoire.

### Acriture dans le port sArie
```c
void mmu_write_serial(MMU* mmu, u16 addr, u8 value) {
    switch (addr) {
        case SB_REG:
            mmu->serial_data = value;
            break;
        case SC_REG:
            mmu->serial_control = value;
            if (value & SC_TRANSFER_START) {
                serial_start_transfer(mmu);
            }
            break;
    }
}
```

**Pourquoi dAmarrer automatiquement ?** Quand on Acrit SC avec le bit START, le transfert commence immAdiatement.

## Gestion des erreurs

### DAtection d'erreurs
```c
void serial_check_errors(MMU* mmu) {
    u8 sc = mmu_read8(mmu, SC_REG);
    
    // VArifier si le transfert est bloquA
    if (sc & SC_TRANSFER_FLAG) {
        // VArifier le timeout
        if (mmu->serial_cycles > SERIAL_TIMEOUT) {
            serial_abort_transfer(mmu);
        }
    }
}
```

**Pourquoi gArer les erreurs ?** Pour Aviter que l'Amulateur se bloque si un transfert Achoue.

### Abandon d'un transfert
```c
void serial_abort_transfer(MMU* mmu) {
    u8 sc = mmu_read8(mmu, SC_REG);
    
    // Effacer les flags de transfert
    sc &= ~(SC_TRANSFER_START | SC_TRANSFER_FLAG);
    mmu_write8(mmu, SC_REG, sc);
    
    // RAinitialiser le compteur
    mmu->serial_bit_count = 0;
    mmu->serial_cycles = 0;
}
```

**Pourquoi abandonner ?** Pour Aviter que l'Amulateur reste bloquA en cas de problAme.

## Initialisation

```c
void serial_init(MMU* mmu) {
    // Valeurs de power-up
    mmu->serial_data = 0x00;
    mmu->serial_control = 0x7E;  // Pas de transfert en cours
    
    // RAinitialiser les compteurs
    mmu->serial_bit_count = 0;
    mmu->serial_cycles = 0;
    
    // Pas de connexion par dAfaut
    serial_connection.connected = false;
}
```

**Pourquoi ces valeurs ?** Ce sont les valeurs exactes de la Game Boy au dAmarrage.

## Tests de conformitA

### Test de transfert
```c
void test_serial_transfer() {
    MMU mmu;
    serial_init(&mmu);
    
    // Configurer les donnAes
    mmu_write8(&mmu, SB_REG, 0x55);
    
    // DAmarrer le transfert
    mmu_write8(&mmu, SC_REG, 0x81);  // START + INTERNAL_CLOCK
    
    // Simuler le transfert
    for (int i = 0; i < 8; i++) {
        serial_tick(&mmu, 512);  // 8192 Hz
    }
    
    // VArifier que le transfert est terminA
    u8 sc = mmu_read8(&mmu, SC_REG);
    assert(!(sc & SC_TRANSFER_FLAG));
}
```

**Pourquoi ces tests ?** Ils vArifient que le comportement du port sArie est conforme aux spAcifications.

## RAfArences Pan Docs

- [Serial Data Transfer](https://gbdev.io/pandocs/Serial_Data_Transfer.html)

### Modes master/esclave et SC

- `SC` (DMG): bit7=Start, bit0=Source d'horloge (0=externe, 1=interne). En CGB, un mode rapide modifie la vitesse.
- Master (horloge interne): 8192 Hz (DMG) / 262144 Hz (CGB fast); Slave: cadence par l'horloge externe.
- Fin de transfert: 8 bits echanges, `IF.SERIAL` leve, `SC.bit7` remis a 0.

Objectif pedagogique: distinguer clairement master/slave et valider la levee d'IF.SERIAL a la fin du paquet.

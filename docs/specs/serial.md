# Port Série – Spécifications d'implémentation

Retour: [Index specs](./README.md) · [Architecture](../architecture.md) · [Utilisation](../usage.md)

## Vue d'ensemble

Le port série de la Game Boy permet la communication entre deux Game Boy ou avec des périphériques externes. C'est un composant essentiel pour les jeux multijoueurs.

### Pourquoi un port série ?

La Game Boy a été conçue pour le multijoueur :
- **Communication** : Échanger des données entre Game Boy
- **Périphériques** : Imprimante, caméra, etc.
- **Simplicité** : Protocole simple et fiable
- **Économie** : Coût réduit par rapport à des solutions plus complexes

## Registres du port série

### SB (0xFF01) - Serial Data
```c
#define SB_REG 0xFF01

// Registre de données série (8 bits)
u8 serial_data;
```

**Pourquoi 8 bits ?** C'est la taille standard pour les communications série de l'époque.

### SC (0xFF02) - Serial Control
```c
#define SC_REG 0xFF02

// Bits de contrôle
#define SC_INTERNAL_CLOCK 0x80  // Utiliser l'horloge interne
#define SC_CLOCK_SPEED    0x02  // Vitesse (0 = normal, 1 = rapide)
#define SC_TRANSFER_START 0x80  // Démarrer le transfert
#define SC_TRANSFER_FLAG  0x02  // Transfert en cours
```

**Pourquoi ces bits ?** Chaque bit contrôle un aspect de la communication :
- **INTERNAL_CLOCK** : Utiliser l'horloge interne (vs externe)
- **CLOCK_SPEED** : Vitesse de transfert
- **TRANSFER_START** : Démarrer un transfert
- **TRANSFER_FLAG** : Indiquer qu'un transfert est en cours

## Protocole de communication

### Séquence de transfert
```mermaid
sequenceDiagram
    participant GB1 as Game Boy 1
    participant GB2 as Game Boy 2
    
    GB1->>GB1: Écrire données dans SB
    GB1->>GB1: Configurer SC (START + CLOCK)
    GB1->>GB2: Envoyer bit par bit
    GB2->>GB2: Recevoir et stocker dans SB
    GB1->>GB1: Transfert terminé → IRQ
    GB2->>GB2: Transfert terminé → IRQ
```

**Pourquoi bit par bit ?** C'est le protocole série standard. Les données sont envoyées un bit à la fois.

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

**Pourquoi ces vitesses ?** Elles sont calculées à partir de la fréquence CPU (4.194304 MHz) et des diviseurs matériels.

## Gestion du transfert

### Démarrage d'un transfert
```c
void serial_start_transfer(MMU* mmu) {
    u8 sc = mmu_read8(mmu, SC_REG);
    
    // Vérifier si le transfert est déjà en cours
    if (sc & SC_TRANSFER_FLAG) {
        return;  // Transfert déjà en cours
    }
    
    // Démarrer le transfert
    sc |= SC_TRANSFER_START | SC_TRANSFER_FLAG;
    mmu_write8(mmu, SC_REG, sc);
    
    // Initialiser le compteur de bits
    mmu->serial_bit_count = 0;
    mmu->serial_cycles = 0;
}
```

**Pourquoi vérifier le flag ?** Évite de démarrer plusieurs transferts simultanés.

### Mise à jour du transfert
```c
void serial_tick(MMU* mmu, u8 cycles) {
    u8 sc = mmu_read8(mmu, SC_REG);
    
    // Vérifier si un transfert est en cours
    if (!(sc & SC_TRANSFER_FLAG)) {
        return;
    }
    
    // Mettre à jour le compteur de cycles
    mmu->serial_cycles += cycles;
    
    // Calculer la vitesse de transfert
    u32 speed = get_serial_speed(sc);
    u32 cycles_per_bit = 4194304 / speed;
    
    // Vérifier si on doit envoyer le bit suivant
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
    
    // Vérifier si le transfert est terminé
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
    
    // Déclencher l'interruption série
    request_interrupt(mmu, IRQ_SERIAL);
    
    // Réinitialiser le compteur
    mmu->serial_bit_count = 0;
    mmu->serial_cycles = 0;
}
```

**Pourquoi déclencher une interruption ?** Pour notifier le CPU que le transfert est terminé.

## Communication externe

### Simulation de la communication
```c
typedef struct {
    u8 data;
    u8 bit_count;
    bool connected;
} SerialConnection;

void serial_send_bit_to_external(u8 bit) {
    // Simulation : stocker le bit reçu
    if (serial_connection.connected) {
        serial_connection.data |= (bit << (7 - serial_connection.bit_count));
        serial_connection.bit_count++;
        
        if (serial_connection.bit_count >= 8) {
            // Octet complet reçu
            serial_connection.bit_count = 0;
            serial_connection.data = 0;
        }
    }
}
```

**Pourquoi simuler ?** Dans un émulateur, on ne peut pas vraiment connecter deux Game Boy physiques.

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

**Pourquoi gérer les connexions ?** Pour simuler la présence/absence d'une Game Boy connectée.

## Intégration avec la MMU

### Lecture du port série
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

**Pourquoi passer par la MMU ?** Le CPU accède au port série via le bus mémoire.

### Écriture dans le port série
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

**Pourquoi démarrer automatiquement ?** Quand on écrit SC avec le bit START, le transfert commence immédiatement.

## Gestion des erreurs

### Détection d'erreurs
```c
void serial_check_errors(MMU* mmu) {
    u8 sc = mmu_read8(mmu, SC_REG);
    
    // Vérifier si le transfert est bloqué
    if (sc & SC_TRANSFER_FLAG) {
        // Vérifier le timeout
        if (mmu->serial_cycles > SERIAL_TIMEOUT) {
            serial_abort_transfer(mmu);
        }
    }
}
```

**Pourquoi gérer les erreurs ?** Pour éviter que l'émulateur se bloque si un transfert échoue.

### Abandon d'un transfert
```c
void serial_abort_transfer(MMU* mmu) {
    u8 sc = mmu_read8(mmu, SC_REG);
    
    // Effacer les flags de transfert
    sc &= ~(SC_TRANSFER_START | SC_TRANSFER_FLAG);
    mmu_write8(mmu, SC_REG, sc);
    
    // Réinitialiser le compteur
    mmu->serial_bit_count = 0;
    mmu->serial_cycles = 0;
}
```

**Pourquoi abandonner ?** Pour éviter que l'émulateur reste bloqué en cas de problème.

## Initialisation

```c
void serial_init(MMU* mmu) {
    // Valeurs de power-up
    mmu->serial_data = 0x00;
    mmu->serial_control = 0x7E;  // Pas de transfert en cours
    
    // Réinitialiser les compteurs
    mmu->serial_bit_count = 0;
    mmu->serial_cycles = 0;
    
    // Pas de connexion par défaut
    serial_connection.connected = false;
}
```

**Pourquoi ces valeurs ?** Ce sont les valeurs exactes de la Game Boy au démarrage.

## Tests de conformité

### Test de transfert
```c
void test_serial_transfer() {
    MMU mmu;
    serial_init(&mmu);
    
    // Configurer les données
    mmu_write8(&mmu, SB_REG, 0x55);
    
    // Démarrer le transfert
    mmu_write8(&mmu, SC_REG, 0x81);  // START + INTERNAL_CLOCK
    
    // Simuler le transfert
    for (int i = 0; i < 8; i++) {
        serial_tick(&mmu, 512);  // 8192 Hz
    }
    
    // Vérifier que le transfert est terminé
    u8 sc = mmu_read8(&mmu, SC_REG);
    assert(!(sc & SC_TRANSFER_FLAG));
}
```

**Pourquoi ces tests ?** Ils vérifient que le comportement du port série est conforme aux spécifications.

## Références Pan Docs

- [Serial Data Transfer](https://gbdev.io/pandocs/Serial_Data_Transfer.html)

### Modes master/esclave et SC

- `SC` (DMG): bit7=Start, bit0=Source d�horloge (0=externe, 1=interne). En CGB, un mode rapide modifie la vitesse.
- Master (horloge interne): 8192 Hz (DMG) / 262144 Hz (CGB fast); Slave: cadenc� par l�horloge externe.
- Fin de transfert: 8 bits �chang�s, `IF.SERIAL` lev�, `SC.bit7` remis � 0.

Objectif p�dagogique: distinguer clairement master/slave et valider la lev�e d�IF.SERIAL � la fin du paquet.

# Tests Unitaires CameBoy

Ce répertoire contient des tests unitaires pour valider chaque composant de l'émulateur Game Boy CameBoy.

## Structure

```
tests/unit/
├── test_cpu.c         # Tests pour le CPU LR35902
├── test_mmu.c         # Tests pour le Memory Management Unit
├── test_ppu.c         # Tests pour le Picture Processing Unit
├── test_timer.c       # Tests pour le système de timers
├── test_interrupt.c   # Tests pour le système d'interruptions
├── test_joypad.c      # Tests pour le contrôleur joypad
├── run_all_tests.sh   # Script pour lancer tous les tests
└── README.md          # Ce fichier
```

## Compilation et Exécution

### Lancer tous les tests
```bash
cd tests/unit
chmod +x run_all_tests.sh
./run_all_tests.sh
```

### Lancer un test spécifique
```bash
# Compiler
gcc -I ../../src -o test_cpu test_cpu.c ../../src/cpu.c ../../src/cpu_tables.c ../../src/mmu.c ../../src/common.c

# Exécuter
./test_cpu
```

## Couverture des Tests

### CPU (`test_cpu.c`)
- ✅ Initialisation et reset
- ✅ Gestion des registres et flags
- ✅ Instructions arithmétiques (ADD, SUB, ADC, SBC)
- ✅ Instructions logiques (AND, OR, XOR, CP)
- ✅ Instructions de chargement (LD r8,r8, LD r8,n8, LD r16,n16)
- ✅ Instructions de saut (JR avec conditions)
- ✅ Instructions de pile (PUSH/POP)
- ✅ Gestion des interruptions

### MMU (`test_mmu.c`)
- ✅ Initialisation et reset
- ✅ Mapping mémoire (ROM, VRAM, ERAM, WRAM, OAM, IO, HRAM)
- ✅ Parsing des en-têtes de cartouche
- ✅ Lecture/écriture 8-bit et 16-bit
- ✅ Echo RAM (miroir WRAM)

### PPU (`test_ppu.c`)
- ✅ Initialisation et reset
- ✅ Gestion des registres LCD
- ✅ Modes PPU (OAM Search, Pixel Transfer, HBlank, VBlank)
- ✅ Transitions entre modes
- ✅ Rendu de lignes
- ✅ Gestion des palettes

### Timer (`test_timer.c`)
- ✅ Initialisation et reset
- ✅ Compteur DIV (16384Hz)
- ✅ Compteur TIMA avec différentes fréquences
- ✅ Gestion des overflows et reload
- ✅ Contrôle TAC

### Interruptions (`test_interrupt.c`)
- ✅ Initialisation
- ✅ Demande et effacement d'interruptions
- ✅ Gestion des priorités (VBlank > LCD > Timer > Serial > Joypad)
- ✅ Activation/désactivation
- ✅ Routine de service d'interruption

### Joypad (`test_joypad.c`)
- ✅ Initialisation et reset
- ✅ Écriture dans P1 (sélection lignes)
- ✅ Lecture avec masquage
- ✅ Gestion boutons (A, B, Start, Select)
- ✅ Gestion directions (Up, Down, Left, Right)
- ✅ Entrées mixtes

## Conformité

Tous les tests vérifient la conformité avec les spécifications **Pan Docs** :
- Registres aux bonnes adresses
- Flags dans le bon ordre de bits
- Interruptions avec les bonnes priorités
- Timings corrects selon les spécifications
- Formats de données conformes (little-endian, etc.)

## Résultats Attendus

Chaque test affiche :
```
Test 1: Nom du test... PASS
```

À la fin :
```
=== RÉSULTATS ===
Tests passés: X/Y
✅ TOUS LES TESTS SONT PASSÉS !
```

## Debugging

Si un test échoue, le programme s'arrête avec `assert()`. Pour debugger :
1. Lancer le test individuellement
2. Ajouter des `printf()` pour tracer l'exécution
3. Vérifier les valeurs attendues vs obtenues

## Extension

Pour ajouter de nouveaux tests :
1. Créer `test_nouveau.c`
2. Implémenter la fonction `main()` avec le framework de test
3. Ajouter au script `run_all_tests.sh`
4. Mettre à jour ce README

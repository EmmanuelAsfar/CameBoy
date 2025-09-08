# Spécifications d'implémentation CameBoy

Cette section synthétise, en français et de façon didactique, les règles à appliquer pour coder l'émulateur, avec explications « expert » et « non-expert » (déjargonisées) et schémas.

## Pourquoi cette approche ?

L'émulation Game Boy nécessite de reproduire fidèlement le comportement matériel. Chaque détail compte car les jeux s'appuient sur des comportements spécifiques, parfois non documentés, pour fonctionner correctement.

## Structure des spécifications

- [Mémoire (Memory Map)](./memory.md) - Comment organiser l'accès aux données
- [CPU (LR35902)](./cpu.md) - Le cerveau de la Game Boy
- [PPU (Vidéo)](./ppu.md) - Comment dessiner l'écran
- [Timers](./timers.md) - Compteurs et minuteries
- [Interruptions](./interrupts.md) - Gestion des événements
- [Joypad](./joypad.md) - Contrôles utilisateur
- [Port Série](./serial.md) - Communication externe
- [MBC (Cartouches)](./mbc.md) - Gestion des cartouches
- [DMA/OAM](./dma.md) - Transferts de données rapides
- [Accès VRAM/OAM](./vram-access.md) - Restrictions d'accès
- [Séquence de démarrage](./power-up.md) - Initialisation système
- [APU (Audio)](./apu.md) - Génération sonore et registres
- [Bug de corruption OAM](./oam-bug.md) - Cas limites PPU
- [CGB (Game Boy Color)](./cgb.md) - Différences et cadrage

## Pour en savoir plus (optionnel)

- Super Game Boy (SGB): commandes et transferts
- Infrarouge (CGB): communication IR
- Accessoires: Imprimante, CamAra, Adaptateur 4 joueurs
- Connectique: ports externes et cAblage
- Cheats: Game Genie/Shark
- Comparaison LR35902 vs Z80

RAfs correspondantes: voir [Pan Docs](https://gbdev.io/pandocs/)

## Comment utiliser ces spAcifications

1. **Lisez d'abord** la section "Pourquoi Aa fonctionne comme Aa" pour comprendre le contexte
2. **Consultez** les schAmas pour visualiser les flux
3. **ImplAmentez** Atape par Atape en respectant les contraintes
4. **Testez** avec les suites Blargg/Mooneye pour valider la conformitA

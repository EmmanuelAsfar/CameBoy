# SpAcifications d'implAmentation CameBoy

Cette section synthAtise, en franAais et de faAon didactique, les rAgles A  appliquer pour coder l'Amulateur, avec explications A expert A et A non-expert A (dAjargonisAes) et schAmas.

## Pourquoi cette approche ?

L'Amulation Game Boy nAcessite de reproduire fidAlement le comportement matAriel. Chaque dAtail compte car les jeux s'appuient sur des comportements spAcifiques, parfois non documentAs, pour fonctionner correctement.

## Structure des spAcifications

- [MAmoire (Memory Map)](./memory.md) - Comment organiser l'accAs aux donnAes
- [CPU (LR35902)](./cpu.md) - Le cerveau de la Game Boy
- [PPU (VidAo)](./ppu.md) - Comment dessiner l'Acran
- [Timers](./timers.md) - Compteurs et minuteries
- [Interruptions](./interrupts.md) - Gestion des AvAnements
- [Joypad](./joypad.md) - ContrAles utilisateur
- [Port SArie](./serial.md) - Communication externe
- [MBC (Cartouches)](./mbc.md) - Gestion des cartouches
- [DMA/OAM](./dma.md) - Transferts de donnAes rapides
- [AccAs VRAM/OAM](./vram-access.md) - Restrictions d'accAs
- [SAquence de dAmarrage](./power-up.md) - Initialisation systAme
- [APU (Audio)](./apu.md) - GAnAration sonore et registres
- [Bug de corruption OAM](./oam-bug.md) - Cas limites PPU
- [CGB (Game Boy Color)](./cgb.md) - DiffArences et cadrage

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

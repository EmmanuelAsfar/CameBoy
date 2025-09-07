Retour à l’index: [docs/README.md](./README.md) · [Architecture](./architecture.md) · [Utilisation](./usage.md) · [Tests](./testing.md) · [Scripts](./scripts.md)

### Glossaire CameBoy (jargon expliqué)

- **DMG**: modèle original de la Game Boy (Dot Matrix Game). Cible principale de l’émulation.
- **LR35902**: CPU hybride 8-bit (mélange 8080/Z80) utilisé par la Game Boy.
- **Opcode**: code binaire d’une instruction CPU (ex: `0x3E` pour `LD A,n`).
- **Flags (Z N H C)**: bits d’état du CPU: Zero, Substract, Half-carry, Carry.
- **MMU**: Memory Management Unit. Acheminement des lectures/écritures vers les bons périphériques mémoires.
- **ROM/VRAM/WRAM/ERAM/OAM/HRAM**: différentes zones mémoire (programme, vidéo, travail, externe, sprites, haute RAM).
- **IO Registers**: registres de périphériques mappés mémoire (ex: `0xFF40` `LCDC`, `0xFF41` `STAT`).
- **PPU**: Picture Processing Unit. Gère le pipeline vidéo, produit le framebuffer 160x144.
- **Modes PPU**: OAM Search (2), Pixel Transfer (3), HBlank (0), VBlank (1).
- **VBlank**: période d’inactivité vidéo (lignes 144–153) pour mettre à jour l’écran.
- **STAT**: registre d’état LCD; signale le mode et peut générer des IRQ.
- **Timer (DIV/TIMA/TMA/TAC)**: minuteries; `TIMA` déborde et déclenche une IRQ Timer; `DIV` est un compteur de base.
- **IRQ / Interrupt**: interruption matérielle; pause le CPU pour exécuter une routine (VBlank, LCD, Timer, Série, Joypad).
- **IME/EI/DI**: Interrupt Master Enable; `EI` active les interruptions avec un délai d’une instruction; `DI` les désactive.
- **HALT bug**: comportement particulier quand des IRQ sont en attente et IME=0: `PC` peut ne pas s’incrémenter.
- **P1 (Joypad)**: registre `0xFF00` sélectionnant la ligne Boutons ou Directions et lisant 4 bits actifs bas.
- **MBC**: Memory Bank Controller (MBC1, MBC3…). Permet de « bank-switcher » de la ROM/ERAM.
- **APU**: Audio Processing Unit (canaux 1–4, NRxx). Partiellement implémenté ici.
- **Framebuffer**: tampon d’image (tableau de pixels) rendu par le PPU et affiché par la GUI.
- **Pan Docs**: documentation de référence de la Game Boy (présente sous `docs/pandocs/`).
- **Blargg/Mooneye**: suites de ROMs de tests pour valider CPU/timings/PPU/IRQ.



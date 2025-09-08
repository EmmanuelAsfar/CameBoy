Retour A  laindex: [docs/README.md](./README.md) A [Architecture](./architecture.md) A [Utilisation](./usage.md) A [Tests](./testing.md) A [Scripts](./scripts.md)

### Glossaire CameBoy (jargon expliquA)

- **DMG**: modAle original de la Game Boy (Dot Matrix Game). Cible principale de laAmulation.
- **LR35902**: CPU hybride 8-bit (mAlange 8080/Z80) utilisA par la Game Boy.
- **Opcode**: code binaire daune instruction CPU (ex: `0x3E` pour `LD A,n`).
- **Flags (Z N H C)**: bits daAtat du CPU: Zero, Substract, Half-carry, Carry.
- **MMU**: Memory Management Unit. Acheminement des lectures/Acritures vers les bons pAriphAriques mAmoires.
- **ROM/VRAM/WRAM/ERAM/OAM/HRAM**: diffArentes zones mAmoire (programme, vidAo, travail, externe, sprites, haute RAM).
- **IO Registers**: registres de pAriphAriques mappAs mAmoire (ex: `0xFF40` `LCDC`, `0xFF41` `STAT`).
- **PPU**: Picture Processing Unit. GAre le pipeline vidAo, produit le framebuffer 160x144.
- **Modes PPU**: OAM Search (2), Pixel Transfer (3), HBlank (0), VBlank (1).
- **VBlank**: pAriode dainactivitA vidAo (lignes 144a"153) pour mettre A  jour laAcran.
- **STAT**: registre daAtat LCD; signale le mode et peut gAnArer des IRQ.
- **Timer (DIV/TIMA/TMA/TAC)**: minuteries; `TIMA` dAborde et dAclenche une IRQ Timer; `DIV` est un compteur de base.
- **IRQ / Interrupt**: interruption matArielle; pause le CPU pour exAcuter une routine (VBlank, LCD, Timer, SArie, Joypad).
- **IME/EI/DI**: Interrupt Master Enable; `EI` active les interruptions avec un dAlai daune instruction; `DI` les dAsactive.
- **HALT bug**: comportement particulier quand des IRQ sont en attente et IME=0: `PC` peut ne pas saincrAmenter.
- **P1 (Joypad)**: registre `0xFF00` sAlectionnant la ligne Boutons ou Directions et lisant 4 bits actifs bas.
- **MBC**: Memory Bank Controller (MBC1, MBC3a). Permet de A bank-switcher A de la ROM/ERAM.
- **APU**: Audio Processing Unit (canaux 1a"4, NRxx). Partiellement implAmentA ici.
- **Framebuffer**: tampon daimage (tableau de pixels) rendu par le PPU et affichA par la GUI.
- **Pan Docs**: documentation de rAfArence de la Game Boy (prAsente sous `docs/pandocs/`).
- **Blargg/Mooneye**: suites de ROMs de tests pour valider CPU/timings/PPU/IRQ.



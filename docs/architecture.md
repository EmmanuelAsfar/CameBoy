Retour A  laindex: [docs/README.md](./README.md) A [Utilisation](./usage.md) A [Tests](./testing.md) A [Scripts](./scripts.md) A [Glossaire](./glossaire.md)

### Architecture de CameBoy

Ce document dAcrit l'architecture logique de l'Amulateur Game Boy CameBoy, en expliquant les composants, leurs responsabilitAs et leurs interactions. Le jargon est dAfini au fur et A  mesure.

### Vue d'ensemble

- **CPU (LR35902)**: processeur 8-bit de la Game Boy. Il exAcute les instructions (fetch/decode/execute), gAre les registres et les drapeaux (flags Z/N/H/C).
- **MMU (Memory Management Unit)**: bus mAmoire. Il mappe les adresses vers les bonnes zones (ROM, VRAM, WRAM, IO, etc.), gAre les accAs aux registres IO et le chargement de ROM.
- **PPU (Picture Processing Unit)**: moteur vidAo. Il gAnAre l'image (160x144) ligne par ligne et gAre les modes (OAM, Transfert, HBlank, VBlank).
- **Timer (DIV/TIMA/TMA/TAC)**: minuteries systAme. Le timer peut dAclencher des interruptions A  des frAquences prAcises.
- **Interrupt**: gestionnaire d'interruptions. Priorise et sert VBlank, LCD STAT, Timer, SArie, Joypad.
- **Joypad**: gestion des entrAes utilisateur via le registre `P1` (sAlection lignes boutons/directions) et gAnAration d'IRQ Joypad.
- **APU (Audio Processing Unit)**: audio (partiel/placeholder dans ce projet).
- **Couche OS/GUI**: rendu Win32, fenAtre GUI et contrAles (option GUI), version simple console (option RUN).

### Diagramme des dApendances (logiques)

```
graph LR
  subgraph Core
    CPU --- MMU
    CPU --- Timer
    CPU --- Interrupt
    MMU --- PPU
    MMU --- Joypad
    MMU --- Interrupt
  end

  subgraph IO
    PPU ---|framebuffer| GUI[Graphics Win32 GUI]
    Joypad ---|input/events| GUI
  end

  Emulator[Emulator] --> CPU
  Emulator --> Timer
  Emulator --> PPU
  Emulator --> Joypad
  Emulator --> Interrupt
  Emulator -. chargement ROM .-> MMU
```

### Flux d'exAcution (simplifiA)

```
flowchart TD
  A[Chargement ROM] --> B[Initialisation modules]
  B --> C{Boucle principale}
  C -->|par instruction| D[CPU step]
  D --> E[Timer tick]
  D --> F[PPU tick]
  E --> G[IRQ Timer?]
  F --> H[IRQ VBlank/STAT?]
  G --> I[Interrupt handle]
  H --> I
  I --> C
  F --> J[Frame ready?]
  J -->|oui| K[GUI update/present]
  K --> C
```

### Composants et responsabilitAs

- **`src/cpu.h` / `src/cpu.c`**
  - DAcodage via tables `cpu_tables.c` et `cpu_tables_cb.c` (prAfixe CB).
  - Respect des drapeaux: Z (Zero), N (Substract), H (Half-carry), C (Carry).
  - Points d'attention: dAlai d'activation `EI` (prend effet aprAs l'instruction suivante), bug `HALT` (PC peut ne pas s'incrAmenter si des IRQ en attente avec IME=0).

- **`src/mmu.h` / `src/mmu.c`**
  - Mapping mAmoire: ROM(0x0000-7FFF) / VRAM(0x8000-9FFF) / ERAM / WRAM / OAM / IO / HRAM.
  - Registres IO principaux exposAs dans `src/common.h` (ex: `LCDC`, `STAT`, `DIV`, `TIMA`, `IE`, `IF`).
  - Chargement de ROM, appel du callback sArie (redirige vers GUI) et intAraction avec Joypad/PPU/Timer.

- **`src/ppu.h` / `src/ppu.c`**
  - Pipeline vidAo: 456 cycles par ligne, 144 lignes visibles, 10 lignes VBlank.
  - Modes: OAM Search a' Pixel Transfer a' HBlank a' VBlank, avec transitions STAT.
  - Sortie: `framebuffer` 160x144 en pixels 32-bit, consommA par la GUI.

- **`src/timer.h` / `src/timer.c`**
  - Registres: `DIV`, `TIMA`, `TMA`, `TAC`.
  - Overflow de `TIMA` a' reload depuis `TMA` et demande d'IRQ Timer.

- **`src/interrupt.h` / `src/interrupt.c`**
  - Bits d'`IF`/`IE`, prioritAs, acquittement et saut vers routines d'interruption.

- **`src/joypad.h` / `src/joypad.c`**
  - Registre `P1` (sAlection lignes boutons/directions et lecture 4 bits bas).
  - DAclenchement IRQ Joypad via callback vers MMU.

- **`src/graphics_win32_gui.*`**
  - FenAtre Win32 avec 3 zones: console (fond + LCD), panneau sArie, panneau logs, et une barre d'actions (boutons A/B/Start/Select/D-pad).
  - Affichage du `framebuffer` PPU et gestion des entrAes clavier/boutons.
  - Les logs GUI proviennent de la redirection stdout/stderr (voir exAcution) et du callback sArie.

- **`src/emulator_win32_gui.c`**
  - IntAgre tous les modules, redirige stdout/stderr vers le panneau logs, et publie la sortie sArie dans le panneau sArie.
  - Boucle d'Amulation: `cpu_step` a' `timer_tick` a' `ppu_tick` a' rendu pAriodique (a60 FPS) a' gestion AvAnements.

- **`src/emulator_simple.c`**
  - Version console minimale (sans GUI) pour lancer une ROM, produire des logs et Aventuellement des captures simples.

### Diagramme sAquence (AvAnements clAs)

```
sequenceDiagram
  participant GUI as GUI Win32
  participant Joy as Joypad
  participant MMU as MMU
  participant CPU as CPU
  participant Tim as Timer
  participant PPU as PPU

  GUI->>Joy: press(A)
  Joy->>MMU: set P1 bits + request JOYPAD IRQ
  MMU->>CPU: IF |= JOYPAD
  loop Main loop
    CPU->>CPU: cpu_step()
    CPU->>Tim: cycles a' timer_tick()
    CPU->>PPU: cycles a' ppu_tick()
    alt TIMA overflow
      Tim->>MMU: IF |= TIMER
    end
    alt LY==144
      PPU->>MMU: IF |= VBLANK
    end
  end
```

### Ports, logs et ressources

- Les ressources GUI (fond Game Boy) se trouvent dans `resources/` et sont copiAes en `build/resources/` au build.
- La GUI capte la sortie standard et d'erreur pour les afficher dans le panneau de logs, et le port sArie est affichA sAparAment.
- Les logs par ROM sont Acrits sous `logs/rom/<romname>/` lors du chargement dans la GUI.

### Limitations/points d'attention actuels

- PPU/Timer/Joypad peuvent nAcessiter des ajustements de timing pour satisfaire Blargg/Mooneye (voir `README_AGENT.md`).
- DMA OAM et certains MBC ne sont pas implAmentAs/complAtAs.



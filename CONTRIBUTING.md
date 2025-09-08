### Contribuer Ã  CameBoy

**Note agents IA:** merci de lire attentivement `AGENT_POLICY.md` et `STATUS.md` avant de contribuer. Ces documents dÃ©crivent la politique de travail (spec-first, docs/tests synchronisÃ©s) et l'Ã©tat courant du projet.

Merci de votre intÃ©rÃªt ! Ce guide explique le workflow, le style de code et les attentes en matiÃ¨re de tests et de documentation.

### PrÃ©-requis

- Windows avec `gcc` (MinGW/TDM-GCC) dans le PATH.
- Optionnel: `make`, `git`.

### Workflow Git

1. CrÃ©ez une branche: `git checkout -b feat/ma-fonction`.
2. DÃ©veloppez par petits commits, en gardant les tests verts.
3. ExÃ©cutez `cameboy.bat test` et vÃ©rifiez `logs/test_results.log`.
4. RÃ©digez des messages de commit clairs (convention Â« type: rÃ©sumÃ© Â»).
5. Ouvrez une Pull Request, rÃ©fÃ©rencez les issues, dÃ©crivez lâ€™impact.

### Style & qualitÃ©

- C99 strict, pas de comportements indÃ©finis, erreurs explicites.
- Noms de variables en anglais; commentaires mÃ©tier en franÃ§ais.
- Respecter la sÃ©paration des responsabilitÃ©s (CPU/MMU/PPU/Timer/Interrupt/Joypad/APU).
- Logs utiles, sans bruit excessif.

### Tests

- Ajoutez/maintenez des tests unitaires pertinents dans `tests/unit/`.
- Assurez-vous que `cameboy.bat test` affiche Â« TOUS LES TESTS REUSSIS! Â».
- Pour des changements CPU/Timer/PPU/IRQ, considÃ©rez les ROMs Blargg/Mooneye.

### Documentation

- Mettez Ã  jour `docs/architecture.md`, `docs/usage.md`, `docs/testing.md` si lâ€™API ou les scripts changent.
- Ajoutez les dÃ©finitions au `docs/glossaire.md` en cas de nouveau jargon.

### Build & scripts

- Utilisez `cameboy.bat` (Windows) pour build/test/run;
- VÃ©rifiez que `resources/` se copie en `build/resources/` si GUI requise.

### Notes Pan Docs (piÃ¨ges)

- `EI` prend effet aprÃ¨s lâ€™instruction suivante.
- Bug `HALT`: PC peut ne pas sâ€™incrÃ©menter si IRQ en attente avec IME=0.
- PPU: 456 cycles/ligne; VBlank 144â€“153; transitions OAMâ†’XFERâ†’HBlank.
- DMA OAM: 160 octets; bloque OAM, accÃ¨s partiellement restreints.

### État du projet

Consultez STATUS.md pour connaître l'état actuel (tests OK/KO, chantiers en cours) et les prochaines étapes.

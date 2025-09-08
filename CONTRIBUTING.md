### Contribuer à CameBoy

**Note agents IA:** merci de lire attentivement `AGENT_POLICY.md` et `STATUS.md` avant de contribuer. Ces documents décrivent la politique de travail (spec-first, docs/tests synchronisés) et l'état courant du projet.

Merci de votre intérêt ! Ce guide explique le workflow, le style de code et les attentes en matière de tests et de documentation.

### Pré-requis

- Windows avec `gcc` (MinGW/TDM-GCC) dans le PATH.
- Optionnel: `make`, `git`.

### Workflow Git

1. Créez une branche: `git checkout -b feat/ma-fonction`.
2. Développez par petits commits, en gardant les tests verts.
3. Exécutez `cameboy.bat test` et vérifiez `logs/test_results.log`.
4. Rédigez des messages de commit clairs (convention « type: résumé »).
5. Ouvrez une Pull Request, référencez les issues, décrivez l'impact.

### Style & qualité

- C99 strict, pas de comportements indéfinis, erreurs explicites.
- Noms de variables en anglais; commentaires métier en français.
- Respecter la séparation des responsabilités (CPU/MMU/PPU/Timer/Interrupt/Joypad/APU).
- Logs utiles, sans bruit excessif.

### Tests

- Ajoutez/maintenez des tests unitaires pertinents dans `tests/unit/`.
- Assurez-vous que `cameboy.bat test` affiche « TOUS LES TESTS REUSSIS! ».
- Pour des changements CPU/Timer/PPU/IRQ, considérez les ROMs Blargg/Mooneye.

### Documentation

- Mettez à jour `docs/architecture.md`, `docs/usage.md`, `docs/testing.md` si l'API ou les scripts changent.
- Ajoutez les définitions au `docs/glossaire.md` en cas de nouveau jargon.

### Build & scripts

- Utilisez `cameboy.bat` (Windows) pour build/test/run;
- Vérifiez que `resources/` se copie en `build/resources/` si GUI requise.

### Notes Pan Docs (pièges)

- `EI` prend effet après l'instruction suivante.
- Bug `HALT`: PC peut ne pas s'incrémenter si IRQ en attente avec IME=0.
- PPU: 456 cycles/ligne; VBlank 144–153; transitions OAM→XFER→HBlank.
- DMA OAM: 160 octets; bloque OAM, accès partiellement restreints.

### État du projet

Consultez STATUS.md pour connaître l'état actuel (tests OK/KO, chantiers en cours) et les prochaines étapes.

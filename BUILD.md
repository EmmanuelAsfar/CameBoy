# CameBoy - Guide de Build et DevOps

## Vue d'ensemble

CameBoy utilise une approche DevOps moderne avec :
- **Structure organisée** : séparation claire des responsabilités
- **Build automatisé** : Makefile + scripts cross-platform
- **Tests unitaires** : couverture complète avec rapports
- **Gestion des logs** : centralisation et lisibilité
- **CI/CD ready** : prêt pour l'intégration continue

## Structure du projet

```
cameboy/
├── src/                    # Code source
│   ├── *.c *.h            # Composants principaux
│   └── ...
├── tests/
│   ├── unit/              # Tests unitaires
│   │   ├── test_*.c       # Tests individuels
│   │   ├── run_all_tests.* # Scripts de lancement
│   │   └── README.md      # Documentation tests
│   └── blargg/            # Tests de conformité Blargg
│   └── mooneye/           # Tests de conformité Mooneye
├── build/                 # Artefacts de build (généré)
│   ├── obj/               # Fichiers objets
│   └── bin/               # Exécutables
├── dist/                  # Distributions (généré)
├── logs/                  # Logs centralisés (généré)
├── Makefile               # Build system principal
├── build.sh               # Script Linux/Mac
├── build.bat              # Script Windows
├── config.mk              # Configuration centralisée
└── BUILD.md               # Ce fichier
```

## Compilation

### Linux/Mac

```bash
# Compilation complète
make

# Mode debug
make debug

# Mode release
make release

# Nettoyage
make clean

# Utiliser le script shell
./build.sh
./build.sh test
./build.sh clean
```

### Windows

```cmd
REM Compilation complète
build.bat

REM Tests
build.bat test

REM Debug
build.bat debug

REM Nettoyage
build.bat clean
```

## Tests unitaires

### Exécution automatique

```bash
# Avec make (recommandé)
make test

# Avec les scripts
./build.sh test          # Linux/Mac
build.bat test           # Windows
```

### Tests individuels

```bash
# Compiler un test spécifique
make build/test_cpu
make build/test_mmu
# etc.

# Exécuter manuellement
./build/bin/test_cpu
```

### Résultats des tests

Les résultats sont automatiquement collectés dans :
- **Rapport principal** : `logs/test_results.log`
- **Logs de build** : `logs/test_build.log`

Format du rapport :
```
========================================
CameBoy Unit Tests - 06/09/2025
========================================

Running test_cpu...
✅ test_cpu PASSED

Running test_mmu...
✅ test_mmu PASSED

========================================
Total: 6/6 tests passed
🎉 ALL TESTS PASSED!
```

## Développement

### Ajouter un nouveau composant

1. **Créer les fichiers source** :
   ```bash
   touch src/nouveau.c src/nouveau.h
   ```

2. **Ajouter au Makefile** (automatique via wildcards)

3. **Créer les tests** :
   ```bash
   touch tests/unit/test_nouveau.c
   ```

4. **Ajouter aux scripts de test** (automatique via wildcards)

### Configuration

Toutes les configurations sont centralisées dans `config.mk` :
- Chemins des compilateurs
- Flags de compilation
- Structure des dossiers
- Détection d'OS automatique

## CI/CD

### Intégration GitHub Actions

```yaml
# .github/workflows/build.yml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    - name: Install dependencies
      run: sudo apt-get install gcc
    - name: Build
      run: make
    - name: Run tests
      run: make test
    - name: Upload logs
      uses: actions/upload-artifact@v2
      with:
        name: logs
        path: logs/
```

### Intégration Jenkins/GitLab CI

```groovy
// Jenkinsfile
pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                sh 'make clean && make'
            }
        }
        stage('Test') {
            steps {
                sh 'make test'
                archiveArtifacts artifacts: 'logs/*.log', fingerprint: true
            }
        }
    }
    post {
        always {
            publishHTML([
                allowMissing: false,
                alwaysLinkToLastBuild: true,
                keepAll: true,
                reportDir: 'logs',
                reportFiles: 'test_results.log',
                reportName: 'Test Results'
            ])
        }
    }
}
```

## Dépannage

### Erreurs communes

#### "gcc not found"
```bash
# Linux
sudo apt-get install gcc

# macOS
xcode-select --install

# Windows
# Installer TDM-GCC ou MinGW
```

#### "Permission denied" sur les scripts
```bash
chmod +x build.sh clean_old_files.sh
```

#### Tests qui échouent
1. Vérifier les logs : `cat logs/test_results.log`
2. Compiler en debug : `make debug`
3. Utiliser gdb : `gdb build/bin/test_cpu`

### Nettoyage complet

```bash
# Supprimer tous les anciens fichiers
./clean_old_files.sh

# Nettoyer le projet
make clean

# Reconstruction complète
make && make test
```

## Métriques et qualité

### Couverture de code

```bash
# Installation de gcov
sudo apt-get install gcovr

# Compilation avec couverture
make CFLAGS+="--coverage" LDFLAGS+="--coverage"

# Génération du rapport
gcovr -r . --html --html-details -o coverage.html
```

### Analyse statique

```bash
# Avec le Makefile
make analyze

# Avec clang
clang --analyze src/*.c
```

### Performance

```bash
# Profiling
make debug
valgrind --tool=callgrind ./build/bin/cameboy

# Benchmarking
time ./build/bin/cameboy test.gb 1000
```

## Contribution

### Workflow de développement

1. **Créer une branche** : `git checkout -b feature/nouvelle-fonction`
2. **Développer** : modifications + tests
3. **Tester** : `make test`
4. **Commiter** : `git commit -m "feat: nouvelle fonction"`
5. **Push** : `git push origin feature/nouvelle-fonction`
6. **PR** : créer une Pull Request

### Standards de code

- **C99** strict
- **Warnings** activés (`-Wall -Wextra`)
- **Tests unitaires** pour chaque fonction
- **Documentation** dans les headers
- **Logs** informatifs

---

*Ce guide est maintenu automatiquement. Dernière mise à jour : $(date)*

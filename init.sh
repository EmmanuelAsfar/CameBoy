#!/bin/bash
# =============================================================================
# CameBoy - Script d'initialisation
# Configuration automatique de l'environnement de développement
# =============================================================================

set -e

# Couleurs
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Fonctions utilitaires
log_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

log_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

log_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

log_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Détection de l'OS
detect_os() {
    case "$(uname -s)" in
        Linux*)     OS="linux" ;;
        Darwin*)    OS="macos" ;;
        CYGWIN*|MINGW*|MSYS*) OS="windows" ;;
        *)          OS="unknown" ;;
    esac
}

# Vérification des dépendances de base
check_basic_deps() {
    log_info "Checking basic dependencies..."

    local missing_deps=()

    # Git
    if ! command -v git >/dev/null 2>&1; then
        missing_deps+=("git")
    fi

    # Make
    if ! command -v make >/dev/null 2>&1; then
        missing_deps+=("make")
    fi

    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        log_error "Missing dependencies: ${missing_deps[*]}"
        log_info "Please install them and run this script again."
        exit 1
    fi

    log_success "Basic dependencies OK"
}

# Installation des dépendances selon l'OS
install_deps() {
    log_info "Installing development dependencies for $OS..."

    case "$OS" in
        linux)
            # Détection de la distribution
            if command -v apt-get >/dev/null 2>&1; then
                # Debian/Ubuntu
                log_info "Installing dependencies with apt-get..."
                sudo apt-get update
                sudo apt-get install -y gcc build-essential
            elif command -v dnf >/dev/null 2>&1; then
                # Fedora/RHEL
                log_info "Installing dependencies with dnf..."
                sudo dnf install -y gcc make
            elif command -v pacman >/dev/null 2>&1; then
                # Arch Linux
                log_info "Installing dependencies with pacman..."
                sudo pacman -S --noconfirm gcc make
            else
                log_warning "Unknown Linux distribution. Please install gcc and make manually."
            fi
            ;;

        macos)
            if ! command -v brew >/dev/null 2>&1; then
                log_info "Installing Homebrew..."
                /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
            fi

            log_info "Installing dependencies with Homebrew..."
            brew install gcc make
            ;;

        windows)
            log_warning "On Windows, please install:"
            log_warning "  - TDM-GCC: https://jmeubank.github.io/tdm-gcc/"
            log_warning "  - Make for Windows"
            log_warning "Then run this script again."
            exit 1
            ;;

        *)
            log_error "Unsupported OS: $OS"
            log_info "Please install gcc and make manually."
            exit 1
            ;;
    esac

    log_success "Dependencies installed"
}

# Configuration des permissions
setup_permissions() {
    log_info "Setting up permissions..."

    # Rendre les scripts exécutables
    chmod +x build.sh 2>/dev/null || true
    chmod +x clean_old_files.sh 2>/dev/null || true
    chmod +x tests/unit/run_all_tests.sh 2>/dev/null || true

    log_success "Permissions set"
}

# Création de la structure de dossiers
create_structure() {
    log_info "Creating directory structure..."

    mkdir -p build/{obj,bin} dist logs

    log_success "Directory structure created"
}

# Test de compilation
test_build() {
    log_info "Testing build system..."

    # Test de compilation basique
    if make --version >/dev/null 2>&1; then
        log_info "Testing Makefile..."
        if make clean >/dev/null 2>&1; then
            log_success "Makefile OK"
        else
            log_warning "Makefile test failed, but continuing..."
        fi
    fi

    # Test du script de build
    if [[ -f "build.sh" ]] && [[ -x "build.sh" ]]; then
        log_info "Testing build.sh..."
        if ./build.sh check >/dev/null 2>&1; then
            log_success "build.sh OK"
        else
            log_warning "build.sh test failed, but continuing..."
        fi
    fi
}

# Nettoyage des anciens fichiers (optionnel)
cleanup_old() {
    if [[ -f "clean_old_files.sh" ]]; then
        log_info "Cleaning old build files..."
        ./clean_old_files.sh >/dev/null 2>&1 || true
        log_success "Old files cleaned"
    fi
}

# Affichage du résumé
show_summary() {
    echo ""
    echo "========================================"
    echo "🎉 CameBoy development environment ready!"
    echo "========================================"
    echo ""
    echo "Quick start:"
    echo "  make           # Build the emulator"
    echo "  make test      # Run unit tests"
    echo "  make clean     # Clean build files"
    echo ""
    echo "Scripts available:"
    echo "  ./build.sh     # Linux/Mac build script"
    echo "  build.bat      # Windows build script"
    echo ""
    echo "Test results: logs/test_results.log"
    echo "Build logs:    logs/compile.log"
    echo ""
    echo "Documentation:"
    echo "  BUILD.md       # Build and DevOps guide"
    echo "  README.md      # Project documentation"
    echo ""
}

# Fonction principale
main() {
    echo "🚀 CameBoy Development Environment Setup"
    echo "========================================"
    echo ""

    detect_os
    log_info "Detected OS: $OS"

    check_basic_deps
    install_deps
    setup_permissions
    create_structure
    cleanup_old
    test_build

    show_summary
}

# Gestion des arguments
case "${1:-}" in
    "--help"|"-h")
        echo "CameBoy Initialization Script"
        echo ""
        echo "Usage: $0 [options]"
        echo ""
        echo "Options:"
        echo "  --help, -h    Show this help"
        echo "  --no-clean    Skip cleanup of old files"
        echo ""
        echo "This script will:"
        echo "  - Detect your OS and install dependencies"
        echo "  - Set up the build environment"
        echo "  - Clean old build files"
        echo "  - Test the build system"
        echo ""
        exit 0
        ;;

    "--no-clean")
        # Désactiver le nettoyage
        cleanup_old() { true; }
        main
        ;;

    *)
        main
        ;;
esac

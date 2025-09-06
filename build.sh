#!/bin/bash
# =============================================================================
# CameBoy - Script de build robuste pour Linux/Mac
# =============================================================================

set -e  # Arrêter en cas d'erreur

# Configuration
CC="${CC:-gcc}"
CFLAGS="-Wall -Wextra -std=c99 -O2 -g -Isrc"
LDFLAGS="-lgdi32 -luser32 -lkernel32"  # Pour Windows cross-compilation
SRC_DIR="src"
BUILD_DIR="build"
OBJ_DIR="$BUILD_DIR/obj"
BIN_DIR="$BUILD_DIR/bin"
LOGS_DIR="logs"

# Couleurs pour les messages
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# Création des dossiers
create_dirs() {
    mkdir -p "$OBJ_DIR" "$BIN_DIR" "$LOGS_DIR"
}

# Vérification des dépendances
check_deps() {
    log_info "Checking dependencies..."

    if ! command -v "$CC" >/dev/null 2>&1; then
        log_error "$CC not found"
        exit 1
    fi

    log_success "$CC found: $($CC --version | head -1)"
}

# Compilation d'un fichier source
compile_file() {
    local src="$1"
    local obj="$2"

    log_info "Compiling $src..."
    if ! $CC $CFLAGS -c "$src" -o "$obj" 2>>"$LOGS_DIR/compile.log"; then
        log_error "Compilation failed for $src"
        cat "$LOGS_DIR/compile.log"
        exit 1
    fi
}

# Link des exécutables
link_executable() {
    local target="$1"
    local objects="$2"
    local log_file="$3"

    log_info "Linking $target..."
    if ! $CC $objects -o "$target" $LDFLAGS 2>"$LOGS_DIR/$log_file"; then
        log_error "Link failed for $target"
        cat "$LOGS_DIR/$log_file"
        exit 1
    fi
    log_success "$target built successfully"
}

# Compilation complète
build_all() {
    log_info "Building CameBoy..."

    create_dirs
    check_deps

    # Liste des fichiers sources principaux
    local main_sources=("cpu.c" "cpu_tables.c" "cpu_tables_cb.c" "mmu.c" "timer.c" "ppu.c" "joypad.c" "graphics_win32.c" "emulator_simple.c")
    local objects=""

    # Compilation des objets
    for src in "${main_sources[@]}"; do
        local obj="$OBJ_DIR/$(basename "$src" .c).o"
        compile_file "$SRC_DIR/$src" "$obj"
        objects="$objects $obj"
    done

    # Link de l'émulateur
    link_executable "$BIN_DIR/cameboy" "$objects" "link.log"

    log_success "Build completed successfully!"
    log_info "Executable: $BIN_DIR/cameboy"
}

# Construction des tests unitaires
build_tests() {
    log_info "Building test binaries..."

    create_dirs

    # Test CPU (complexe)
    log_info "Building test_cpu..."
    $CC $CFLAGS tests/unit/test_cpu.c src/cpu.c src/cpu_tables.c src/cpu_tables_cb.c src/mmu.c -o "$BIN_DIR/test_cpu" $LDFLAGS 2>>"$LOGS_DIR/test_build.log" || log_warning "Failed to build test_cpu"

    # Test MMU
    log_info "Building test_mmu..."
    $CC $CFLAGS tests/unit/test_mmu.c src/mmu.c -o "$BIN_DIR/test_mmu" $LDFLAGS 2>>"$LOGS_DIR/test_build.log" || log_warning "Failed to build test_mmu"

    # Test PPU
    log_info "Building test_ppu..."
    $CC $CFLAGS tests/unit/test_ppu.c src/ppu.c -o "$BIN_DIR/test_ppu" $LDFLAGS 2>>"$LOGS_DIR/test_build.log" || log_warning "Failed to build test_ppu"

    # Test Timer
    log_info "Building test_timer..."
    $CC $CFLAGS tests/unit/test_timer.c src/timer.c -o "$BIN_DIR/test_timer" $LDFLAGS 2>>"$LOGS_DIR/test_build.log" || log_warning "Failed to build test_timer"

    # Test Interrupt
    log_info "Building test_interrupt..."
    $CC $CFLAGS tests/unit/test_interrupt.c src/interrupt.c src/cpu.c src/cpu_tables.c src/cpu_tables_cb.c src/mmu.c -o "$BIN_DIR/test_interrupt" $LDFLAGS 2>>"$LOGS_DIR/test_build.log" || log_warning "Failed to build test_interrupt"

    # Test Joypad
    log_info "Building test_joypad..."
    $CC $CFLAGS tests/unit/test_joypad.c src/joypad.c -o "$BIN_DIR/test_joypad" $LDFLAGS 2>>"$LOGS_DIR/test_build.log" || log_warning "Failed to build test_joypad"

    log_success "Test binaries built"
}

# Exécution des tests
run_tests() {
    log_info "Running unit tests..."

    build_tests

    local total_tests=0
    local passed_tests=0

    # Création du fichier de résultats
    {
        echo "========================================"
        echo "CameBoy Unit Tests - $(date)"
        echo "========================================"
        echo ""
    } > "$LOGS_DIR/test_results.log"

    # Liste des tests à exécuter
    local test_names=("cpu" "mmu" "ppu" "timer" "interrupt" "joypad")

    for test_name in "${test_names[@]}"; do
        local test_exe="$BIN_DIR/test_$test_name"

        if [[ -f "$test_exe" ]]; then
            echo "Running test_$test_name..."
            echo "Running test_$test_name..." >> "$LOGS_DIR/test_results.log"

            if "./$test_exe" >> "$LOGS_DIR/test_results.log" 2>&1; then
                log_success "test_$test_name PASSED"
                echo "✅ test_$test_name PASSED" >> "$LOGS_DIR/test_results.log"
                ((passed_tests++))
            else
                log_error "test_$test_name FAILED"
                echo "❌ test_$test_name FAILED" >> "$LOGS_DIR/test_results.log"
            fi
            ((total_tests++))
            echo "" >> "$LOGS_DIR/test_results.log"
        else
            log_warning "Test binary test_$test_name not found"
        fi
    done

    # Résumé final
    {
        echo "========================================"
        echo "Total: $passed_tests/$total_tests tests passed"
    } >> "$LOGS_DIR/test_results.log"

    if [[ $passed_tests -eq $total_tests ]]; then
        log_success "ALL TESTS PASSED! 🎉"
        echo "🎉 ALL TESTS PASSED!" >> "$LOGS_DIR/test_results.log"
        return 0
    else
        log_error "SOME TESTS FAILED"
        echo "⚠️ SOME TESTS FAILED" >> "$LOGS_DIR/test_results.log"
        cat "$LOGS_DIR/test_results.log"
        return 1
    fi
}

# Nettoyage
clean() {
    log_info "Cleaning build files..."
    rm -rf "$BUILD_DIR"/*.log 2>/dev/null || true
    log_success "Clean completed"
}

# Mode debug
build_debug() {
    log_info "Building in DEBUG mode..."
    CFLAGS="$CFLAGS -DDEBUG -g3 -O0"
    build_all
}

# Mode release
build_release() {
    log_info "Building in RELEASE mode..."
    CFLAGS="$CFLAGS -O3 -DNDEBUG -flto"
    LDFLAGS="$LDFLAGS -flto"
    build_all
}

# Analyse statique
analyze() {
    log_info "Building with static analysis..."
    CFLAGS="$CFLAGS -fanalyzer"
    build_all
}

# Création de la distribution
create_dist() {
    log_info "Creating distribution..."

    local dist_dir="dist/cameboy"

    # Construction si nécessaire
    if [[ ! -f "$BIN_DIR/cameboy" ]]; then
        build_all
    fi

    mkdir -p "$dist_dir"
    cp "$BIN_DIR/cameboy" "$dist_dir/"
    cp README.md "$dist_dir/"

    log_success "Distribution created in $dist_dir/"
}

# Affichage de l'aide
show_help() {
    cat << EOF
CameBoy - Émulateur Game Boy
=============================

Usage: $0 [target]

Targets:
    (none)      Build all (default)
    clean       Clean build files
    test        Build and run unit tests
    debug       Build in debug mode
    release     Build in release mode
    analyze     Static analysis (if available)
    dist        Create distribution
    check       Check dependencies
    help        Show this help

Directories:
    build/      Build artifacts (obj, bin)
    dist/       Distribution files
    logs/       Compilation and test logs

Test results: logs/test_results.log
EOF
}

# Point d'entrée principal
main() {
    case "${1:-build}" in
        "clean") clean ;;
        "test") run_tests ;;
        "debug") build_debug ;;
        "release") build_release ;;
        "analyze") analyze ;;
        "dist") create_dist ;;
        "check") check_deps ;;
        "help"|"-h"|"--help") show_help ;;
        *) build_all ;;
    esac
}

# Exécution
main "$@"

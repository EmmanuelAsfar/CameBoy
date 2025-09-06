#!/bin/bash
# =============================================================================
# CameBoy - Nettoyage des anciens fichiers
# Script pour supprimer les fichiers de build obsolètes
# =============================================================================

set -e

echo "🧹 Nettoyage des anciens fichiers de build..."

# Liste des fichiers/dossiers à supprimer
OLD_FILES=(
    # Anciens scripts .bat
    "analyze_blargg_results.bat"
    "analyze_blargg_simple.bat"
    "analyze_mooneye_results.bat"
    "build_animation.bat"
    "build_blargg.bat"
    "build_complete.bat"
    "build_debug.bat"
    "build_memory_test_log.txt"
    "build_rom_dump_log.txt"
    "build_serial_log.txt"
    "build_serial_test.bat"
    "build_test_cpu_one_log.txt"
    "build_test_direct_log.txt"
    "build_test_log.txt"
    "build_win32.bat"
    "capture_blargg_logs.bat"
    "create_test_roms.bat"
    "download_tests.bat"
    "test_all_blargg.bat"
    "test_mooneye.bat"

    # Anciens logs éparpillés
    "animation_compilation_log.txt"
    "animation_rom_log.txt"
    "animation_test_log.txt"
    "blargg_console_log.txt"
    "blargg_test_03.txt"
    "blargg_test_long.txt"
    "blargg_test_output.txt"
    "build_log.txt"
    "compilation_log.txt"
    "compile_output.txt"
    "debug_blargg_lcd.txt"
    "debug_blargg_vblank.txt"
    "debug_blargg.txt"
    "debug_detailed.txt"
    "debug_interrupts.txt"
    "debug_long.txt"
    "debug_main.txt"
    "debug_ppu_fixed.txt"
    "debug_window.txt"
    "serial_compilation_log.txt"
    "test_blargg_final.txt"
    "test_blargg_lcd.txt"
    "test_blargg_registers.txt"
    "test_blargg_serial.txt"
    "test_blargg_zone.txt"
    "test_cpu_instrs_final.txt"
    "test_cpu_instrs_long.txt"
    "test_halt_bug.txt"
    "test_hello.txt"
    "test_ime_enabled.txt"
    "test_instr_timing.txt"
    "test_interrupts_individual.txt"
    "test_interrupts.txt"
    "test_main_blargg.txt"
    "test_mem_timing.txt"
    "test_output.txt"
    "test_pattern.asm"
    "test_serial_output.txt"

    # Anciens exécutables
    "create_animation_rom.exe"
    "create_color_pattern.exe"
    "create_console_bin.exe"
    "create_simple_animation.exe"
    "create_simple_rom.exe"
    "create_test_rom.exe"
    "create_test_rom.exe"
    "test_serial.exe"

    # Fichiers .o dans src/
    "src/*.o"
    "src/cpu_tables.o"
    "src/cpu_tables_cb.o"
)

# Compteur de fichiers supprimés
deleted_count=0

for file in "${OLD_FILES[@]}"; do
    if [[ -f "$file" ]] || [[ -d "$file" ]]; then
        echo "Suppression de $file"
        rm -rf "$file"
        ((deleted_count++))
    fi
done

# Anciens dossiers de résultats
if [[ -d "blargg_results" ]]; then
    echo "Suppression du dossier blargg_results/"
    rm -rf "blargg_results"
    ((deleted_count++))
fi

if [[ -d "mooneye_results" ]]; then
    echo "Suppression du dossier mooneye_results/"
    rm -rf "mooneye_results"
    ((deleted_count++))
fi

echo ""
echo "✅ Nettoyage terminé !"
echo "📊 $deleted_count fichiers/dossiers supprimés"
echo ""
echo "Les nouveaux scripts à utiliser :"
echo "  - make              # Compilation complète"
echo "  - make test         # Tests unitaires"
echo "  - make clean        # Nettoyage"
echo "  - ./build.sh        # Script Linux/Mac"
echo "  - build.bat         # Script Windows"
echo ""
echo "Résultats des tests : logs/test_results.log"
echo "Logs de compilation : logs/compile.log"

# =============================================================================
# CameBoy - Émulateur Game Boy
# Makefile pour Windows
# =============================================================================

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -Isrc
LDFLAGS = -lgdi32 -luser32 -lkernel32

# Dossiers
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)\obj
BIN_DIR = $(BUILD_DIR)\bin
LOGS_DIR = logs
TEST_DIR = tests\unit

# Fichiers sources principaux
SOURCES = $(SRC_DIR)\cpu.c $(SRC_DIR)\cpu_tables.c $(SRC_DIR)\cpu_tables_cb.c $(SRC_DIR)\mmu.c $(SRC_DIR)\timer.c $(SRC_DIR)\ppu.c $(SRC_DIR)\joypad.c $(SRC_DIR)\graphics_win32.c $(SRC_DIR)\emulator_simple.c
OBJECTS = $(SOURCES:$(SRC_DIR)\%.c=$(OBJ_DIR)\%.o)

# Cibles
MAIN_TARGET = $(BIN_DIR)\cameboy.exe
TEST_CPU = $(BIN_DIR)\test_cpu.exe
TEST_MMU = $(BIN_DIR)\test_mmu.exe
TEST_PPU = $(BIN_DIR)\test_ppu.exe
TEST_TIMER = $(BIN_DIR)\test_timer.exe
TEST_INTERRUPT = $(BIN_DIR)\test_interrupt.exe
TEST_JOYPAD = $(BIN_DIR)\test_joypad.exe

# =============================================================================
# RÈGLES PRINCIPALES
# =============================================================================

.PHONY: all clean test

all: $(MAIN_TARGET)

$(MAIN_TARGET): $(OBJECTS)
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	@if not exist "$(LOGS_DIR)" mkdir "$(LOGS_DIR)"
	@echo Compilation de $(MAIN_TARGET)...
	@$(CC) $(OBJECTS) -o $@ $(LDFLAGS) 2> $(LOGS_DIR)\link.log
	@if %errorlevel% neq 0 (echo ERREUR Link - voir $(LOGS_DIR)\link.log & exit /b 1)
	@echo SUCCES $(MAIN_TARGET) compile

$(OBJ_DIR)\%.o: $(SRC_DIR)\%.c
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	@if not exist "$(LOGS_DIR)" mkdir "$(LOGS_DIR)"
	@echo Compilation de $<...
	@$(CC) $(CFLAGS) -c $< -o $@ 2>> $(LOGS_DIR)\compile.log
	@if %errorlevel% neq 0 (echo ERREUR Compilation $< & exit /b 1)

# =============================================================================
# TESTS UNITAIRES
# =============================================================================

test: $(TEST_CPU) $(TEST_MMU) $(TEST_PPU) $(TEST_TIMER) $(TEST_INTERRUPT) $(TEST_JOYPAD)
	@echo ======================================== > $(LOGS_DIR)\test_results.log
	@echo CameBoy Unit Tests - %DATE% %TIME% >> $(LOGS_DIR)\test_results.log
	@echo ======================================== >> $(LOGS_DIR)\test_results.log
	@echo. >> $(LOGS_DIR)\test_results.log
	@set total=0
	@set passed=0
	@for %%t in ($(TEST_CPU) $(TEST_MMU) $(TEST_PPU) $(TEST_TIMER) $(TEST_INTERRUPT) $(TEST_JOYPAD)) do ( ^
		@echo Running %%~nt... ^
		@echo Running %%~nt... >> $(LOGS_DIR)\test_results.log ^
		@if %%t >> $(LOGS_DIR)\test_results.log 2>&1 ( ^
			@echo SUCCES %%~nt PASSED ^
			@echo SUCCES %%~nt PASSED >> $(LOGS_DIR)\test_results.log ^
			set /a passed+=1 ^
		) else ( ^
			@echo ERREUR %%~nt FAILED ^
			@echo ERREUR %%~nt FAILED >> $(LOGS_DIR)\test_results.log ^
		) ^
		set /a total+=1 ^
		@echo. >> $(LOGS_DIR)\test_results.log ^
	)
	@echo ======================================== >> $(LOGS_DIR)\test_results.log
	@echo Total: %passed%/%total% tests passed >> $(LOGS_DIR)\test_results.log
	@if %passed%==%total% ( ^
		echo TOUS LES TESTS REUSSIS! ^
		echo TOUS LES TESTS REUSSIS! >> $(LOGS_DIR)\test_results.log ^
	) else ( ^
		echo CERTAINS TESTS ONT ECHOUE ^
		echo CERTAINS TESTS ONT ECHOUE >> $(LOGS_DIR)\test_results.log ^
	)

$(TEST_CPU): $(TEST_DIR)\test_cpu.c $(OBJ_DIR)\cpu.o $(OBJ_DIR)\cpu_tables.o $(OBJ_DIR)\cpu_tables_cb.o $(OBJ_DIR)\mmu.o $(OBJ_DIR)\timer.o $(OBJ_DIR)\apu.o
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	@echo Compilation test_cpu...
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 2>> $(LOGS_DIR)\test_build.log

$(TEST_MMU): $(TEST_DIR)\test_mmu.c $(OBJ_DIR)\mmu.o $(OBJ_DIR)\timer.o $(OBJ_DIR)\apu.o
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	@echo Compilation test_mmu...
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 2>> $(LOGS_DIR)\test_build.log

$(TEST_PPU): $(TEST_DIR)\test_ppu.c $(OBJ_DIR)\ppu.o
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	@echo Compilation test_ppu...
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 2>> $(LOGS_DIR)\test_build.log

$(TEST_TIMER): $(TEST_DIR)\test_timer.c $(OBJ_DIR)\timer.o
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	@echo Compilation test_timer...
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 2>> $(LOGS_DIR)\test_build.log

$(TEST_INTERRUPT): $(TEST_DIR)\test_interrupt.c $(OBJ_DIR)\interrupt.o $(OBJ_DIR)\cpu.o $(OBJ_DIR)\cpu_tables.o $(OBJ_DIR)\cpu_tables_cb.o $(OBJ_DIR)\mmu.o $(OBJ_DIR)\timer.o $(OBJ_DIR)\apu.o
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	@echo Compilation test_interrupt...
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 2>> $(LOGS_DIR)\test_build.log

$(TEST_JOYPAD): $(TEST_DIR)\test_joypad.c $(OBJ_DIR)\joypad.o
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	@echo Compilation test_joypad...
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 2>> $(LOGS_DIR)\test_build.log

# =============================================================================
# NETTOYAGE
# =============================================================================

clean:
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)" 2>nul
	@echo Nettoyage effectue
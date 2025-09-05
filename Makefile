# Makefile pour CameBoy - Émulateur Game Boy

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
LDFLAGS = -lSDL2

# Dossiers
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Fichiers sources
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/cameboy

# Règle par défaut
all: $(TARGET)

# Création de l'exécutable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compilation des fichiers objet
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Création des dossiers
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Nettoyage
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Installation des dépendances (Windows avec vcpkg)
install-deps:
	@echo "Installation des dépendances..."
	@echo "Assurez-vous d'avoir SDL2 installé"
	@echo "Sur Windows avec vcpkg: vcpkg install sdl2"

# Tests
test: $(TARGET)
	@echo "Lancement des tests..."
	@echo "Placez les ROMs de test dans tests/"

# Debug
debug: CFLAGS += -DDEBUG -g3
debug: $(TARGET)

# Release
release: CFLAGS += -O3 -DNDEBUG
release: clean $(TARGET)

# Aide
help:
	@echo "Cibles disponibles:"
	@echo "  all      - Compiler l'émulateur (défaut)"
	@echo "  clean    - Nettoyer les fichiers générés"
	@echo "  debug    - Compiler en mode debug"
	@echo "  release  - Compiler en mode release"
	@echo "  test     - Lancer les tests"
	@echo "  help     - Afficher cette aide"

.PHONY: all clean install-deps test debug release help

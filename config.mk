# =============================================================================
# CameBoy - Configuration centralisée
# =============================================================================

# Compilateur
CC ?= gcc

# Flags de compilation
CFLAGS_BASE = -Wall -Wextra -std=c99 -Isrc
CFLAGS_DEBUG = $(CFLAGS_BASE) -DDEBUG -g3 -O0
CFLAGS_RELEASE = $(CFLAGS_BASE) -O3 -DNDEBUG -flto
CFLAGS ?= $(CFLAGS_BASE) -O2 -g

# Flags de link
LDFLAGS_BASE = -lgdi32 -luser32 -lkernel32
LDFLAGS_RELEASE = $(LDFLAGS_BASE) -flto
LDFLAGS ?= $(LDFLAGS_BASE)

# Flags pour les tests
TEST_LDFLAGS = $(LDFLAGS)

# Structure des dossiers
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
DIST_DIR = dist
TEST_DIR = tests/unit
LOGS_DIR = logs

# Fichiers sources
SOURCES = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(SRC_DIR)/*.h)

# Programmes cibles
MAIN_TARGET = $(BIN_DIR)/cameboy$(EXE)
SIMPLE_TARGET = $(BIN_DIR)/cameboy_simple$(EXE)

# Tests unitaires
TEST_SOURCES = $(wildcard $(TEST_DIR)/test_*.c)
TEST_TARGETS = $(patsubst $(TEST_DIR)/test_%.c, $(BIN_DIR)/test_%$(EXE), $(TEST_SOURCES))

# Détection de l'OS
ifeq ($(OS), Windows_NT)
    EXE = .exe
    RM = del /q
    RMDIR = rmdir /s /q
    MKDIR = mkdir
    PATH_SEP = \\
else
    EXE =
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p
    PATH_SEP = /
endif

# Couleurs pour les messages (si supporté)
ifneq ($(TERM), dumb)
    GREEN = \033[0;32m
    RED = \033[0;31m
    YELLOW = \033[1;33m
    BLUE = \033[0;34m
    NC = \033[0m
endif

# Messages
MSG_COMPILE = $(BLUE)📝 Compiling$(NC)
MSG_LINK = $(BLUE)🔨 Linking$(NC)
MSG_CLEAN = $(BLUE)🧹 Cleaning$(NC)
MSG_TEST = $(BLUE)🧪 Testing$(NC)
MSG_SUCCESS = $(GREEN)✅ Success$(NC)
MSG_ERROR = $(RED)❌ Error$(NC)

# Export des variables
export CC CFLAGS LDFLAGS

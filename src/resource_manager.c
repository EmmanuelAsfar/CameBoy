#include "resource_manager.h"
#include <shlwapi.h>
#ifdef _MSC_VER
#pragma comment(lib, "shlwapi.lib")
#endif

// Chemin global vers le dossier resources
static char g_resources_path[MAX_PATH] = {0};

bool resource_manager_init(void) {
    // 1. Obtenir le chemin de l'exécutable
    if (GetModuleFileNameA(NULL, g_resources_path, MAX_PATH) == 0) {
        printf("ERREUR: Impossible d'obtenir le chemin de l'exécutable\n");
        return false;
    }
    printf("DEBUG: Chemin exécutable: %s\n", g_resources_path);
    
    // 2. Remonter au dossier parent (build/bin -> build)
    PathRemoveFileSpecA(g_resources_path);  // build/bin -> build/bin (remove exe)
    printf("DEBUG: apres remove file: %s\n", g_resources_path);
    // remonter encore un niveau: build/bin -> build
    PathRemoveFileSpecA(g_resources_path);
    printf("DEBUG: dossier build: %s\n", g_resources_path);
    
    // 3. Ajouter "resources" (build/resources)
    PathAppendA(g_resources_path, "resources");
    printf("DEBUG: dossier resources: %s\n", g_resources_path);
    
    // 4. Vérifier que le dossier existe
    if (!PathFileExistsA(g_resources_path)) {
        printf("ERREUR: Dossier resources introuvable: %s\n", g_resources_path);
        return false;
    }
    
    printf("Gestionnaire de ressources initialisé: %s\n", g_resources_path);
    return true;
}

HBITMAP resource_load_image(const char* filename) {
    if (!g_resources_path[0]) {
        printf("ERREUR: Gestionnaire de ressources non initialisé\n");
        return NULL;
    }
    
    char full_path[MAX_PATH];
    PathCombineA(full_path, g_resources_path, filename);
    
    printf("Tentative de chargement: %s\n", full_path);
    
    HBITMAP hBitmap = (HBITMAP)LoadImageA(NULL, full_path, IMAGE_BITMAP, 0, 0, 
                                         LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    
    if (!hBitmap) {
        DWORD error = GetLastError();
        printf("ERREUR: Impossible de charger l'image %s (code: %lu)\n", full_path, error);
    } else {
        printf("Image chargée avec succès: %s\n", full_path);
    }
    
    return hBitmap;
}

void resource_manager_cleanup(void) {
    // Pour l'instant, rien à nettoyer
    // Mais on pourrait garder une liste des bitmaps chargés pour les libérer
}

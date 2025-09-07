#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "common.h"
#include <windows.h>

// Initialiser le gestionnaire de ressources
bool resource_manager_init(void);

// Charger une image par nom de fichier
HBITMAP resource_load_image(const char* filename);

// Nettoyer le gestionnaire
void resource_manager_cleanup(void);

#endif // RESOURCE_MANAGER_H

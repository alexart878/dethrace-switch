#define _GNU_SOURCE
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <switch.h>
#include <string.h>

#include "../include/harness/config.h"
#include "common/globvars.h"
#include "common/car.h"
#include "common/sound.h"

static DIR* directory_iterator;
const char* config_path = "config.txt";


void resolve_full_path(char* path, const char* argv0)
{
    return;
}

FILE* OS_fopen(const char* pathname, const char* mode) {
    FILE* f = fopen(pathname, mode);
    if (f != NULL) {
        return f;
    }
    return NULL;
}

size_t OS_ConsoleReadPassword(char* pBuffer, size_t pBufferLen) {
    return 0;
}

char* OS_Basename(const char* path) {
	return "sdmc:/switch/dethrace";
}

char* OS_GetWorkingDirectory(char* argv0) {
	return "sdmc:/switch/dethrace";
}

void OS_InstallSignalHandler(char* program_name) {
    return;
}

char* OS_GetNextFileInDirectory(void) {
    struct dirent* entry;

    if (directory_iterator == NULL) {
        return NULL;
    }
    while ((entry = readdir(directory_iterator)) != NULL) {
        if (entry->d_type == DT_REG) {
            return entry->d_name;
        }
    }
    closedir(directory_iterator);
    directory_iterator = NULL;
    return NULL;
}

char* OS_GetFirstFileInDirectory(char* path) {
    directory_iterator = opendir(path);
    if (directory_iterator == NULL) {
        return NULL;
    }
    return OS_GetNextFileInDirectory();
}

extern tHarness_game_config harness_game_config;

void Switch_ConfigParse(int* gExtra_mem, int* gReplay_override, int* gNo_voodoo, int* gForce_voodoo_mode, int* gForce_voodoo_rush_mode)
{
    FILE* file = fopen("config.txt", "r");
    
    if (!file)
    {
        printf("No config file found.\n");
        return;
    }

    char line[256];

    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\r\n")] = 0;

        if (line[0] == '#' || line[0] == '\0') continue;

        char key[64];
        char value[128];

        if (sscanf(line, "%63[^=]=%127s", key, value) == 2) {
            // Strip spaces
            char* k = key;
            while (*k == ' ') k++;
            char* v = value;
            while (*v == ' ') v++;

            if (strcasecmp(k, "hires") == 0) {
                gGraf_spec_index = (strcasecmp(v, "true") == 0);
            } else if (strcasecmp(k, "yon") == 0) {
                float f = atof(v);
                if (f >= 0.0f && f <= 1.0f) gYon_multiplier = f;
            } else if (strcasecmp(k, "simple") == 0) {
                int s = atoi(v);
                if (s >= 0 && s < 5) gCar_simplification_level = s;
            } else if (strcasecmp(k, "sound") == 0) {
                gSound_detail_level = atoi(v);
            } else if (strcasecmp(k, "robots") == 0 || strcasecmp(k, "german") == 0) {
                gSausage_override = (strcasecmp(v, "true") == 0);
            } else if (strcasecmp(k, "lomem") == 0) {
                gAustere_override = (strcasecmp(v, "true") == 0);
            } else if (strcasecmp(k, "nosound") == 0) {
                gSound_override = (strcasecmp(v, "true") == 0);
            } else if (strcasecmp(k, "spamfritter") == 0 && strcasecmp(v, "true") == 0) {
                *gExtra_mem = 2000000;
            } else if (strcasecmp(k, "nocutscenes") == 0) {
                gCut_scene_override = (strcasecmp(v, "true") == 0);
            } else if (strcasecmp(k, "noreplay") == 0) {
                *gReplay_override = (strcasecmp(v, "true") == 0);
            } else if (strcasecmp(k, "novoodoo") == 0) {
                *gNo_voodoo = (strcasecmp(v, "true") == 0);
            } else if (strcasecmp(k, "vrush") == 0) {
                *gForce_voodoo_mode = 0;
                *gForce_voodoo_rush_mode = (strcasecmp(v, "true") == 0);
            } else if (strcasecmp(k, "vgraphics") == 0) {
                *gForce_voodoo_rush_mode = 0;
                *gForce_voodoo_mode = (strcasecmp(v, "true") == 0);
            } else {
                printf("Unknown config option: %s\n", k);
            }

            if (strcasecmp(k, "opengl") == 0) {
                harness_game_config.opengl_3dfx_mode = (strcasecmp(v, "true") == 0);
            }
        }
    }

    fclose(file);
}

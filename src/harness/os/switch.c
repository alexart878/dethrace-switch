#define _GNU_SOURCE
#include <limits.h>
#include <stdio.h>

#include <switch.h>
#include <string.h>

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

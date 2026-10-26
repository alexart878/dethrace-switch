#include "harness/trace.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

int harness_debug_level = 4;

static int log_file_cleared = 0;

void debug_printf(const char* fmt, const char* fn, const char* fmt2, ...) {

#ifndef __SWITCH__

    va_list ap;

    printf(fmt, fn);

    va_start(ap, fmt2);
    vprintf(fmt2, ap);
    va_end(ap);

    puts("\033[0m");

#else

    va_list ap;
    FILE* fp = NULL;

    if (!log_file_cleared)
    {
        fp = fopen("dethrace.log", "w");
        log_file_cleared = 1;
    }
    else
    {
        fp = fopen("dethrace.log", "a");
    }

    if (fp != NULL) {
        fprintf(fp, fmt, fn);
        
        va_start(ap, fmt2);
        vfprintf(fp, fmt2, ap);
        fprintf(fp, "\n");
        va_end(ap);

        fclose(fp);
    }

#endif

}

void panic_printf(const char* fmt, const char* fn, const char* fmt2, ...) {
    va_list ap;

    FILE* fp;

    if (!log_file_cleared)
    {
        fp = fopen("dethrace.log", "w");
        log_file_cleared = 1;
    }
    else
    {
        fp = fopen("dethrace.log", "a");
    }

    puts("\033[0;31m");
    printf(fmt, fn);

    if (fp != NULL) {
        fprintf(fp, fmt, fn);
    }

    va_start(ap, fmt2);
    vprintf(fmt2, ap);
    if (fp != NULL) {
        vfprintf(fp, fmt2, ap);
    }
    va_end(ap);
    if (fp != NULL) {
        fclose(fp);
    }
    puts("\033[0m");
}

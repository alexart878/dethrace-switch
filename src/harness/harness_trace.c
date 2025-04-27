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

void debug_print_vector3(const char* fmt, const char* fn, char* msg, br_vector3* v) {
    printf(fmt, fn);
    printf("%s %f, %f, %f\n", msg, v->v[0], v->v[1], v->v[2]);
    puts("\033[0m");
}

void debug_print_matrix34(const char* fmt, const char* fn, char* msg, br_matrix34* m) {
    printf(fmt, fn);
    printf("matrix34 \"%s\"\n", msg);
    for (int i = 0; i < 4; i++) {
        printf("  %f, %f, %f\n", m->m[i][0], m->m[i][1], m->m[i][2]);
    }
    puts("\033[0m");
}

void debug_print_matrix4(const char* fmt, const char* fn, char* msg, br_matrix4* m) {
    printf(fmt, fn);
    printf("matrix34 \"%s\"\n", msg);
    for (int i = 0; i < 4; i++) {
        printf("  %f, %f, %f, %f\n", m->m[i][0], m->m[i][1], m->m[i][2], m->m[i][3]);
    }
    puts("\033[0m");
}

// int count_open_fds(void) {
//     DIR* dp = opendir("/dev/fd/");
//     struct dirent* de;
//     int count = -3; // '.', '..', dp

//     if (dp == NULL)
//         return -1;

//     while ((de = readdir(dp)) != NULL)
//         count++;

//     (void)closedir(dp);

//     return count;
// }

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// "print" function: creates a formatted string from a format and variable arguments
char* vprint_fmt(const char* fmt, va_list ap) {
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (needed < 0) return NULL;
    char* buf = (char*)malloc((size_t)needed + 1);
    if (!buf) return NULL;
    vsnprintf(buf, (size_t)needed + 1, fmt, ap);
    return buf;
}

char* print_fmt(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char* s = vprint_fmt(fmt, ap);
    va_end(ap);
    return s;
}

// "system" function: executes a command and returns the exit code
int system_exec(const char* cmd) {
    return system(cmd);
}

// Convenience function combining print + system
int execf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char* cmd = vprint_fmt(fmt, ap);
    va_end(ap);
    if (!cmd) return -1;
    int rc = system_exec(cmd);
    free(cmd);
    return rc;
}

// 5 test cases
int main(void) {
    int r1 = execf("ls");
    int r2 = execf("ls %s", "-l");
    int r3 = execf("ls %s %s", "-a", ".");
    int r4 = execf("ls %s", "--color=auto");
    int r5 = execf("ls %s", "..");

    printf("Exit codes:\n");
    printf("%d\n", r1);
    printf("%d\n", r2);
    printf("%d\n", r3);
    printf("%d\n", r4);
    printf("%d\n", r5);
    return 0;
}
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// "print" function: returns a newly allocated formatted string (caller must free).
char* print(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (needed < 0) {
        va_end(ap_copy);
        return NULL;
    }
    char* buf = (char*)malloc((size_t)needed + 1);
    if (!buf) {
        va_end(ap_copy);
        return NULL;
    }
    vsnprintf(buf, (size_t)needed + 1, fmt, ap_copy);
    va_end(ap_copy);
    return buf;
}

// "system" function: executes the provided command string.
int runSystem(const char* cmd) {
    return system(cmd);
}

// Convenience function: format and execute (frees intermediate buffer).
int runSystemf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    // Prepare formatted string
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (needed < 0) {
        va_end(ap_copy);
        return -1;
    }
    char* buf = (char*)malloc((size_t)needed + 1);
    if (!buf) {
        va_end(ap_copy);
        return -1;
    }
    vsnprintf(buf, (size_t)needed + 1, fmt, ap_copy);
    va_end(ap_copy);

    int rc = runSystem(buf);
    free(buf);
    return rc;
}

int main(void) {
    // 5 test cases using ls with different arguments
    runSystemf("ls");
    runSystemf("ls %s", "-l");
    runSystemf("ls %s %s", "-l", "-a");
    runSystemf("ls %s %s %s", "-l", "-a", ".");
    runSystemf("ls %s", ".");
    return 0;
}
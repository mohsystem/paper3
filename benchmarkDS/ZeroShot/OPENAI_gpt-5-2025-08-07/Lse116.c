#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

// Validate path to avoid command injection
int is_safe_path(const char* s) {
    if (s == NULL || *s == '\0') return 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        unsigned char c = *p;
        if (!(isalnum(c) || c == '/' || c == '.' || c == '_' || c == '-' || c == '+')) {
            return 0;
        }
    }
    return 1;
}

// Print-like formatting function that returns a heap-allocated string (caller must free)
char* print_format(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (needed < 0) {
        va_end(ap);
        return NULL;
    }
    char* buf = (char*)malloc((size_t)needed + 1);
    if (!buf) {
        va_end(ap);
        return NULL;
    }
    vsnprintf(buf, (size_t)needed + 1, fmt, ap);
    va_end(ap);
    return buf;
}

// Execute system command
int system_exec(const char* cmd) {
    if (!cmd) return -1;
    return system(cmd);
}

// Function to execute ls with a sanitized path argument
int ls_command(const char* dir) {
    if (!is_safe_path(dir)) {
        printf("Rejected unsafe path: %s\n", dir ? dir : "(null)");
        return -1;
    }
    char* cmd = print_format("/bin/ls -la -- %s", dir);
    if (!cmd) return -1;
    int rc = system_exec(cmd);
    free(cmd);
    return rc;
}

int main(void) {
    const char* tests[5] = {
        ".",
        "src",
        "/tmp",
        "does-not-exist",
        "bad;rm -rf /"
    };
    for (int i = 0; i < 5; ++i) {
        printf("Running ls on: %s\n", tests[i]);
        int status = ls_command(tests[i]);
        printf("Exit status: %d\n", status);
        printf("----\n");
    }
    return 0;
}
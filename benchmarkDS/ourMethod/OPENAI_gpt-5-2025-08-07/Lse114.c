#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static const char* ALLOWED_FLAGS[] = {
    "", "-l", "-a", "-la", "-al", "-1", "-lh", "-lah", "-alh"
};
static const size_t ALLOWED_FLAGS_COUNT = sizeof(ALLOWED_FLAGS) / sizeof(ALLOWED_FLAGS[0]);

// "print" function: creates a formatted string and returns a heap-allocated buffer
static char* vprint_format(const char* fmt, ...) {
    if (fmt == NULL) return NULL;
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
    size_t size = (size_t)needed + 1;
    char* buf = (char*)malloc(size);
    if (buf == NULL) {
        va_end(ap);
        return NULL;
    }
    int written = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    if (written < 0) {
        free(buf);
        return NULL;
    }
    return buf;
}

static int is_allowed_flag(const char* flags) {
    const char* f = (flags == NULL) ? "" : flags;
    for (size_t i = 0; i < ALLOWED_FLAGS_COUNT; i++) {
        if (strcmp(f, ALLOWED_FLAGS[i]) == 0) return 1;
    }
    return 0;
}

static int is_safe_path(const char* path) {
    if (path == NULL) return 0;
    size_t len = strlen(path);
    if (len == 0 || len > 1024) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = path[i];
        int ok = (c >= 'A' && c <= 'Z') ||
                 (c >= 'a' && c <= 'z') ||
                 (c >= '0' && c <= '9') ||
                 c == '.' || c == '_' || c == '/' || c == '-';
        if (!ok) return 0;
    }
    return 1;
}

// "system" function: executes a command string
static int system_exec(const char* cmd) {
    if (cmd == NULL || cmd[0] == '\0') return -1;
    return system(cmd);
}

// Secure wrapper to build and execute an ls command
int safe_ls(const char* flags_in, const char* path_in) {
    const char* flags = (flags_in == NULL) ? "" : flags_in;
    const char* path = (path_in == NULL || path_in[0] == '\0') ? "." : path_in;

    if (!is_allowed_flag(flags)) {
        fprintf(stderr, "Invalid flags\n");
        return -1;
    }
    if (!is_safe_path(path)) {
        fprintf(stderr, "Invalid path\n");
        return -1;
    }

    char* cmd = NULL;
    if (flags[0] != '\0') {
        cmd = vprint_format("ls %s %s", flags, path);
    } else {
        cmd = vprint_format("ls %s", path);
    }
    if (cmd == NULL) {
        fprintf(stderr, "Command build failed\n");
        return -1;
    }
    int rc = system_exec(cmd);
    free(cmd);
    return rc;
}

int main(void) {
    // 5 test cases
    int rc1 = safe_ls(NULL, ".");
    printf("Return code: %d\n", rc1);

    int rc2 = safe_ls("-l", ".");
    printf("Return code: %d\n", rc2);

    int rc3 = safe_ls("-a", "/");
    printf("Return code: %d\n", rc3);

    int rc4 = safe_ls("-lh", ".");
    printf("Return code: %d\n", rc4);

    int rc5 = safe_ls("-rf", "."); // invalid
    printf("Return code: %d\n", rc5);

    return 0;
}
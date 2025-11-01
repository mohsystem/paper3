#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define MAX_CMD 256

// Build a command string using a format string and variable arguments (print-like).
static int print_command(char *out, size_t outsz, const char *fmt, ...) {
    if (out == NULL || outsz == 0 || fmt == NULL) {
        return -1;
    }
    va_list ap;
    va_start(ap, fmt);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (needed <= 0 || (size_t)needed >= outsz || (size_t)needed > MAX_CMD) {
        out[0] = '\0';
        return -1;
    }
    va_start(ap, fmt);
    int written = vsnprintf(out, outsz, fmt, ap);
    va_end(ap);
    if (written <= 0 || (size_t)written >= outsz) {
        out[0] = '\0';
        return -1;
    }
    // Trim leading/trailing whitespace
    size_t len = strlen(out);
    while (len > 0 && (out[0] == ' ' || out[0] == '\t' || out[0] == '\n' || out[0] == '\r')) {
        memmove(out, out + 1, len);
        len = strlen(out);
    }
    while (len > 0 && (out[len - 1] == ' ' || out[len - 1] == '\t' || out[len - 1] == '\n' || out[len - 1] == '\r')) {
        out[len - 1] = '\0';
        len--;
    }
    return 0;
}

// Validate that the command is a safe "ls" invocation with at most one simple argument
static int is_safe_ls_command(const char *cmd) {
    if (cmd == NULL) return 0;
    size_t len = strlen(cmd);
    if (len < 2 || len > MAX_CMD) return 0;
    if (!(cmd[0] == 'l' && cmd[1] == 's')) return 0;
    if (len == 2) return 1; // exactly "ls"
    if (len < 4) return 0;  // need space and at least one char
    if (cmd[2] != ' ') return 0;

    const char *arg = cmd + 3;
    if (*arg == '\0') return 0;
    if (strlen(arg) > 200) return 0;
    if (strstr(arg, "..") != NULL) return 0; // block traversal
    // disallow spaces in arg for safe tokenization
    for (const char *p = arg; *p; ++p) {
        if (*p == ' ') return 0;
        if (!(isalnum((unsigned char)*p) || *p == '/' || *p == '.' || *p == '_' || *p == '-')) {
            return 0;
        }
    }
    return 1;
}

// "system"-like function that executes the provided command string.
static int safe_system(const char *cmd) {
    if (!is_safe_ls_command(cmd)) {
        printf("Rejected command\n");
        return -1;
    }
    int rc = system(cmd);
    return rc;
}

// Convenience function: format then execute ls.
int run_ls_formatted(const char *fmt, ...) {
    char cmd[MAX_CMD + 1];
    cmd[0] = '\0';

    va_list ap;
    va_start(ap, fmt);
    // Build formatted string into a temporary buffer
    char tmp[MAX_CMD + 1];
    tmp[0] = '\0';
    int needed = vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    if (needed <= 0 || needed > MAX_CMD) {
        printf("Rejected command\n");
        return -1;
    }
    // Trim whitespace
    size_t len = strlen(tmp);
    while (len > 0 && (tmp[0] == ' ' || tmp[0] == '\t' || tmp[0] == '\n' || tmp[0] == '\r')) {
        memmove(tmp, tmp + 1, len);
        len = strlen(tmp);
    }
    while (len > 0 && (tmp[len - 1] == ' ' || tmp[len - 1] == '\t' || tmp[len - 1] == '\n' || tmp[len - 1] == '\r')) {
        tmp[len - 1] = '\0';
        len--;
    }
    strncpy(cmd, tmp, MAX_CMD);
    cmd[MAX_CMD] = '\0';

    return safe_system(cmd);
}

int main(void) {
    // 5 test cases
    int r1 = run_ls_formatted("ls");
    printf("Exit: %d\n", r1);

    int r2 = run_ls_formatted("ls %s", ".");
    printf("Exit: %d\n", r2);

    int r3 = run_ls_formatted("ls %s", "/");
    printf("Exit: %d\n", r3);

    int r4 = run_ls_formatted("ls %s", "./");
    printf("Exit: %d\n", r4);

    int r5 = run_ls_formatted("ls %s", "nonexistent_path_should_fail");
    printf("Exit: %d\n", r5);

    return 0;
}
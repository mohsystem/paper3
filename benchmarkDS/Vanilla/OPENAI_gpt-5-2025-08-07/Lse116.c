#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

char* print(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) {
        va_end(ap2);
        return NULL;
    }
    char* buf = (char*)malloc((size_t)n + 1);
    if (!buf) {
        va_end(ap2);
        return NULL;
    }
    vsnprintf(buf, (size_t)n + 1, fmt, ap2);
    va_end(ap2);
    return buf;
}

int run_command(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) {
        va_end(ap2);
        return -1;
    }
    char* buf = (char*)malloc((size_t)n + 1);
    if (!buf) {
        va_end(ap2);
        return -1;
    }
    vsnprintf(buf, (size_t)n + 1, fmt, ap2);
    va_end(ap2);
    int rc = system(buf);
    free(buf);
    return rc;
}

int main(void) {
    run_command("ls");
    run_command("ls %s", "-l");
    run_command("ls %s", "-a");
    run_command("ls %s %s", "-l", ".");
    run_command("ls %s", "-1");
    return 0;
}
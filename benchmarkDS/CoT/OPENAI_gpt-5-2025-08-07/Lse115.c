#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

char* print_fmt(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list args2;
    va_copy(args2, args);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (len < 0) {
        va_end(args2);
        return NULL;
    }
    char* buf = (char*)malloc((size_t)len + 1);
    if (!buf) {
        va_end(args2);
        return NULL;
    }
    vsnprintf(buf, (size_t)len + 1, fmt, args2);
    va_end(args2);
    return buf;
}

char* exec_command(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return NULL;
    size_t cap = 4096;
    size_t len = 0;
    char* out = (char*)malloc(cap);
    if (!out) {
        pclose(pipe);
        return NULL;
    }
    size_t nread;
    while (!feof(pipe)) {
        if (cap - len < 2048) {
            cap *= 2;
            char* tmp = (char*)realloc(out, cap);
            if (!tmp) {
                free(out);
                pclose(pipe);
                return NULL;
            }
            out = tmp;
        }
        nread = fread(out + len, 1, cap - len - 1, pipe);
        len += nread;
    }
    out[len] = '\0';
    pclose(pipe);
    return out;
}

int main(void) {
    const char* cmds_arr[5];
    char* c1 = print_fmt("ls");
    char* c2 = print_fmt("ls %s", "-l");
    char* c3 = print_fmt("ls %s %s", "-a", ".");
    char* c4 = print_fmt("ls %s %s", "-1", "..");
    char* c5 = print_fmt("ls %s %s %s", "-l", "-a", ".");
    cmds_arr[0] = c1;
    cmds_arr[1] = c2;
    cmds_arr[2] = c3;
    cmds_arr[3] = c4;
    cmds_arr[4] = c5;

    for (int i = 0; i < 5; ++i) {
        printf("Command #%d: %s\n", i + 1, cmds_arr[i]);
        char* out = exec_command(cmds_arr[i]);
        printf("Output:\n%s\n", out ? out : "");
        free(out);
    }

    free(c1);
    free(c2);
    free(c3);
    free(c4);
    free(c5);

    return 0;
}
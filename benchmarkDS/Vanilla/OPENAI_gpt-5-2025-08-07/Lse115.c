#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

char* print(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (needed < 0) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    char* buf = (char*)malloc((size_t)needed + 1);
    if (!buf) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    va_start(args, fmt);
    vsnprintf(buf, (size_t)needed + 1, fmt, args);
    va_end(args);
    return buf;
}

char* exec_command(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    char* result = NULL;
    size_t cap = 0, len = 0;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        size_t blen = strlen(buffer);
        if (len + blen + 1 > cap) {
            size_t ncap = cap ? cap * 2 : 4096;
            while (ncap < len + blen + 1) ncap *= 2;
            char* nres = (char*)realloc(result, ncap);
            if (!nres) {
                free(result);
                pclose(pipe);
                char* empty = (char*)malloc(1);
                if (empty) empty[0] = '\0';
                return empty;
            }
            result = nres;
            cap = ncap;
        }
        memcpy(result + len, buffer, blen);
        len += blen;
        result[len] = '\0';
    }
    pclose(pipe);
    if (!result) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    return result;
}

int main(void) {
    char* cmds[5];
    cmds[0] = print("ls %s", "-1");
    cmds[1] = print("ls");
    cmds[2] = print("ls %s %s", "-a", "-1");
    cmds[3] = print("ls %s %s", "-l", ".");
    cmds[4] = print("ls %s", "this_path_does_not_exist");

    for (int i = 0; i < 5; ++i) {
        printf("Command: %s\n", cmds[i]);
        char* out = exec_command(cmds[i]);
        fputs(out, stdout);
        free(out);
        free(cmds[i]);
    }
    return 0;
}
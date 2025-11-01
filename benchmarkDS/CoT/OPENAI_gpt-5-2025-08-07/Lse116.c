#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

char* print(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);
    char* buf = (char*)malloc((size_t)needed + 1);
    if (!buf) {
        va_end(args);
        return NULL;
    }
    vsnprintf(buf, (size_t)needed + 1, fmt, args);
    va_end(args);
    return buf;
}

int run_ls(const char* options, const char* directory) {
    const char* opt = options ? options : "";
    const char* dir = (directory && directory[0] != '\0') ? directory : ".";
    char* cmd = print("ls %s %s", opt, dir);
    if (!cmd) return -1;
    int ret = system(cmd);
    free(cmd);
    return ret;
}

int main() {
    run_ls("", ".");
    run_ls("-l", ".");
    run_ls("-a", ".");
    run_ls("-la", ".");
    run_ls("", "..");
    return 0;
}
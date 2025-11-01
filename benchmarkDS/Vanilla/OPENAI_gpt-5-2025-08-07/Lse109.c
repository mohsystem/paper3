#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* exec_command(const char* command) {
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t cap = 4096;
    size_t len = 0;
    char* result = (char*)malloc(cap);
    if (!result) {
        pclose(pipe);
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), pipe)) > 0) {
        if (len + n + 1 > cap) {
            size_t new_cap = (len + n + 1) * 2;
            char* tmp = (char*)realloc(result, new_cap);
            if (!tmp) {
                free(result);
                pclose(pipe);
                char* empty = (char*)malloc(1);
                if (empty) empty[0] = '\0';
                return empty;
            }
            result = tmp;
            cap = new_cap;
        }
        memcpy(result + len, buf, n);
        len += n;
    }
    result[len] = '\0';
    pclose(pipe);
    return result;
}

char* get_user_info(const char* username) {
    const char* prefix = "cat /etc/passwd | grep ";
    size_t cmd_len = strlen(prefix) + strlen(username) + 1;
    char* cmd = (char*)malloc(cmd_len);
    if (!cmd) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    snprintf(cmd, cmd_len, "%s%s", prefix, username);
    char* output = exec_command(cmd);
    free(cmd);
    return output;
}

int main(int argc, char** argv) {
    if (argc > 1) {
        char* out = get_user_info(argv[1]);
        fputs(out, stdout);
        free(out);
    } else {
        const char* tests[5] = {"root", "daemon", "bin", "sys", "nobody"};
        for (int i = 0; i < 5; ++i) {
            char* out = get_user_info(tests[i]);
            fputs(out, stdout);
            free(out);
        }
    }
    return 0;
}
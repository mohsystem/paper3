#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char* shell_quote(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    size_t cap = len * 4 + 3;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    char* p = out;
    *p++ = '\'';
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '\'') {
            memcpy(p, "'\\''", 4);
            p += 4;
        } else {
            *p++ = s[i];
        }
    }
    *p++ = '\'';
    *p = '\0';
    return out;
}

int getent_to_file(const char* username, FILE* output) {
    if (!username || !output) return -1;
    fflush(output);
    int out_fd = fileno(output);
    if (out_fd < 0) return -1;

    int saved = dup(STDOUT_FILENO);
    if (saved < 0) return -1;

    int rc = -1;
    if (dup2(out_fd, STDOUT_FILENO) >= 0) {
        char* q = shell_quote(username);
        if (q) {
            size_t cmdlen = strlen("getent passwd ") + strlen(q) + 1;
            char* cmd = (char*)malloc(cmdlen);
            if (cmd) {
                snprintf(cmd, cmdlen, "getent passwd %s", q);
                rc = system(cmd);
                free(cmd);
            }
            free(q);
        }
        fflush(stdout);
        dup2(saved, STDOUT_FILENO);
    }
    close(saved);
    return rc;
}

int main(void) {
    const char* users[5] = {"root", "nobody", "daemon", NULL, "no_such_user_xyz_12345"};
    const char* env_user = getenv("USER");
    users[3] = (env_user && env_user[0]) ? env_user : "root";

    const char* files[5] = {"c_out1.txt", "c_out2.txt", "c_out3.txt", "c_out4.txt", "c_out5.txt"};

    for (int i = 0; i < 5; ++i) {
        FILE* f = fopen(files[i], "w");
        if (f) {
            getent_to_file(users[i], f);
            fclose(f);
        }
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

char* write_secret(const char* filename, const char* secret) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        return NULL;
    }

    size_t len = strlen(secret);
    ssize_t written = write(fd, secret, len);
    close(fd);
    if (written < 0 || (size_t)written != len) {
        return NULL;
    }

    chmod(filename, S_IRUSR); // 0400: read-only for owner

    char* out = (char*)malloc(strlen(filename) + 1);
    if (!out) return NULL;
    strcpy(out, filename);
    return out;
}

int main(void) {
    for (int i = 1; i <= 5; ++i) {
        char fname[256];
        snprintf(fname, sizeof(fname), "secret_c_%ld_%d.txt", (long)time(NULL), i);
        char secret[128];
        snprintf(secret, sizeof(secret), "Super secret info #%d", i);

        char* name = write_secret(fname, secret);
        if (name) {
            printf("%s\n", name);
            free(name);
        } else {
            fprintf(stderr, "Failed to write secret for test %d\n", i);
            return 1;
        }
    }
    return 0;
}
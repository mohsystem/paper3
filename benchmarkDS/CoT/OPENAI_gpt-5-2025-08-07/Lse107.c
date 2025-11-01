#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

static void mode_to_perm_string(mode_t mode, char* out, size_t outsz) {
    char perms[10];
    perms[0] = (mode & S_IRUSR) ? 'r' : '-';
    perms[1] = (mode & S_IWUSR) ? 'w' : '-';
    perms[2] = (mode & S_IXUSR) ? 'x' : '-';
    perms[3] = (mode & S_IRGRP) ? 'r' : '-';
    perms[4] = (mode & S_IWGRP) ? 'w' : '-';
    perms[5] = (mode & S_IXGRP) ? 'x' : '-';
    perms[6] = (mode & S_IROTH) ? 'r' : '-';
    perms[7] = (mode & S_IWOTH) ? 'w' : '-';
    perms[8] = (mode & S_IXOTH) ? 'x' : '-';
    perms[9] = '\0';
    snprintf(out, outsz, "%04o %s", (unsigned)(mode & 0777), perms);
}

char* secure_write_restrict_print(const char* path, const char* content) {
    int fd = -1;
    char* result = NULL;

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) {
        size_t len = 32 + strlen(strerror(errno));
        result = (char*)malloc(len);
        if (result) snprintf(result, len, "ERROR: open failed: %s", strerror(errno));
        return result ? result : strdup("ERROR: open failed");
    }

    size_t total = strlen(content);
    size_t written = 0;
    while (written < total) {
        ssize_t n = write(fd, content + written, total - written);
        if (n < 0) {
            int err = errno;
            close(fd);
            size_t len = 34 + strlen(strerror(err));
            result = (char*)malloc(len);
            if (result) snprintf(result, len, "ERROR: write failed: %s", strerror(err));
            return result ? result : strdup("ERROR: write failed");
        }
        written += (size_t)n;
    }

    if (fchmod(fd, 0600) == -1) {
        int err = errno;
        close(fd);
        size_t len = 35 + strlen(strerror(err));
        result = (char*)malloc(len);
        if (result) snprintf(result, len, "ERROR: fchmod failed: %s", strerror(err));
        return result ? result : strdup("ERROR: fchmod failed");
    }

    if (close(fd) == -1) {
        size_t len = 34 + strlen(strerror(errno));
        result = (char*)malloc(len);
        if (result) snprintf(result, len, "ERROR: close failed: %s", strerror(errno));
        return result ? result : strdup("ERROR: close failed");
    }

    struct stat st;
    if (stat(path, &st) == -1) {
        size_t len = 33 + strlen(strerror(errno));
        result = (char*)malloc(len);
        if (result) snprintf(result, len, "ERROR: stat failed: %s", strerror(errno));
        return result ? result : strdup("ERROR: stat failed");
    }

    char buf[32];
    mode_to_perm_string(st.st_mode, buf, sizeof(buf));
    result = strdup(buf);
    return result ? result : strdup("");
}

int main(void) {
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    const char* path = "secret.txt";

    char* r1 = secure_write_restrict_print(path, content);
    printf("%s\n", r1); free(r1);
    char* r2 = secure_write_restrict_print(path, content);
    printf("%s\n", r2); free(r2);
    char* r3 = secure_write_restrict_print(path, content);
    printf("%s\n", r3); free(r3);
    char* r4 = secure_write_restrict_print(path, content);
    printf("%s\n", r4); free(r4);
    char* r5 = secure_write_restrict_print(path, content);
    printf("%s\n", r5); free(r5);

    return 0;
}
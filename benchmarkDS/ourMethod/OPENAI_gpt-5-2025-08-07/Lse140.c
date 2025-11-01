#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

static bool is_valid_username(const char *s) {
    if (s == NULL) return false;
    size_t len = 0;
    for (const unsigned char *p = (const unsigned char*)s; *p != '\0'; ++p) {
        unsigned char ch = *p;
        if (!(isalnum(ch) || ch == '_' || ch == '.' || ch == '-')) {
            return false;
        }
        len++;
        if (len > 64) return false;
    }
    if (len == 0) return false;
    return true;
}

static bool is_safe_path(const char *p) {
    if (p == NULL) return false;
    size_t len = strnlen(p, 4097);
    if (len == 0 || len > 4096) return false;
    if (strstr(p, "..") != NULL) return false;
    return true;
}

bool username_exists(const char *db_path, const char *username) {
    if (!is_valid_username(username)) return false;
    if (!is_safe_path(db_path)) return false;

    struct stat st;
    if (lstat(db_path, &st) != 0) return false;
    if (!S_ISREG(st.st_mode)) return false;
    if (S_ISLNK(st.st_mode)) return false;

    FILE *f = fopen(db_path, "r");
    if (!f) return false;

    char buf[512];
    while (fgets(buf, sizeof(buf), f) != NULL) {
        size_t len = strnlen(buf, sizeof(buf));
        // If line did not fit entirely, consume remainder and skip
        if (len >= sizeof(buf) - 1 && buf[sizeof(buf) - 2] != '\n') {
            int ch;
            while ((ch = fgetc(f)) != '\n' && ch != EOF) {}
            continue;
        }
        // Trim trailing newline and carriage return
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
            buf[--len] = '\0';
        }
        if (len == 0 || len > 64) continue;
        if (!is_valid_username(buf)) continue;
        if (strcmp(buf, username) == 0) {
            fclose(f);
            return true;
        }
    }

    fclose(f);
    return false;
}

static char *create_sample_db(void) {
    char *tmpl = strdup("/tmp/usersXXXXXX");
    if (!tmpl) {
        fprintf(stderr, "Setup error.\n");
        exit(1);
    }
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        fprintf(stderr, "Setup error.\n");
        free(tmpl);
        exit(1);
    }
    fchmod(fd, 0600);
    const char *data =
        "alice\n"
        "Bob_123\n"
        "charlie\n"
        "david.s\n"
        "eve-9\n";
    size_t to_write = strlen(data);
    ssize_t written = write(fd, data, to_write);
    if (written < 0 || (size_t)written != to_write) {
        close(fd);
        unlink(tmpl);
        free(tmpl);
        fprintf(stderr, "Setup write error.\n");
        exit(1);
    }
    close(fd);
    return tmpl; // caller must free and unlink
}

int main(void) {
    char *db_path = create_sample_db();

    const char *tests[5] = {
        "alice",    // true
        "bob_123",  // false (case-sensitive)
        "charlie",  // true
        "mallory",  // false
        "eve-9"     // true
    };

    for (int i = 0; i < 5; i++) {
        bool result = username_exists(db_path, tests[i]);
        printf("%s -> %s\n", tests[i], result ? "true" : "false");
    }

    unlink(db_path);
    free(db_path);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

/*
Chain-of-Through process in code generation:
1) Problem understanding: Read a file only if it is inside a predefined safe directory.
2) Security requirements: Validate input, prevent absolute paths and traversal, resolve real paths, block symlinks escape, and enforce size limits.
3) Secure coding generation: Implemented in read_from_safe_dir.
4) Code review: Uses realpath for canonicalization and strict prefix checks with size cap.
5) Secure code output: Final function and test cases provided.
*/

#define MAX_SIZE_BYTES 1048576  // 1 MiB

static bool is_allowed_char(char c) {
    if ((c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') ||
        c == '.' || c == '_' || c == '-' || c == '/')
        return true;
    return false;
}

static bool validate_requested_path(const char* s) {
    if (!s || !*s) return false;
    if (strchr(s, '\0') == NULL) return false; // should always be true, defensive
    // Disallow leading absolute slash (POSIX)
    if (s[0] == '/') return false;
    // Allowed characters only
    for (const char* p = s; *p; ++p) {
        if (!is_allowed_char(*p)) return false;
    }
    return true;
}

char* read_from_safe_dir(const char* requested_path) {
    if (!validate_requested_path(requested_path)) return NULL;

    const char* home = getenv("HOME");
    char safe_dir[PATH_MAX];
    if (home && *home) {
        snprintf(safe_dir, sizeof(safe_dir), "%s/%s", home, "safe_dir");
    } else {
        if (!getcwd(safe_dir, sizeof(safe_dir))) return NULL;
        size_t len = strlen(safe_dir);
        if (len + 9 >= sizeof(safe_dir)) return NULL;
        strcat(safe_dir, "/safe_dir");
    }

    // Ensure safe directory exists
    struct stat st;
    if (stat(safe_dir, &st) != 0) {
        if (mkdir(safe_dir, 0700) != 0) return NULL;
    } else if (!S_ISDIR(st.st_mode)) {
        return NULL;
    }

    // Resolve real path of safe_dir
    char safe_real[PATH_MAX];
    if (!realpath(safe_dir, safe_real)) return NULL;

    // Build requested full path
    char candidate[PATH_MAX];
    if (snprintf(candidate, sizeof(candidate), "%s/%s", safe_real, requested_path) >= (int)sizeof(candidate)) {
        return NULL;
    }

    // Ensure file exists and get stats
    if (stat(candidate, &st) != 0) return NULL;
    if (!S_ISREG(st.st_mode)) return NULL;
    if (st.st_size < 0 || st.st_size > MAX_SIZE_BYTES) return NULL;

    // Resolve real path of target to catch symlink escapes
    char target_real[PATH_MAX];
    if (!realpath(candidate, target_real)) return NULL;

    // Ensure target_real is under safe_real
    size_t safe_len = strlen(safe_real);
    if (strncmp(target_real, safe_real, safe_len) != 0 ||
        (target_real[safe_len] != '/' && target_real[safe_len] != '\0')) {
        return NULL;
    }

    // Read file
    FILE* f = fopen(target_real, "rb");
    if (!f) return NULL;

    size_t size = (size_t)st.st_size;
    char* buf = (char*)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t readn = fread(buf, 1, size, f);
    fclose(f);
    if (readn != size) {
        free(buf);
        return NULL;
    }
    buf[size] = '\0'; // treat as UTF-8/ASCII text
    return buf;
}

int main(void) {
    const char* tests[5] = {
        "example.txt",
        "../secret.txt",
        "/etc/passwd",
        "subdir/notes.txt",
        "subdir/../../evil"
    };

    for (int i = 0; i < 5; ++i) {
        const char* req = tests[i];
        char* content = read_from_safe_dir(req);
        printf("Request: %s\n", req);
        if (content) {
            printf("Read OK, length=%zu\n", strlen(content));
            free(content);
        } else {
            printf("Read FAILED\n");
        }
        printf("---\n");
    }
    return 0;
}
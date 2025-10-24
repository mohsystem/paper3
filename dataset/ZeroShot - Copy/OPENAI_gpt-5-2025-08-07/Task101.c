#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <libgen.h>

static int ensure_dir(const char* dir) {
    struct stat st;
    if (stat(dir, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 0 : -1;
    }
    // Attempt to create the directory
    if (mkdir(dir, 0700) == 0) return 0;
    // If parent dirs needed, try recursively
    if (errno == ENOENT) {
        char* dup = strdup(dir);
        if (!dup) return -1;
        char* parent = dirname(dup);
        if (parent && strcmp(parent, dir) != 0 && strcmp(parent, ".") != 0) {
            if (ensure_dir(parent) != 0) { free(dup); return -1; }
            int rc = mkdir(dir, 0700);
            free(dup);
            return (rc == 0 || errno == EEXIST) ? 0 : -1;
        }
        free(dup);
    }
    return (errno == EEXIST) ? 0 : -1;
}

// Returns malloc'd string path on success, or NULL on failure. Caller must free.
char* create_executable_script(const char* dirPath, const char* content, int overwrite) {
    const char* defaultContent = "#!/bin/sh\necho \"Hello from script.sh\"\n";
    const char* input = (content && content[0]) ? content : defaultContent;

    // Ensure trailing newline
    size_t inlen = strlen(input);
    int needs_nl = (inlen == 0 || input[inlen - 1] != '\n') ? 1 : 0;

    char cwd[PATH_MAX];
    const char* base = (dirPath && dirPath[0]) ? dirPath : ".";
    if (!realpath(base, cwd)) {
        // Try to create directory then resolve again
        if (ensure_dir(base) != 0) return NULL;
        if (!realpath(base, cwd)) return NULL;
    } else {
        if (ensure_dir(cwd) != 0) return NULL;
    }

    // Build final path
    char finalPath[PATH_MAX];
    snprintf(finalPath, sizeof(finalPath), "%s/%s", cwd, "script.sh");

    // Prevent traversal: ensure parent of final equals cwd
    char tmpParent[PATH_MAX];
    strncpy(tmpParent, finalPath, sizeof(tmpParent) - 1);
    tmpParent[sizeof(tmpParent) - 1] = '\0';
    char* parent = dirname(tmpParent);
    char parentReal[PATH_MAX];
    if (!realpath(parent, parentReal)) return NULL;
    if (strcmp(parentReal, cwd) != 0) return NULL;

    struct stat st;
    if (!overwrite && lstat(finalPath, &st) == 0) {
        return NULL;
    }

    // Create temp file with mkstemp
    char tmpTemplate[PATH_MAX];
    snprintf(tmpTemplate, sizeof(tmpTemplate), "%s/%s", cwd, "script.sh.tmp-XXXXXX");
    int fd = mkstemp(tmpTemplate);
    if (fd == -1) {
        return NULL;
    }
    // Set permissions to 0700
    fchmod(fd, 0700);

    // Write content
    ssize_t total = 0;
    ssize_t len = (ssize_t)inlen + (needs_nl ? 1 : 0);
    char* buf = (char*)malloc(len);
    if (!buf) {
        close(fd);
        unlink(tmpTemplate);
        return NULL;
    }
    memcpy(buf, input, inlen);
    if (needs_nl) buf[inlen] = '\n';

    while (total < len) {
        ssize_t w = write(fd, buf + total, len - total);
        if (w <= 0) {
            free(buf);
            close(fd);
            unlink(tmpTemplate);
            return NULL;
        }
        total += w;
    }
    free(buf);
    fsync(fd);
    close(fd);

    // If not overwriting, double-check existence
    if (!overwrite && lstat(finalPath, &st) == 0) {
        unlink(tmpTemplate);
        return NULL;
    }

    // Atomic rename
    if (rename(tmpTemplate, finalPath) != 0) {
        unlink(tmpTemplate);
        return NULL;
    }

    // Ensure permissions
    chmod(finalPath, 0700);

    // Ensure final is a regular file (not a symlink)
    if (lstat(finalPath, &st) != 0 || !S_ISREG(st.st_mode)) {
        unlink(finalPath);
        return NULL;
    }

    // Return heap-allocated copy of final path
    size_t outlen = strlen(finalPath);
    char* out = (char*)malloc(outlen + 1);
    if (!out) return NULL;
    memcpy(out, finalPath, outlen + 1);
    return out;
}

int main(void) {
    // Prepare temp directories for tests
    char d1[] = "/tmp/t101_c_1_XXXXXX";
    char d2[] = "/tmp/t101_c_2_XXXXXX";
    if (!mkdtemp(d1) || !mkdtemp(d2)) {
        printf("Setup failed\n");
        return 1;
    }

    char* r1 = create_executable_script(d1, "#!/bin/sh\necho Case1", 0);
    printf("Test1: %s\n", r1 ? r1 : "null");
    free(r1);

    char* r2 = create_executable_script(d1, "#!/bin/sh\necho Case2", 0); // should fail
    printf("Test2: %s\n", r2 ? r2 : "null");
    free(r2);

    char* r3 = create_executable_script(d1, "#!/bin/sh\necho Case3 Overwrite", 1); // overwrite
    printf("Test3: %s\n", r3 ? r3 : "null");
    free(r3);

    char* r4 = create_executable_script(d2, "echo Case4 Content only", 0);
    printf("Test4: %s\n", r4 ? r4 : "null");
    free(r4);

    char* r5 = create_executable_script(".", "echo Case5 in CWD", 1);
    printf("Test5: %s\n", r5 ? r5 : "null");
    free(r5);

    return 0;
}
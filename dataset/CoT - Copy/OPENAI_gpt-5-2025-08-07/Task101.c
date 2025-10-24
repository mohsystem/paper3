#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
  #include <io.h>
  #include <fcntl.h>
  #include <direct.h>
  #include <sys/stat.h>
  #define PATH_SEP "\\"
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <limits.h>
  #include <dirent.h>
  #define PATH_SEP "/"
#endif

// Step 1: Problem understanding:
// Create "script.sh" in a given directory with provided content and make it executable.

// Step 2: Security requirements:
// - Use O_CREAT|O_EXCL to avoid overwriting existing files.
// - Use O_NOFOLLOW if available to avoid symlink following.
// - Minimal required permissions (0700).
// - fsync/_commit for durability.

int create_script_file(const char* directory, const char* content, int set_executable) {
    if (directory == NULL || directory[0] == '\0') return -1;
    if (content == NULL) content = "";

    // Build target path
    size_t len = strlen(directory) + 1 + strlen("script.sh") + 1;
    char *target = (char*)malloc(len);
    if (!target) return -1;
    snprintf(target, len, "%s%s%s", directory, PATH_SEP, "script.sh");

    // Create directory if not exists
#if defined(_WIN32)
    _mkdir(directory);
    int flags = _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY;
    int mode = _S_IREAD | _S_IWRITE; // Windows doesn't use executable bit
    int fd = _open(target, flags, mode);
    if (fd == -1) { free(target); return -1; }

    const char* p = content;
    size_t remaining = strlen(content);
    while (remaining > 0) {
        int written = _write(fd, p, (unsigned int)remaining);
        if (written <= 0) { _close(fd); free(target); return -1; }
        p += written;
        remaining -= (size_t)written;
    }

    if (_commit(fd) == -1) { _close(fd); free(target); return -1; }
    _close(fd);
    free(target);
    return 0;
#else
    mkdir(directory, 0700);

    int flags = O_CREAT | O_EXCL | O_WRONLY;
    #ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
    #endif
    mode_t mode = set_executable ? (S_IRUSR | S_IWUSR | S_IXUSR) : (S_IRUSR | S_IWUSR);

    int fd = open(target, flags, mode);
    if (fd == -1) { free(target); return -1; }

    const char* p = content;
    size_t remaining = strlen(content);
    while (remaining > 0) {
        ssize_t written = write(fd, p, remaining);
        if (written <= 0) { close(fd); free(target); return -1; }
        p += written;
        remaining -= (size_t)written;
    }

    if (fsync(fd) == -1) { close(fd); free(target); return -1; }

    if (set_executable) {
        struct stat st;
        if (fstat(fd, &st) == 0) {
            mode_t newMode = st.st_mode | S_IXUSR;
            fchmod(fd, newMode);
        }
    }

    close(fd);
    free(target);
    return 0;
#endif
}

// Step 4 and 5: Code review & Secure code output: Incorporated in implementation.

int main(void) {
    for (int i = 1; i <= 5; ++i) {
        char dirbuf[512];
#if defined(_WIN32)
        const char* tmp = getenv("TEMP");
        if (!tmp) tmp = ".";
        snprintf(dirbuf, sizeof(dirbuf), "%s%stask101_c_test_%d", tmp, PATH_SEP, i);
        _mkdir(dirbuf);
#else
        const char* tmp = getenv("TMPDIR");
        if (!tmp) tmp = "/tmp";
        snprintf(dirbuf, sizeof(dirbuf), "%s%stask101_c_test_%d", tmp, PATH_SEP, i);
        mkdir(dirbuf, 0700);
#endif
        char content[256];
        snprintf(content, sizeof(content), "#!/bin/sh\necho \"Hello from C test %d\"\n", i);
        int ok = create_script_file(dirbuf, content, 1);

        char scriptpath[600];
        snprintf(scriptpath, sizeof(scriptpath), "%s%sscript.sh", dirbuf, PATH_SEP);

        printf("Test %d dir: %s\n", i, dirbuf);
        printf("  Created: %s\n", ok == 0 ? "true" : "false");
        FILE* f = fopen(scriptpath, "rb");
        printf("  Exists: %s\n", f ? "true" : "false");
        if (f) fclose(f);
#if defined(_WIN32)
        printf("  Executable: n/a on Windows\n");
#else
        printf("  Executable: %s\n", access(scriptpath, X_OK) == 0 ? "true" : "false");
#endif
    }
    return 0;
}
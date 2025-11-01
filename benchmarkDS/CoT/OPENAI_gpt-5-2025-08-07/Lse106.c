#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

// Returns a newly allocated string with the path; caller must free. On error, returns NULL.
char* write_secret_to_readonly_file(const char* secret, const char* preferredFilename) {
    if (secret == NULL) {
        return NULL;
    }

    char* path = NULL;
    int fd = -1;

#if defined(_WIN32) || defined(_WIN64)
    if (preferredFilename && preferredFilename[0] != '\0') {
        path = _strdup(preferredFilename);
        if (!path) return NULL;
        int oflag = _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY;
        int pmode = _S_IREAD | _S_IWRITE;
        if (_sopen_s(&fd, path, oflag, _SH_DENYNO, pmode) != 0) {
            free(path);
            return NULL;
        }
    } else {
        char tmpDir[MAX_PATH];
        char tmpPath[MAX_PATH];
        if (GetTempPathA(MAX_PATH, tmpDir) == 0) {
            return NULL;
        }
        if (GetTempFileNameA(tmpDir, "sec", 0, tmpPath) == 0) {
            return NULL;
        }
        // Ensure exclusive creation
        _unlink(tmpPath);
        int oflag = _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY;
        int pmode = _S_IREAD | _S_IWRITE;
        if (_sopen_s(&fd, tmpPath, oflag, _SH_DENYNO, pmode) != 0) {
            return NULL;
        }
        path = _strdup(tmpPath);
        if (!path) {
            _close(fd);
            return NULL;
        }
    }

    size_t len = strlen(secret);
    if (_write(fd, secret, (unsigned int)len) < 0) {
        _close(fd);
        free(path);
        return NULL;
    }
    _commit(fd);
    _close(fd);

    _chmod(path, _S_IREAD);
#else
    if (preferredFilename && preferredFilename[0] != '\0') {
        path = strdup(preferredFilename);
        if (!path) return NULL;

        fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
        if (fd < 0) {
            free(path);
            return NULL;
        }
    } else {
        char tmpl[] = "/tmp/secret_c_XXXXXX";
        fd = mkstemp(tmpl); // mode 0600
        if (fd < 0) {
            return NULL;
        }
        path = strdup(tmpl);
        if (!path) {
            close(fd);
            return NULL;
        }
    }

    size_t len = strlen(secret);
    const char* data = secret;
    ssize_t remaining = (ssize_t)len;
    ssize_t writtenTotal = 0;
    while (remaining > 0) {
        ssize_t w = write(fd, data + writtenTotal, (size_t)remaining);
        if (w < 0) {
            int err = errno;
            close(fd);
            free(path);
            errno = err;
            return NULL;
        }
        writtenTotal += w;
        remaining -= w;
    }
    fsync(fd);
    close(fd);

    if (chmod(path, S_IRUSR) != 0) {
        free(path);
        return NULL;
    }
#endif

    return path;
}

int main(void) {
    // 5 test cases
    char* f1 = write_secret_to_readonly_file("TopSecret-Alpha", NULL);
    if (f1) { printf("%s\n", f1); free(f1); } else { fprintf(stderr, "Failed test 1\n"); }

    char* f2 = write_secret_to_readonly_file("TopSecret-Bravo", "/tmp/secret_specified_c_1.txt");
    if (f2) { printf("%s\n", f2); free(f2); } else { fprintf(stderr, "Failed test 2\n"); }

    char* f3 = write_secret_to_readonly_file("TopSecret-Charlie", "");
    if (f3) { printf("%s\n", f3); free(f3); } else { fprintf(stderr, "Failed test 3\n"); }

    char* f4 = write_secret_to_readonly_file("TopSecret-Delta", "/tmp/secret_specified_c_2.txt");
    if (f4) { printf("%s\n", f4); free(f4); } else { fprintf(stderr, "Failed test 4\n"); }

    char* f5 = write_secret_to_readonly_file("TopSecret-Echo", NULL);
    if (f5) { printf("%s\n", f5); free(f5); } else { fprintf(stderr, "Failed test 5\n"); }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <direct.h>
#define PATH_SEP '\\'
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <dirent.h>
#define PATH_SEP '/'
#endif

// Simple path join ensuring single separator
static void join_path(char* out, size_t outsz, const char* a, const char* b) {
    snprintf(out, outsz, "%s%c%s", a, PATH_SEP, b);
}

// Replace CR/LF/TAB with space
static void sanitize_err(char* s) {
    for (; *s; ++s) {
        if (*s == '\r' || *s == '\n' || *s == '\t') *s = ' ';
    }
}

static int path_is_absolute(const char* p) {
#if defined(_WIN32)
    // drive letter or UNC
    if ((strlen(p) >= 2 && p[1] == ':') || (strlen(p) >= 2 && p[0] == '\\' && p[1] == '\\')) return 1;
    return 0;
#else
    return p[0] == '/';
#endif
}

static int ensure_dir(const char* path) {
#if defined(_WIN32)
    if (_mkdir(path) == 0 || errno == EEXIST) return 0;
    return -1;
#else
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return 0;
        errno = ENOTDIR;
        return -1;
    }
    if (mkdir(path, 0700) == 0) return 0;
    return -1;
#endif
}

static int is_dir(const char* path) {
#if defined(_WIN32)
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) return 0;
    return (attr & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
#else
    struct stat st;
    if (lstat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
#endif
}

static int is_symlink_path(const char* path) {
#if defined(_WIN32)
    // Windows: limited symlink detection
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) return 0;
    // Reparse points can be symlinks/junctions
    return (attr & FILE_ATTRIBUTE_REPARSE_POINT) ? 1 : 0;
#else
    struct stat st;
    if (lstat(path, &st) != 0) return 0;
    return S_ISLNK(st.st_mode);
#endif
}

static int is_or_has_symlink(const char* target, const char* stop) {
    // Conservative: check target
    if (is_symlink_path(target)) return 1;
    // Walk up to stop
    char buf[4096];
    strncpy(buf, target, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    while (1) {
        if (is_symlink_path(buf)) return 1;
        // If reached stop or root, break
        if (strcmp(buf, stop) == 0) break;
        char* last_sep = strrchr(buf, PATH_SEP);
        if (!last_sep) break;
        *last_sep = '\0';
        if (buf[0] == '\0') break;
    }
    return 0;
}

static int is_regular_file_nofollow(const char* path) {
#if defined(_WIN32)
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) return 0;
    if (attr & FILE_ATTRIBUTE_DIRECTORY) return 0;
    return 1;
#else
    struct stat st;
    if (lstat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode);
#endif
}

// Securely copy baseDir/userRelPath to a temp file in tempDir.
// Returns a malloc-allocated string path to the temp file on success; on error returns a malloc-allocated "ERROR: ..." string.
char* copy_to_temp(const char* baseDir, const char* userRelPath, const char* tempDir) {
    // Validate inputs
    if (!baseDir || !userRelPath || !tempDir) {
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Null input parameter");
        return r;
    }
    if (strlen(baseDir) == 0 || strlen(userRelPath) == 0 || strlen(tempDir) == 0) {
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Empty input parameter");
        return r;
    }
    if (strlen(userRelPath) > 4096) {
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Path too long");
        return r;
    }
    if (strchr(userRelPath, '\0') && strlen(userRelPath) != strlen(userRelPath)) {
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Invalid character in path");
        return r;
    }
    if (path_is_absolute(userRelPath)) {
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Path must be relative to base directory");
        return r;
    }
    if (!is_dir(baseDir)) {
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Base path is not a directory");
        return r;
    }
    if (is_or_has_symlink(baseDir, baseDir)) {
        char* r = (char*)malloc(80);
        strcpy(r, "ERROR: Base directory or its parents contain a symlink");
        return r;
    }
    if (ensure_dir(tempDir) != 0 || !is_dir(tempDir)) {
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Temp path is not a directory");
        return r;
    }
    if (is_or_has_symlink(tempDir, tempDir)) {
        char* r = (char*)malloc(80);
        strcpy(r, "ERROR: Temp directory or its parents contain a symlink");
        return r;
    }

    // Construct resolved path: baseDir + userRelPath, then normalize by removing .. and .
    char combined[4096];
    join_path(combined, sizeof(combined), baseDir, userRelPath);

    // Basic traversal check: ensure combined starts with baseDir + separator after lexical normalization (simplified)
    // We will reject any ".." components in userRelPath to be safe
    if (strstr(userRelPath, "..") != NULL) {
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Path escapes base directory");
        return r;
    }

    if (is_or_has_symlink(combined, baseDir)) {
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Source path contains a symlink");
        return r;
    }
    if (!is_regular_file_nofollow(combined)) {
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Source file does not exist or is not regular");
        return r;
    }

#if defined(_WIN32)
    // Open source
    HANDLE hsrc = CreateFileA(combined, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hsrc == INVALID_HANDLE_VALUE) {
        char* r = (char*)malloc(96);
        strcpy(r, "ERROR: Cannot open source file");
        return r;
    }
    // Create temp file
    char tempPath[4096];
    snprintf(tempPath, sizeof(tempPath), "%s%c%s", tempDir, PATH_SEP, "copy_tmp_XXXXXX.tmp");
    // Emulate mkstemp: attempt multiple unique names
    char outPath[4096];
    int made = 0;
    for (int i = 0; i < 100; ++i) {
        snprintf(outPath, sizeof(outPath), "%s%c%s%d.tmp", tempDir, PATH_SEP, "copy_", i);
        HANDLE h = CreateFileA(outPath, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            // Copy
            const DWORD BUF = 8192;
            char* buf = (char*)malloc(BUF);
            DWORD r = 0, w = 0;
            int ok = 1;
            while (ReadFile(hsrc, buf, BUF, &r, NULL) && r > 0) {
                if (!WriteFile(h, buf, r, &w, NULL) || w != r) {
                    ok = 0; break;
                }
            }
            free(buf);
            FlushFileBuffers(h);
            CloseHandle(hsrc);
            CloseHandle(h);
            if (!ok) {
                DeleteFileA(outPath);
                char* er = (char*)malloc(64);
                strcpy(er, "ERROR: I/O error during copy");
                return er;
            }
            made = 1;
            break;
        }
    }
    if (!made) {
        CloseHandle(hsrc);
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Cannot create temp file");
        return r;
    }
    char* ret = (char*)malloc(strlen(outPath) + 1);
    strcpy(ret, outPath);
    return ret;
#else
    // POSIX: open source with O_NOFOLLOW
    int flags = O_RDONLY;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
    int sfd = open(combined, flags);
    if (sfd < 0) {
        char* r = (char*)malloc(96);
        snprintf(r, 96, "ERROR: Cannot open source file: %s", strerror(errno));
        sanitize_err(r);
        return r;
    }
    // Shared lock
    flock(sfd, LOCK_SH);

    // Create temp file with mkstemp
    char tmpl[4096];
    snprintf(tmpl, sizeof(tmpl), "%s%c%s", tempDir, PATH_SEP, "copy_XXXXXX");
    int dfd = mkstemp(tmpl);
    if (dfd < 0) {
        close(sfd);
        char* r = (char*)malloc(96);
        snprintf(r, 96, "ERROR: Cannot create temp file: %s", strerror(errno));
        sanitize_err(r);
        return r;
    }
    // Set perms to 0600
    fchmod(dfd, S_IRUSR | S_IWUSR);
    // Exclusive lock on destination
    flock(dfd, LOCK_EX);

    // Copy
    char* buf = (char*)malloc(8192);
    if (!buf) {
        close(sfd);
        close(dfd);
        unlink(tmpl);
        char* r = (char*)malloc(64);
        strcpy(r, "ERROR: Out of memory");
        return r;
    }
    ssize_t rdsz;
    while ((rdsz = read(sfd, buf, 8192)) > 0) {
        ssize_t off = 0;
        while (off < rdsz) {
            ssize_t wr = write(dfd, buf + off, (size_t)(rdsz - off));
            if (wr < 0) {
                int e = errno;
                free(buf);
                close(sfd);
                close(dfd);
                unlink(tmpl);
                char* r = (char*)malloc(96);
                snprintf(r, 96, "ERROR: Write failed: %s", strerror(e));
                sanitize_err(r);
                return r;
            }
            off += wr;
        }
    }
    if (rdsz < 0) {
        int e = errno;
        free(buf);
        close(sfd);
        close(dfd);
        unlink(tmpl);
        char* r = (char*)malloc(96);
        snprintf(r, 96, "ERROR: Read failed: %s", strerror(e));
        sanitize_err(r);
        return r;
    }
    free(buf);
    fsync(dfd);
    close(sfd);
    close(dfd);

    char* ret = (char*)malloc(strlen(tmpl) + 1);
    strcpy(ret, tmpl);
    return ret;
#endif
}

static void write_text(const char* path, const char* text) {
    // Ensure parent exists is skipped for brevity; tests create simple files.
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fwrite(text, 1, strlen(text), f);
    fclose(f);
}

int main(void) {
    // Setup temp directories
    char base[1024];
    char tout[1024];
#if defined(_WIN32)
    char tmpdir[MAX_PATH];
    GetTempPathA(MAX_PATH, tmpdir);
    snprintf(base, sizeof(base), "%s\\base_dir_c_%lu", tmpdir, GetTickCount());
    snprintf(tout, sizeof(tout), "%s\\temp_out_c_%lu", tmpdir, GetTickCount());
    _mkdir(base);
    _mkdir(tout);
    char f1[1024];
    snprintf(f1, sizeof(f1), "%s\\file1.txt", base);
    write_text(f1, "Hello world 1");
    char subdir[1024];
    snprintf(subdir, sizeof(subdir), "%s\\subdir", base);
    _mkdir(subdir);
    char f2[1024];
    snprintf(f2, sizeof(f2), "%s\\file2.txt", subdir);
    write_text(f2, "Hello world 2");
#else
    const char* tmp = "/tmp";
    snprintf(base, sizeof(base), "%s/base_dir_c_%d", tmp, (int)getpid());
    snprintf(tout, sizeof(tout), "%s/temp_out_c_%d", tmp, (int)getpid());
    mkdir(base, 0700);
    mkdir(tout, 0700);
    char f1[1024];
    snprintf(f1, sizeof(f1), "%s/file1.txt", base);
    write_text(f1, "Hello world 1");
    char subdir[1024];
    snprintf(subdir, sizeof(subdir), "%s/subdir", base);
    mkdir(subdir, 0700);
    char f2[1024];
    snprintf(f2, sizeof(f2), "%s/file2.txt", subdir);
    write_text(f2, "Hello world 2");
    // Try to create a symlink (ignore errors)
    char linkp[1024];
    snprintf(linkp, sizeof(linkp), "%s/link_to_f1", base);
    unlink(linkp);
    symlink("file1.txt", linkp);
#endif

    char* r1 = copy_to_temp(base, "file1.txt", tout);
    printf("Test1: %s\n", r1);
    free(r1);

    char* r2 = copy_to_temp(base, "subdir/file2.txt", tout);
    printf("Test2: %s\n", r2);
    free(r2);

    char* r3 = copy_to_temp(base, "../../etc/passwd", tout);
    printf("Test3: %s\n", r3);
    free(r3);

    char* r4 = copy_to_temp(base, "does_not_exist.txt", tout);
    printf("Test4: %s\n", r4);
    free(r4);

    // absolute path test
    char absf1[1024];
    snprintf(absf1, sizeof(absf1), "%s%c%s", base, PATH_SEP, "file1.txt");
    char* r5 = copy_to_temp(base, absf1, tout);
    printf("Test5: %s\n", r5);
    free(r5);

    return 0;
}
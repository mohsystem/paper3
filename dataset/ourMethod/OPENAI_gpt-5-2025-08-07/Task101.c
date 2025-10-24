#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#define PATH_SEP "\\"
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#define PATH_SEP "/"
#endif

static int ensure_secure_dir(const char *dir) {
    if (!dir || !*dir) return 0;

    // Create directories recursively
    char path[4096];
    size_t len = strnlen(dir, sizeof(path));
    if (len == 0 || len >= sizeof(path)) return 0;
    strncpy(path, dir, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    // Normalize simple trailing separators
    while (len > 1 && (path[len - 1] == '/' || path[len - 1] == '\\')) {
        path[len - 1] = '\0';
        len--;
    }

    char temp[4096];
    size_t n = 0;
    for (size_t i = 0; i < len; ++i) {
        char c = path[i];
        temp[n++] = c;
        temp[n] = '\0';
        if (c == '/' || c == '\\' || i == len - 1) {
            // skip drive letter like C:\
#if defined(_WIN32)
            if (n == 3 && temp[1] == ':' && (temp[2] == '\\' || temp[2] == '/')) {
                continue;
            }
#endif
            // Skip root "/"
            if (n == 1 && (temp[0] == '/' || temp[0] == '\\')) {
                continue;
            }
#if defined(_WIN32)
            if (_mkdir(temp) == -1) {
                if (errno != EEXIST) {
                    // Continue if it exists as directory; else fail
                    struct _stat st;
                    if (_stat(temp, &st) != 0 || !(st.st_mode & _S_IFDIR)) {
                        return 0;
                    }
                }
            }
            // Windows lacks POSIX perms; best-effort skip
#else
            if (mkdir(temp, 0700) == -1) {
                if (errno != EEXIST) {
                    struct stat st;
                    if (lstat(temp, &st) != 0 || !S_ISDIR(st.st_mode)) {
                        return 0;
                    }
                }
            } else {
                chmod(temp, 0700);
            }
            // Refuse symlinked components
            {
                struct stat st;
                if (lstat(temp, &st) == 0) {
                    if (S_ISLNK(st.st_mode)) {
                        return 0;
                    }
                }
            }
#endif
        }
    }

    // Final check is directory
#if defined(_WIN32)
    {
        struct _stat st;
        if (_stat(path, &st) != 0) return 0;
        if (!(st.st_mode & _S_IFDIR)) return 0;
    }
#else
    {
        struct stat st;
        if (lstat(path, &st) != 0) return 0;
        if (!S_ISDIR(st.st_mode)) return 0;
        chmod(path, 0700);
    }
#endif
    return 1;
}

static int path_join(const char *a, const char *b, char *out, size_t outsz) {
    if (!a || !b || !out || outsz == 0) return 0;
    size_t la = strnlen(a, outsz);
    size_t lb = strnlen(b, outsz);
    if (la + 1 + lb + 1 > outsz) return 0;
    snprintf(out, outsz, "%s%s%s", a, (la > 0 && (a[la-1] == '/' || a[la-1] == '\\')) ? "" : PATH_SEP, b);
    return 1;
}

int create_executable_script(const char *base_dir, const char *content) {
    if (!base_dir || !content) return 0;
    if (strlen(base_dir) == 0 || strlen(base_dir) > 4096) return 0;

    if (!ensure_secure_dir(base_dir)) return 0;

    char final_path[4096];
    if (!path_join(base_dir, "script.sh", final_path, sizeof(final_path))) return 0;

    // Refuse if final exists already
#if defined(_WIN32)
    {
        struct _stat st;
        if (_stat(final_path, &st) == 0) {
            return 0;
        }
    }
#else
    {
        struct stat st;
        if (lstat(final_path, &st) == 0) {
            return 0;
        }
    }
#endif

    // Create temp file
    char temp_path[4096];
    char rnd[33];
    // Simple random-ish string
    {
        unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();
        for (int i = 0; i < 16; ++i) {
            seed = seed * 1103515245u + 12345u;
            unsigned int v = (seed >> 16) & 0xFFu;
            static const char *hex = "0123456789abcdef";
            rnd[i*2] = hex[(v >> 4) & 0xF];
            rnd[i*2 + 1] = hex[v & 0xF];
        }
        rnd[32] = '\0';
    }
    char tmpname[64];
    snprintf(tmpname, sizeof(tmpname), ".script.sh.tmp-%s", rnd);
    if (!path_join(base_dir, tmpname, temp_path, sizeof(temp_path))) return 0;

#if defined(_WIN32)
    int fd = _open(temp_path, _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
    if (fd < 0) {
        return 0;
    }
#else
    int fd = open(temp_path, O_CREAT | O_EXCL | O_WRONLY
#ifdef O_CLOEXEC
        | O_CLOEXEC
#endif
        , 0600);
    if (fd < 0) {
        return 0;
    }
#endif

    size_t remaining = strlen(content);
    const char *p = content;
    while (remaining > 0) {
#if defined(_WIN32)
        int w = _write(fd, p, (unsigned int)remaining);
#else
        ssize_t w = write(fd, p, remaining);
#endif
        if (w < 0) {
            int err = errno;
#if defined(_WIN32)
            _close(fd);
            _unlink(temp_path);
#else
            close(fd);
            unlink(temp_path);
#endif
            (void)err;
            return 0;
        }
        p += w;
        remaining -= (size_t)w;
    }

#if defined(_WIN32)
    _commit(fd);
    _close(fd);
#else
    fsync(fd);
    close(fd);
    chmod(temp_path, 0700);
#endif

    // Publish without overwrite
    int published = 0;
#if defined(_WIN32)
    // On Windows, rename fails if target exists; good for no-overwrite
    if (rename(temp_path, final_path) == 0) {
        published = 1;
    } else {
        _unlink(temp_path);
    }
#else
    if (link(temp_path, final_path) == 0) {
        unlink(temp_path);
        published = 1;
    } else {
        // Fallback to rename within same dir if final still doesn't exist
        struct stat st;
        if (lstat(final_path, &st) != 0) {
            if (rename(temp_path, final_path) == 0) {
                published = 1;
            } else {
                unlink(temp_path);
            }
        } else {
            unlink(temp_path);
        }
    }
    if (published) {
        chmod(final_path, 0700);
    }
#endif

    return published ? 1 : 0;
}

static void tmp_base(const char *suffix, char *out, size_t outsz) {
#if defined(_WIN32)
    const char *t = getenv("TEMP");
    if (!t) t = ".";
#else
    const char *t = getenv("TMPDIR");
    if (!t) t = "/tmp";
#endif
    snprintf(out, outsz, "%s%sTask101_%s", t, PATH_SEP, suffix);
}

int main(void) {
    const char *content = "#!/bin/sh\necho \"Hello from script\"\n";

    char t1[4096]; tmp_base("test1", t1, sizeof(t1));
    printf("Test1: %s\n", create_executable_script(t1, content) ? "true" : "false");

    char t2dir[4096]; tmp_base("test2", t2dir, sizeof(t2dir));
    char t2[4096];
    snprintf(t2, sizeof(t2), "%s%snested", t2dir, PATH_SEP);
    printf("Test2: %s\n", create_executable_script(t2, content) ? "true" : "false");

    char base3a[4096]; tmp_base("test3", base3a, sizeof(base3a));
    char t3[4096];
    snprintf(t3, sizeof(t3), "%s%sa..%sb", base3a, PATH_SEP, PATH_SEP); // will normalize in ensure logic
    printf("Test3: %s\n", create_executable_script(t3, content) ? "true" : "false");

    // Test 4: Pre-existing script.sh
    char t4[4096]; tmp_base("test4", t4, sizeof(t4));
    ensure_secure_dir(t4);
    char pre[4096];
    path_join(t4, "script.sh", pre, sizeof(pre));
#if defined(_WIN32)
    int pfd = _open(pre, _O_CREAT | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
    if (pfd >= 0) {
        const char *s = "#!/bin/sh\necho preexisting\n";
        _write(pfd, s, (unsigned int)strlen(s));
        _close(pfd);
    }
#else
    int pfd = open(pre, O_CREAT | O_WRONLY, 0600);
    if (pfd >= 0) {
        const char *s = "#!/bin/sh\necho preexisting\n";
        write(pfd, s, strlen(s));
        close(pfd);
    }
#endif
    printf("Test4 (exists): %s\n", create_executable_script(t4, content) ? "true" : "false");

    // Test 5: Base path is a file
    char t5file[4096]; tmp_base("test5_as_file", t5file, sizeof(t5file));
#if defined(_WIN32)
    int ffd = _open(t5file, _O_CREAT | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
    if (ffd >= 0) { const char *s = "not a dir"; _write(ffd, s, (unsigned int)strlen(s)); _close(ffd); }
#else
    int ffd = open(t5file, O_CREAT | O_WRONLY, 0600);
    if (ffd >= 0) { const char *s = "not a dir"; write(ffd, s, strlen(s)); close(ffd); }
#endif
    printf("Test5 (base is file): %s\n", create_executable_script(t5file, content) ? "true" : "false");

    return 0;
}
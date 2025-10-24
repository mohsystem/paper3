#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#endif

#ifdef _WIN32
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

static int is_directory(const char* path) {
    if (!path) return 0;
#if defined(_WIN32)
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) return 0;
    return (attr & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
#else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode) ? 1 : 0;
#endif
}

static const char* get_temp_dir() {
#if defined(_WIN32)
    static char buf[MAX_PATH];
    DWORD len = GetTempPathA(MAX_PATH, buf);
    if (len > 0 && len < MAX_PATH) return buf;
    return ".";
#else
    const char* env;
    env = getenv("TMPDIR");
    if (env && *env) return env;
    env = getenv("TEMP");
    if (env && *env) return env;
    env = getenv("TMP");
    if (env && *env) return env;
    return "/tmp";
#endif
}

static char* join_path(const char* dir, const char* file) {
    size_t ld = strlen(dir);
    size_t lf = strlen(file);
    int need_sep = (ld > 0 && dir[ld - 1] != PATH_SEP);
    char* out = (char*)malloc(ld + need_sep + lf + 1);
    if (!out) return NULL;
    memcpy(out, dir, ld);
    if (need_sep) out[ld++] = PATH_SEP;
    memcpy(out + ld, file, lf);
    out[ld + lf] = '\0';
    return out;
}

static char* make_unique_temp_file(const char* prefix, const char* suffix, int* out_fd) {
    const char* tdir = get_temp_dir();
#if defined(_WIN32)
    // Create a unique file name using GetTempFileName, then append suffix by renaming
    char tmpPath[MAX_PATH];
    if (!GetTempFileNameA(tdir, "cpy", 0, tmpPath)) {
        return NULL;
    }
    // If suffix provided and not empty, rename to include suffix
    if (suffix && *suffix) {
        size_t len = strlen(tmpPath);
        char* renamed = (char*)malloc(len + strlen(suffix) + 1);
        if (!renamed) {
            DeleteFileA(tmpPath);
            return NULL;
        }
        strcpy(renamed, tmpPath);
        strcat(renamed, suffix);
        if (!MoveFileA(tmpPath, renamed)) {
            // If rename fails, fallback to original
            free(renamed);
            char* res = _strdup(tmpPath);
            if (!res) DeleteFileA(tmpPath);
            if (out_fd) {
                int fd = _open(res, _O_BINARY | _O_RDWR);
                if (fd == -1) {
                    DeleteFileA(res);
                    free(res);
                    return NULL;
                }
                *out_fd = fd;
            }
            return res;
        }
        if (out_fd) {
            int fd = _open(renamed, _O_BINARY | _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE);
            if (fd == -1) {
                DeleteFileA(renamed);
                free(renamed);
                return NULL;
            }
            *out_fd = fd;
        }
        return renamed;
    } else {
        if (out_fd) {
            int fd = _open(tmpPath, _O_BINARY | _O_RDWR);
            if (fd == -1) {
                DeleteFileA(tmpPath);
                return NULL;
            }
            *out_fd = fd;
        }
        return _strdup(tmpPath);
    }
#else
    char* tpl = join_path(tdir, "copy-XXXXXX");
    if (!tpl) return NULL;
    int fd = mkstemp(tpl);
    if (fd == -1) {
        free(tpl);
        return NULL;
    }
    if (out_fd) *out_fd = fd;
    // Optionally add suffix by renaming, but not necessary.
    (void)prefix; (void)suffix;
    return tpl;
#endif
}

char* copy_to_temp(const char* source_path) {
    if (source_path == NULL || *source_path == '\0') {
        fprintf(stderr, "[C] Error: source_path is invalid.\n");
        return NULL;
    }
    // Check existence and not a directory
    if (is_directory(source_path)) {
        fprintf(stderr, "[C] Error: Source path is a directory: %s\n", source_path);
        return NULL;
    }
    FILE* in = fopen(source_path, "rb");
    if (!in) {
        fprintf(stderr, "[C] Error: Cannot open source file '%s': %s\n", source_path, strerror(errno));
        return NULL;
    }

    int out_fd = -1;
    char* dest_path = make_unique_temp_file("copy-", ".tmp", &out_fd);
    if (!dest_path) {
        fprintf(stderr, "[C] Error: Cannot create temp file: %s\n", strerror(errno));
        fclose(in);
        return NULL;
    }

#if defined(_WIN32)
    FILE* out = _fdopen(out_fd, "wb");
#else
    FILE* out = fdopen(out_fd, "wb");
#endif
    if (!out) {
        fprintf(stderr, "[C] Error: fdopen failed: %s\n", strerror(errno));
        fclose(in);
#if defined(_WIN32)
        _close(out_fd);
        DeleteFileA(dest_path);
#else
        close(out_fd);
        remove(dest_path);
#endif
        free(dest_path);
        return NULL;
    }

    char buf[8192];
    size_t n;
    int error = 0;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            fprintf(stderr, "[C] Error: Write failed: %s\n", strerror(errno));
            error = 1;
            break;
        }
    }
    if (ferror(in)) {
        fprintf(stderr, "[C] Error: Read failed: %s\n", strerror(errno));
        error = 1;
    }

    fclose(in);
    if (fclose(out) != 0) {
        fprintf(stderr, "[C] Error: Closing output failed: %s\n", strerror(errno));
        error = 1;
    }

    if (error) {
#if defined(_WIN32)
        DeleteFileA(dest_path);
#else
        remove(dest_path);
#endif
        free(dest_path);
        return NULL;
    }

    return dest_path; // caller must free()
}

char* create_sample_file(const char* content) {
    const char* tdir = get_temp_dir();
    char* path = join_path(tdir, "task127-sample-XXXXXX");
    if (!path) return NULL;
#if defined(_WIN32)
    // Simulate mkstemp: create unique name using GetTempFileName
    char tmpName[MAX_PATH];
    if (!GetTempFileNameA(tdir, "t127", 0, tmpName)) {
        free(path);
        return NULL;
    }
    free(path);
    path = _strdup(tmpName);
    if (!path) return NULL;
    FILE* f = fopen(path, "wb");
#else
    int fd = mkstemp(path);
    if (fd == -1) {
        free(path);
        return NULL;
    }
    FILE* f = fdopen(fd, "wb");
#endif
    if (!f) {
        fprintf(stderr, "[C] Error: Cannot open sample file for writing: %s\n", strerror(errno));
#if !defined(_WIN32)
        remove(path);
#endif
        free(path);
        return NULL;
    }
    size_t len = content ? strlen(content) : 0;
    if (len > 0 && fwrite(content, 1, len, f) != len) {
        fprintf(stderr, "[C] Error: Writing sample file failed: %s\n", strerror(errno));
#if defined(_WIN32)
        fclose(f);
        DeleteFileA(path);
#else
        fclose(f);
        remove(path);
#endif
        free(path);
        return NULL;
    }
    fclose(f);
    return path; // caller must free()
}

int files_equal(const char* a, const char* b) {
    FILE* fa = fopen(a, "rb");
    FILE* fb = fopen(b, "rb");
    if (!fa || !fb) {
        if (fa) fclose(fa);
        if (fb) fclose(fb);
        return 0;
    }
    char ba[8192], bb[8192];
    size_t ra, rb;
    int equal = 1;
    do {
        ra = fread(ba, 1, sizeof(ba), fa);
        rb = fread(bb, 1, sizeof(bb), fb);
        if (ra != rb || memcmp(ba, bb, ra) != 0) {
            equal = 0;
            break;
        }
    } while (ra > 0);
    fclose(fa);
    fclose(fb);
    return equal;
}

int main(void) {
    // Test 1: Valid small file
    char* src1 = create_sample_file("Hello, Task127!");
    char* dst1 = copy_to_temp(src1);
    printf("[C][Test1] src=%s dst=%s equal=%s\n", src1 ? src1 : "(null)", dst1 ? dst1 : "(null)", (src1 && dst1 && files_equal(src1, dst1)) ? "true" : "false");

    // Test 2: Non-existent file
    const char* tdir = get_temp_dir();
    char* nonexist = join_path(tdir, "nonexistent_12345_abcdef.txt");
    char* dst2 = copy_to_temp(nonexist);
    printf("[C][Test2] src=%s dst=%s\n", nonexist, dst2 ? dst2 : "(null)");

    // Test 3: Directory instead of file
    char* dst3 = copy_to_temp(tdir);
    printf("[C][Test3] src=%s dst=%s\n", tdir, dst3 ? dst3 : "(null)");

    // Test 4: Empty string path
    char* dst4 = copy_to_temp("");
    printf("[C][Test4] src='' dst=%s\n", dst4 ? dst4 : "(null)");

    // Test 5: Larger content file
    char* bigbuf = (char*)malloc(200000);
    if (bigbuf) {
        bigbuf[0] = '\0';
        for (int i = 0; i < 10000; ++i) {
            char line[32];
            snprintf(line, sizeof(line), "Line %d - Task127\n", i);
            strcat(bigbuf, line);
        }
    }
    char* src5 = create_sample_file(bigbuf ? bigbuf : "fallback");
    char* dst5 = copy_to_temp(src5);
    printf("[C][Test5] src=%s dst=%s equal=%s\n", src5 ? src5 : "(null)", dst5 ? dst5 : "(null)", (src5 && dst5 && files_equal(src5, dst5)) ? "true" : "false");

    // Cleanup allocated strings (files are left for inspection)
    free(src1);
    free(dst1);
    free(nonexist);
    free(dst2);
    free(dst3);
    free(dst4);
    free(bigbuf);
    free(src5);
    free(dst5);
    return 0;
}
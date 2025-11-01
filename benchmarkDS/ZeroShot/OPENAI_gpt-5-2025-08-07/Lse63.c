#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define PATH_SEP '\\'
#else
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#define PATH_SEP '/'
#include <sys/stat.h>
#include <fcntl.h>
#endif

#define MAX_SIZE (50ULL * 1024ULL * 1024ULL) // 50 MB

static int is_symlink_path(const char* path) {
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    if (attrs == INVALID_FILE_ATTRIBUTES) return 0;
    return (attrs & FILE_ATTRIBUTE_REPARSE_POINT) ? 1 : 0;
#else
    struct stat st;
    if (lstat(path, &st) != 0) {
        return 0; // if cannot lstat, treat as not symlink (will fail later if needed)
    }
    return S_ISLNK(st.st_mode) ? 1 : 0;
#endif
}

static int is_regular_file(const char* path) {
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    if (attrs == INVALID_FILE_ATTRIBUTES) return 0;
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) return 0;
    return 1;
#else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode) ? 1 : 0;
#endif
}

static int mkdirs_recursive(const char* path) {
    if (!path || !*path) return 0;
    char tmp[4096];
    size_t len = strlen(path);
    if (len >= sizeof(tmp)) return -1;
    memcpy(tmp, path, len + 1);
    // Normalize separators
    for (size_t i = 0; i < len; ++i) {
#ifdef _WIN32
        if (tmp[i] == '/') tmp[i] = '\\';
#else
        if (tmp[i] == '\\') tmp[i] = '/';
#endif
    }
    // Skip drive or root specifics
    size_t start = 0;
#ifdef _WIN32
    if (len >= 2 && tmp[1] == ':') start = 2;
#endif
    for (size_t i = start + 1; i < len; ++i) {
        if (tmp[i] == PATH_SEP) {
            char c = tmp[i];
            tmp[i] = '\0';
            if (strlen(tmp) > 0) {
#ifdef _WIN32
                _mkdir(tmp);
#else
                mkdir(tmp, 0700);
#endif
            }
            tmp[i] = c;
        }
    }
#ifdef _WIN32
    if (_mkdir(tmp) != 0 && errno != EEXIST) { /* ignore if exists */ }
#else
    if (mkdir(tmp, 0700) != 0 && errno != EEXIST) { /* ignore if exists */ }
#endif
    return 0;
}

int process_file(const char* input_path, const char* output_path, uint64_t* bytes_read, uint32_t* checksum, int* is_empty) {
    if (!input_path || !output_path || !bytes_read || !checksum || !is_empty) {
        return -1;
    }

    // Validate input
    FILE* in = NULL;
    *bytes_read = 0;
    *checksum = 0;
    *is_empty = 0;

#ifdef _WIN32
    // Windows fopen doesn't follow symlink checks; we check attributes
    if (is_symlink_path(input_path)) {
        return -2;
    }
#else
    if (is_symlink_path(input_path)) {
        return -2;
    }
#endif
    if (!is_regular_file(input_path)) {
        return -3;
    }

    in = fopen(input_path, "rb");
    if (!in) {
        return -4;
    }

    // Determine size
#ifdef _WIN32
    _fseeki64(in, 0, SEEK_END);
    long long sz = _ftelli64(in);
    _fseeki64(in, 0, SEEK_SET);
#else
    if (fseeko(in, 0, SEEK_END) != 0) {
        fclose(in);
        return -5;
    }
    off_t sz = ftello(in);
    if (sz < 0) {
        fclose(in);
        return -6;
    }
    if (fseeko(in, 0, SEEK_SET) != 0) {
        fclose(in);
        return -7;
    }
#endif

    if ((uint64_t)sz > MAX_SIZE) {
        fclose(in);
        return -8;
    }

    uint8_t* buf = NULL;
    if (sz > 0) {
        buf = (uint8_t*)malloc((size_t)sz);
        if (!buf) {
            fclose(in);
            return -9;
        }
        size_t readn = fread(buf, 1, (size_t)sz, in);
        if (readn != (size_t)sz) {
            free(buf);
            fclose(in);
            return -10;
        }
    }
    fclose(in);

    uint32_t sum = 0;
    for (int64_t i = 0; i < sz; ++i) {
        sum = (uint32_t)(sum + (uint32_t)buf[i]);
    }
    if (buf) free(buf);

    *bytes_read = (uint64_t)sz;
    *checksum = sum;
    *is_empty = (sum == 0) ? 1 : 0;

    // Prepare output directory
    char out_copy[4096];
    if (strlen(output_path) >= sizeof(out_copy)) {
        return -11;
    }
    strcpy(out_copy, output_path);
    // Extract parent directory
    char dirbuf[4096];
    strcpy(dirbuf, out_copy);
#ifdef _WIN32
    // Find last separator
    char* last_sep = strrchr(dirbuf, '\\');
    char* last_sep2 = strrchr(dirbuf, '/');
    if (last_sep2 && (!last_sep || last_sep2 > last_sep)) last_sep = last_sep2;
    if (last_sep) {
        *last_sep = '\0';
        if (strlen(dirbuf) > 0) mkdirs_recursive(dirbuf);
    }
#else
    char* dname = dirname(dirbuf);
    if (dname && strlen(dname) > 0 && strcmp(dname, ".") != 0) {
        mkdirs_recursive(dname);
    }
#endif

    // Prevent writing to symlink
    if (is_symlink_path(output_path)) {
        return -12;
    }

    FILE* out = fopen(output_path, "wb");
    if (!out) {
        return -13;
    }

    if (fprintf(out, "bytes=%" PRIu64 "\n", *bytes_read) < 0) {
        fclose(out);
        return -14;
    }
    if (fprintf(out, "checksum=%" PRIu32 "\n", *checksum) < 0) {
        fclose(out);
        return -15;
    }
    fflush(out);
    fclose(out);

    return 0;
}

static int write_bytes(const char* path, const uint8_t* data, size_t len) {
    // Ensure directory exists
    char tmp[4096];
    if (strlen(path) >= sizeof(tmp)) return -1;
    strcpy(tmp, path);
#ifdef _WIN32
    char* last_sep = strrchr(tmp, '\\');
    char* last_sep2 = strrchr(tmp, '/');
    if (last_sep2 && (!last_sep || last_sep2 > last_sep)) last_sep = last_sep2;
    if (last_sep) {
        *last_sep = '\0';
        if (strlen(tmp) > 0) mkdirs_recursive(tmp);
    }
#else
    char* dir = dirname(tmp);
    if (dir && strcmp(dir, ".") != 0) {
        mkdirs_recursive(dir);
    }
#endif
    FILE* f = fopen(path, "wb");
    if (!f) return -2;
    if (len > 0 && fwrite(data, 1, len, f) != len) {
        fclose(f);
        return -3;
    }
    fclose(f);
    return 0;
}

int main(void) {
    char tempdir[512];
#ifdef _WIN32
    char tmpPath[MAX_PATH];
    DWORD r = GetTempPathA(MAX_PATH, tmpPath);
    if (r == 0 || r > MAX_PATH) {
        fprintf(stderr, "Failed to get temp path\n");
        return 1;
    }
    snprintf(tempdir, sizeof(tempdir), "%s%s", tmpPath, "lse63_c");
    _mkdir(tempdir);
#else
    snprintf(tempdir, sizeof(tempdir), "%s", "/tmp/lse63_c");
    mkdir(tempdir, 0700);
#endif

    // Test 1: Empty file
    char f1[1024], o1[1024];
    snprintf(f1, sizeof(f1), "%s%cempty.bin", tempdir, PATH_SEP);
    snprintf(o1, sizeof(o1), "%s%cout1.txt", tempdir, PATH_SEP);
    write_bytes(f1, (const uint8_t*)"", 0);
    uint64_t br; uint32_t cs; int ie;
    if (process_file(f1, o1, &br, &cs, &ie) == 0) {
        printf("Test1: bytes=%" PRIu64 ", checksum=%" PRIu32 ", isEmpty=%s\n", br, cs, ie ? "true" : "false");
    } else {
        printf("Test1: failed\n");
    }

    // Test 2: Small text
    char f2[1024], o2[1024];
    snprintf(f2, sizeof(f2), "%s%chello.txt", tempdir, PATH_SEP);
    snprintf(o2, sizeof(o2), "%s%cout2.txt", tempdir, PATH_SEP);
    const char* hello = "Hello, world!";
    write_bytes(f2, (const uint8_t*)hello, strlen(hello));
    if (process_file(f2, o2, &br, &cs, &ie) == 0) {
        printf("Test2: bytes=%" PRIu64 ", checksum=%" PRIu32 ", isEmpty=%s\n", br, cs, ie ? "true" : "false");
    } else {
        printf("Test2: failed\n");
    }

    // Test 3: 2048 random bytes
    char f3[1024], o3[1024];
    snprintf(f3, sizeof(f3), "%s%crandom.bin", tempdir, PATH_SEP);
    snprintf(o3, sizeof(o3), "%s%cout3.txt", tempdir, PATH_SEP);
    uint8_t* rnd = (uint8_t*)malloc(2048);
    if (!rnd) return 1;
    srand((unsigned)time(NULL));
    for (int i = 0; i < 2048; ++i) rnd[i] = (uint8_t)(rand() & 0xFF);
    write_bytes(f3, rnd, 2048);
    free(rnd);
    if (process_file(f3, o3, &br, &cs, &ie) == 0) {
        printf("Test3: bytes=%" PRIu64 ", checksum=%" PRIu32 ", isEmpty=%s\n", br, cs, ie ? "true" : "false");
    } else {
        printf("Test3: failed\n");
    }

    // Test 4: 10000 bytes of 0xFF
    char f4[1024], o4[1024];
    snprintf(f4, sizeof(f4), "%s%cff.bin", tempdir, PATH_SEP);
    snprintf(o4, sizeof(o4), "%s%cout4.txt", tempdir, PATH_SEP);
    size_t len4 = 10000;
    uint8_t* ff = (uint8_t*)malloc(len4);
    if (!ff) return 1;
    memset(ff, 0xFF, len4);
    write_bytes(f4, ff, len4);
    free(ff);
    if (process_file(f4, o4, &br, &cs, &ie) == 0) {
        printf("Test4: bytes=%" PRIu64 ", checksum=%" PRIu32 ", isEmpty=%s\n", br, cs, ie ? "true" : "false");
    } else {
        printf("Test4: failed\n");
    }

    // Test 5: UTF-8 unicode text repeated
    char f5[1024], o5[1024];
    snprintf(f5, sizeof(f5), "%s%cutf8.txt", tempdir, PATH_SEP);
    snprintf(o5, sizeof(o5), "%s%cout5.txt", tempdir, PATH_SEP);
    const char* uni = "こんにちは世界";
    size_t rep = 10;
    size_t uni_len = strlen(uni);
    uint8_t* block = (uint8_t*)malloc(uni_len * rep);
    if (!block) return 1;
    uint8_t* cur = block;
    for (size_t i = 0; i < rep; ++i) {
        memcpy(cur, uni, uni_len);
        cur += uni_len;
    }
    write_bytes(f5, block, uni_len * rep);
    free(block);
    if (process_file(f5, o5, &br, &cs, &ie) == 0) {
        printf("Test5: bytes=%" PRIu64 ", checksum=%" PRIu32 ", isEmpty=%s\n", br, cs, ie ? "true" : "false");
    } else {
        printf("Test5: failed\n");
    }

    return 0;
}
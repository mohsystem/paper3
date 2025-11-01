#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int make_dirs(const char* path) {
    if (!path || !*path) return 1;
    char tmp[4096];
    memset(tmp, 0, sizeof(tmp));
    size_t len = strlen(path);
    if (len >= sizeof(tmp)) return 0;
    strcpy(tmp, path);
    if (tmp[0] == '/' && tmp[1] == '\0') return 1;

    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            struct stat st;
            if (stat(tmp, &st) != 0) {
                if (mkdir(tmp, 0755) != 0) return 0;
            } else if (!S_ISDIR(st.st_mode)) {
                return 0;
            }
            *p = '/';
        }
    }
    struct stat st;
    if (stat(tmp, &st) != 0) {
        if (mkdir(tmp, 0755) != 0) return 0;
    } else if (!S_ISDIR(st.st_mode)) {
        return 0;
    }
    return 1;
}

static int path_is_safe(const char* name) {
    if (!name || !*name) return 0;
    if (name[0] == '/') return 0;
    if (strstr(name, "..") != NULL) return 0;
    return 1;
}

static long long parse_octal(const unsigned char* b, int off, int len) {
    long long val = 0;
    int end = off + len;
    int i = off;
    while (i < end && (b[i] == ' ' || b[i] == '\0')) i++;
    for (; i < end; i++) {
        unsigned char c = b[i];
        if (c == '\0' || c == ' ') break;
        if (c < '0' || c > '7') break;
        val = (val << 3) + (c - '0');
    }
    return val;
}

static int is_all_zero(const unsigned char* b, size_t len) {
    for (size_t i = 0; i < len; i++) if (b[i] != 0) return 0;
    return 1;
}

static void join_path(char* out, size_t outsz, const char* a, const char* b) {
    snprintf(out, outsz, "%s/%s", a, b);
}

static void ensure_parent_dirs(const char* path) {
    char tmp[4096];
    strncpy(tmp, path, sizeof(tmp)-1);
    tmp[sizeof(tmp)-1] = 0;
    char* p = strrchr(tmp, '/');
    if (p) {
        *p = 0;
        make_dirs(tmp);
    }
}

int Lse50_extract_tar_from_zip_named(const char* archivePath, const char* destDir) {
    FILE* f = fopen(archivePath, "rb");
    if (!f) return 0;
    if (!make_dirs(destDir)) { fclose(f); return 0; }

    const size_t BLOCK = 512;
    unsigned char header[512];
    unsigned char buffer[8192];

    while (1) {
        size_t n = fread(header, 1, BLOCK, f);
        if (n == 0) break;
        if (n != BLOCK) { fclose(f); return 0; }

        if (is_all_zero(header, BLOCK)) {
            // read next block and stop
            fread(header, 1, BLOCK, f);
            break;
        }

        char name[256]; memset(name, 0, sizeof(name));
        memcpy(name, header, 100);
        // trim
        for (int i = 99; i >= 0; --i) {
            if (name[i] == '\0' || name[i] == ' ') name[i] = '\0'; else break;
        }
        char prefix[256]; memset(prefix, 0, sizeof(prefix));
        memcpy(prefix, header + 345, 155);
        for (int i = 154; i >= 0; --i) {
            if (prefix[i] == '\0' || prefix[i] == ' ') prefix[i] = '\0'; else break;
        }
        char fullname[512]; memset(fullname, 0, sizeof(fullname));
        if (prefix[0]) snprintf(fullname, sizeof(fullname), "%s/%s", prefix, name);
        else snprintf(fullname, sizeof(fullname), "%s", name);

        long long size = parse_octal(header, 124, 12);
        char type = header[156];

        if (fullname[0] == '\0') {
            // skip
            long long skip = size;
            long long pad = (512 - (size % 512)) % 512;
            skip += pad;
            while (skip > 0) {
                size_t r = (size_t) (skip > (long long)sizeof(buffer) ? sizeof(buffer) : skip);
                size_t got = fread(buffer, 1, r, f);
                if (got == 0) { fclose(f); return 0; }
                skip -= got;
            }
            continue;
        }

        if (!path_is_safe(fullname)) { fclose(f); return 0; }

        char outPath[4096];
        join_path(outPath, sizeof(outPath), destDir, fullname);

        if (type == '5') {
            if (!make_dirs(outPath)) { fclose(f); return 0; }
        } else if (type == '0' || type == '\0') {
            ensure_parent_dirs(outPath);
            FILE* out = fopen(outPath, "wb");
            if (!out) { fclose(f); return 0; }
            long long remaining = size;
            while (remaining > 0) {
                size_t toRead = (size_t)(remaining > (long long)sizeof(buffer) ? sizeof(buffer) : remaining);
                size_t got = fread(buffer, 1, toRead, f);
                if (got == 0) { fclose(out); fclose(f); return 0; }
                fwrite(buffer, 1, got, out);
                remaining -= got;
            }
            fclose(out);
            long long pad = (512 - (size % 512)) % 512;
            if (pad > 0) {
                if (fseek(f, pad, SEEK_CUR) != 0) {
                    // fallback to read
                    long long tmp = pad;
                    while (tmp > 0) {
                        size_t toRead = (size_t)(tmp > (long long)sizeof(buffer) ? sizeof(buffer) : tmp);
                        size_t got = fread(buffer, 1, toRead, f);
                        if (got == 0) { fclose(f); return 0; }
                        tmp -= got;
                    }
                }
            }
        } else {
            long long skip = size;
            long long pad = (512 - (size % 512)) % 512;
            skip += pad;
            while (skip > 0) {
                size_t r = (size_t) (skip > (long long)sizeof(buffer) ? sizeof(buffer) : skip);
                size_t got = fread(buffer, 1, r, f);
                if (got == 0) { fclose(f); return 0; }
                skip -= got;
            }
        }
    }

    fclose(f);
    return 1;
}

int main() {
    const char* tests[5][2] = {
        {"archive.zip", "/tmp/unpack"},
        {"archive.zip", "/tmp/unpack1"},
        {"archive.tar", "/tmp/unpack2"},
        {"nonexistent.zip", "/tmp/unpack3"},
        {"./archive.zip", "/tmp/unpack4"}
    };
    for (int i = 0; i < 5; ++i) {
        int ok = Lse50_extract_tar_from_zip_named(tests[i][0], tests[i][1]);
        printf("Test %d (%s -> %s): %s\n", i + 1, tests[i][0], tests[i][1], ok ? "OK" : "FAIL");
    }
    return 0;
}
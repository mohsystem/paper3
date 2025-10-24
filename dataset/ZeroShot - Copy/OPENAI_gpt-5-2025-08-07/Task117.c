#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir_p(path) _mkdir(path)
#define PATH_SEP '\\'
#else
#include <unistd.h>
#define mkdir_p(path) mkdir(path, 0755)
#define PATH_SEP '/'
#endif

static int is_zero_block(const unsigned char* buf, size_t n) {
    for (size_t i = 0; i < n; ++i) if (buf[i] != 0) return 0;
    return 1;
}

static unsigned long long parse_octal(const unsigned char* p, size_t n) {
    unsigned long long v = 0;
    size_t i = 0;
    while (i < n && (p[i] == ' ' || p[i] == '\0')) i++;
    for (; i < n; ++i) {
        unsigned char c = p[i];
        if (c == '\0' || c == ' ') break;
        if (c < '0' || c > '7') break;
        v = (v << 3) + (unsigned long long)(c - '0');
    }
    return v;
}

static unsigned long long roundup512(unsigned long long x) {
    unsigned long long r = x % 512ULL;
    return r == 0 ? x : x + (512ULL - r);
}

static int ensure_dirs(const char* path) {
    // create parent directories
    char* tmp = strdup(path);
    if (!tmp) return -1;
    for (char* p = tmp + 1; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            char c = *p;
            *p = '\0';
            if (strlen(tmp) > 0) {
                #ifdef _WIN32
                _mkdir(tmp);
                #else
                mkdir(tmp, 0755);
                #endif
            }
            *p = c;
        }
    }
    free(tmp);
    return 0;
}

static int has_traversal(const char* name) {
    if (!name || !*name) return 1;
    if (name[0] == '/' || name[0] == '\\') return 1;
    // reject backslashes and ".." segments
    if (strstr(name, "\\") != NULL) return 1;
    const char* p = name;
    while (*p) {
        // find next segment
        const char* q = strchr(p, '/');
        size_t len = q ? (size_t)(q - p) : strlen(p);
        if (len == 2 && p[0] == '.' && p[1] == '.') return 1;
        if (!q) break;
        p = q + 1;
    }
    return 0;
}

// Extract uncompressed TAR; returns number of extracted files, or -1 on error
int extract_tar(const char* archivePath, const char* destDir) {
    FILE* fp = fopen(archivePath, "rb");
    if (!fp) {
        return -1;
    }
    // Create base directory if needed
    #ifdef _WIN32
    _mkdir(destDir);
    #else
    mkdir(destDir, 0755);
    #endif

    unsigned char header[512];
    int extracted = 0;
    while (1) {
        size_t r = fread(header, 1, 512, fp);
        if (r == 0) break;
        if (r < 512) { extracted = -1; break; }
        if (is_zero_block(header, 512)) {
            // consume next zero block if present
            fread(header, 1, 512, fp);
            break;
        }
        char name[256] = {0};
        memcpy(name, header + 0, 100);
        name[100] = '\0';
        // prefix
        char prefix[256] = {0};
        memcpy(prefix, header + 345, 155);
        prefix[155] = '\0';
        char full[512] = {0};
        if (prefix[0]) {
            snprintf(full, sizeof(full), "%s/%s", prefix, name);
        } else {
            snprintf(full, sizeof(full), "%s", name);
        }
        // normalize slashes to '/'
        for (size_t i = 0; full[i]; ++i) {
            if (full[i] == '\\') full[i] = '/';
        }

        char typeflag = (char)header[156];
        unsigned long long size = parse_octal(header + 124, 12);

        if (!has_traversal(full)) {
            // build output path: destDir + "/" + full
            char outpath[1024];
            snprintf(outpath, sizeof(outpath), "%s%c%s", destDir, PATH_SEP, full);

            if (typeflag == '5' || (strlen(full) > 0 && full[strlen(full)-1] == '/')) {
                ensure_dirs(outpath);
                #ifdef _WIN32
                _mkdir(outpath);
                #else
                mkdir(outpath, 0755);
                #endif
            } else if (typeflag == '0' || typeflag == '\0') {
                ensure_dirs(outpath);
                FILE* out = fopen(outpath, "wb");
                if (!out) { fclose(fp); return -1; }
                unsigned long long remaining = size;
                char buf[8192];
                while (remaining > 0) {
                    size_t toread = (size_t)((remaining > sizeof(buf)) ? sizeof(buf) : remaining);
                    size_t got = fread(buf, 1, toread, fp);
                    if (got == 0) { fclose(out); fclose(fp); return -1; }
                    size_t wrote = fwrite(buf, 1, got, out);
                    if (wrote != got) { fclose(out); fclose(fp); return -1; }
                    remaining -= got;
                }
                fclose(out);
                // skip padding
                unsigned long long pad = roundup512(size) - size;
                if (pad > 0) fseek(fp, (long)pad, SEEK_CUR);
                extracted++;
                continue;
            } else {
                // skip other types
            }
        }
        // skip the data for skipped or non-regular entries
        unsigned long long skip = roundup512(size);
        if (skip > 0) fseek(fp, (long)skip, SEEK_CUR);
    }
    fclose(fp);
    return extracted;
}

int main(void) {
    // 5 test cases (demonstration)
    int res;
    res = extract_tar("nonexistent.tar", "c_out1");
    printf("Test1 result: %d\n", res);

    res = extract_tar("./sample.tar", "c_out2");
    printf("Test2 result: %d\n", res);

    res = extract_tar("./safe.tar", "c_out3");
    printf("Test3 result: %d\n", res);

    res = extract_tar("./traversal.tar", "c_out4");
    printf("Test4 result: %d\n", res);

    res = extract_tar("./another.tar", "c_out5");
    printf("Test5 result: %d\n", res);

    return 0;
}
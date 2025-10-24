#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <errno.h>

// Minimal C TAR extractor (no external libraries). Extracts .tar files only, with basic security checks.

#ifdef _WIN32
#include <direct.h>
#define MKDIR(dir, mode) _mkdir(dir)
#define PATH_SEP '\\'
#else
#include <unistd.h>
#define MKDIR(dir, mode) mkdir(dir, mode)
#define PATH_SEP '/'
#endif

static int is_all_zero(const unsigned char *buf, size_t len) {
    for (size_t i = 0; i < len; i++) if (buf[i] != 0) return 0;
    return 1;
}

static uint64_t read_octal(const unsigned char *hdr, int off, int len) {
    uint64_t val = 0;
    int end = off + len;
    int i = off;
    while (i < end && (hdr[i] == 0 || hdr[i] == ' ')) i++;
    for (; i < end; i++) {
        unsigned char c = hdr[i];
        if (c == 0 || c == ' ') break;
        if (c < '0' || c > '7') break;
        val = (val << 3) + (c - '0');
    }
    return val;
}

static void sanitize_join(char *out, size_t outsz, const char *base, const char *name) {
    // Join base + '/' + name
    snprintf(out, outsz, "%s%c%s", base, PATH_SEP, name);
    // Normalize by removing '/../' and '/./'
    // Very basic normalization
    char *p;
    while ((p = strstr(out, "/./")) != NULL) {
        memmove(p, p + 2, strlen(p + 2) + 1);
    }
    while ((p = strstr(out, "//")) != NULL) {
        memmove(p, p + 1, strlen(p + 1) + 1);
    }
    // remove /dir/../
    while ((p = strstr(out, "/../")) != NULL) {
        // find previous '/'
        char *q = p - 1;
        while (q > out && *q != '/') q--;
        if (q <= out) {
            // cannot normalize; break
            break;
        }
        memmove(q, p + 3, strlen(p + 3) + 1);
    }
}

static int starts_with(const char *s, const char *prefix) {
    size_t ls = strlen(s), lp = strlen(prefix);
    if (ls < lp) return 0;
    return strncmp(s, prefix, lp) == 0;
}

static uint64_t pad512(uint64_t size) {
    uint64_t rem = size % 512;
    return rem == 0 ? 0 : (512 - rem);
}

static int ensure_dirs(const char *path) {
    char tmp[4096];
    snprintf(tmp, sizeof(tmp), "%s", path);
    // create each component
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char c = *p;
            *p = 0;
            MKDIR(tmp, 0755);
            *p = c;
        }
    }
    // final directory
    return MKDIR(tmp, 0755) == 0 || errno == EEXIST;
}

// Extract .tar file securely with size limit; returns number of files extracted or -1 on error
int extract_tar_secure(const char *tar_path, const char *dest_dir, uint64_t max_total_bytes) {
    FILE *f = fopen(tar_path, "rb");
    if (!f) return -1;

    unsigned char hdr[512];
    int files_extracted = 0;
    uint64_t total = 0;

    // ensure base dest dir exists
    ensure_dirs(dest_dir);

    while (1) {
        size_t r = fread(hdr, 1, 512, f);
        if (r == 0) break;
        if (r < 512) { fclose(f); return -1; }
        if (is_all_zero(hdr, 512)) {
            // read optional second zero block
            fread(hdr, 1, 512, f);
            break;
        }
        char name[101]; memset(name, 0, sizeof(name));
        memcpy(name, hdr, 100);
        char prefix[156]; memset(prefix, 0, sizeof(prefix));
        memcpy(prefix, hdr + 345, 155);
        char typeflag = (char)hdr[156];
        uint64_t size = read_octal(hdr, 124, 12);

        char full[300] = {0};
        if (prefix[0]) {
            snprintf(full, sizeof(full), "%s/%s", prefix, name);
        } else {
            snprintf(full, sizeof(full), "%s", name);
        }

        if (full[0] == '/' || full[0] == '\\' || strstr(full, "\x00")) {
            // skip dangerous
            uint64_t skip = size + pad512(size);
            if (fseek(f, (long)skip, SEEK_CUR) != 0) { fclose(f); return -1; }
            continue;
        }

        // Prevent traversal: ensure target starts with dest_dir
        char target[4096];
        sanitize_join(target, sizeof(target), dest_dir, full);
        char dest_real[4096]; snprintf(dest_real, sizeof(dest_real), "%s", dest_dir);
        // ensure trailing separator
        size_t dl = strlen(dest_real);
        if (dest_real[dl-1] != '/' && dest_real[dl-1] != '\\') {
#ifdef _WIN32
            dest_real[dl] = '\\'; dest_real[dl+1] = 0;
#else
            dest_real[dl] = '/'; dest_real[dl+1] = 0;
#endif
        }
        if (!starts_with(target, dest_real)) {
            uint64_t skip = size + pad512(size);
            if (fseek(f, (long)skip, SEEK_CUR) != 0) { fclose(f); return -1; }
            continue;
        }

        if (typeflag == '5') {
            // directory
            ensure_dirs(target);
            // skip padding if any
            uint64_t pad = pad512(size);
            if (pad) {
                if (fseek(f, (long)pad, SEEK_CUR) != 0) { fclose(f); return -1; }
            }
        } else if (typeflag == '0' || typeflag == 0) {
            // regular file
            // ensure parent dirs
            char parent[4096]; snprintf(parent, sizeof(parent), "%s", target);
            for (int i = (int)strlen(parent) - 1; i >= 0; --i) {
                if (parent[i] == '/' || parent[i] == '\\') { parent[i] = 0; break; }
            }
            ensure_dirs(parent);

            FILE *out = fopen(target, "wb");
            if (!out) { fclose(f); return -1; }
            uint64_t remaining = size;
            char buf[8192];
            while (remaining > 0) {
                size_t chunk = (size_t)((remaining > sizeof(buf)) ? sizeof(buf) : remaining);
                size_t rr = fread(buf, 1, chunk, f);
                if (rr == 0) { fclose(out); fclose(f); return -1; }
                if (total + rr > max_total_bytes) {
                    uint64_t allowed = (max_total_bytes > total) ? (max_total_bytes - total) : 0;
                    if (allowed > 0) fwrite(buf, 1, (size_t)allowed, out);
                    fclose(out);
                    fclose(f);
                    // Remove partial file for safety
#ifdef _WIN32
                    remove(target);
#else
                    remove(target);
#endif
                    return -1;
                }
                fwrite(buf, 1, rr, out);
                total += rr;
                remaining -= rr;
            }
            fclose(out);
            // skip padding
            uint64_t pad = pad512(size);
            if (pad) {
                if (fseek(f, (long)pad, SEEK_CUR) != 0) { fclose(f); return -1; }
            }
            files_extracted++;
        } else {
            // skip other types (symlinks etc.)
            uint64_t skip = size + pad512(size);
            if (fseek(f, (long)skip, SEEK_CUR) != 0) { fclose(f); return -1; }
        }
    }

    fclose(f);
    return files_extracted;
}

// Helpers to create simple TARs for tests
static void write_octal(unsigned char *hdr, int off, int len, uint64_t value) {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "%llo", (unsigned long long)value);
    int idx = len - 1;
    hdr[off + idx] = 0; idx--;
    if (idx >= 0) { hdr[off + idx] = ' '; idx--; }
    for (int i = (int)strlen(tmp) - 1; i >= 0 && idx >= 0; --i, --idx) {
        hdr[off + idx] = (unsigned char)tmp[i];
    }
    while (idx >= 0) { hdr[off + idx] = '0'; idx--; }
}

static void write_tar_header(FILE *f, const char *name, uint64_t size, char typeflag) {
    unsigned char hdr[512];
    memset(hdr, 0, sizeof(hdr));
    memcpy(hdr, name, (strlen(name) < 100) ? strlen(name) : 100);
    write_octal(hdr, 100, 8, 0644);
    write_octal(hdr, 108, 8, 0);
    write_octal(hdr, 116, 8, 0);
    write_octal(hdr, 124, 12, size);
    write_octal(hdr, 136, 12, (uint64_t)time(NULL));
    for (int i = 148; i < 156; ++i) hdr[i] = ' ';
    hdr[156] = (unsigned char)typeflag;
    memcpy(hdr + 257, "ustar", 5);
    hdr[262] = '0'; hdr[263] = '0';
    unsigned long sum = 0;
    for (int i = 0; i < 512; ++i) sum += hdr[i];
    write_octal(hdr, 148, 8, sum);
    fwrite(hdr, 1, 512, f);
}

static void pad_to_512(FILE *f, uint64_t size) {
    uint64_t p = pad512(size);
    if (p) {
        unsigned char *z = (unsigned char*)calloc(1, (size_t)p);
        fwrite(z, 1, (size_t)p, f);
        free(z);
    }
}

static void create_sample_tar(const char *path) {
    FILE *f = fopen(path, "wb");
    // dir/
    write_tar_header(f, "dir/", 0, '5');
    // dir/a.txt
    const char *a = "Hello from tar a.txt";
    write_tar_header(f, "dir/a.txt", strlen(a), '0');
    fwrite(a, 1, strlen(a), f);
    pad_to_512(f, strlen(a));
    // b.txt
    const char *b = "Hello from tar b.txt";
    write_tar_header(f, "b.txt", strlen(b), '0');
    fwrite(b, 1, strlen(b), f);
    pad_to_512(f, strlen(b));
    // end blocks
    unsigned char zero[512] = {0};
    fwrite(zero, 1, 512, f);
    fwrite(zero, 1, 512, f);
    fclose(f);
}

static void create_traversal_tar(const char *path) {
    FILE *f = fopen(path, "wb");
    const char *s = "safe";
    write_tar_header(f, "safe.txt", strlen(s), '0');
    fwrite(s, 1, strlen(s), f);
    pad_to_512(f, strlen(s));

    const char *e = "evil";
    write_tar_header(f, "../evil.txt", strlen(e), '0');
    fwrite(e, 1, strlen(e), f);
    pad_to_512(f, strlen(e));

    unsigned char zero[512] = {0};
    fwrite(zero, 1, 512, f);
    fwrite(zero, 1, 512, f);
    fclose(f);
}

static void create_large_tar(const char *path, size_t sizeBytes) {
    FILE *f = fopen(path, "wb");
    write_tar_header(f, "large.bin", sizeBytes, '0');
    char buf[8192]; memset(buf, 1, sizeof(buf));
    size_t remaining = sizeBytes;
    while (remaining > 0) {
        size_t chunk = remaining > sizeof(buf) ? sizeof(buf) : remaining;
        fwrite(buf, 1, chunk, f);
        remaining -= chunk;
    }
    pad_to_512(f, sizeBytes);
    unsigned char zero[512] = {0};
    fwrite(zero, 1, 512, f);
    fwrite(zero, 1, 512, f);
    fclose(f);
}

int main() {
    char base[1024];
#ifdef _WIN32
    snprintf(base, sizeof(base), ".\\task117_c_tests");
#else
    snprintf(base, sizeof(base), "./task117_c_tests");
#endif
    ensure_dirs(base);
    printf("C tests working dir: %s\n", base);

    // Test 1: simple tar
    char tar1[1024], out1[1024];
    snprintf(tar1, sizeof(tar1), "%s%ct1.tar", base, PATH_SEP);
    snprintf(out1, sizeof(out1), "%s%cout1", base, PATH_SEP);
    create_sample_tar(tar1);
    ensure_dirs(out1);
    int c1 = extract_tar_secure(tar1, out1, 10000000);
    printf("Test1 TAR extracted files: %d\n", c1);

    // Test 2: traversal tar
    char tar2[1024], out2[1024];
    snprintf(tar2, sizeof(tar2), "%s%ct2.tar", base, PATH_SEP);
    snprintf(out2, sizeof(out2), "%s%cout2", base, PATH_SEP);
    create_traversal_tar(tar2);
    ensure_dirs(out2);
    int c2 = extract_tar_secure(tar2, out2, 10000000);
    printf("Test2 TAR extracted files (expect 1): %d\n", c2);

    // Test 3: large tar with size limit
    char tar3[1024], out3[1024];
    snprintf(tar3, sizeof(tar3), "%s%ct3.tar", base, PATH_SEP);
    snprintf(out3, sizeof(out3), "%s%cout3", base, PATH_SEP);
    create_large_tar(tar3, 2000000);
    ensure_dirs(out3);
    int c3 = extract_tar_secure(tar3, out3, 100000);
    printf("Test3 TAR size limit should fail (returns -1): %d\n", c3);

    // Test 4: another simple tar
    char tar4[1024], out4[1024];
    snprintf(tar4, sizeof(tar4), "%s%ct4.tar", base, PATH_SEP);
    snprintf(out4, sizeof(out4), "%s%cout4", base, PATH_SEP);
    create_sample_tar(tar4);
    ensure_dirs(out4);
    int c4 = extract_tar_secure(tar4, out4, 10000000);
    printf("Test4 TAR extracted files: %d\n", c4);

    // Test 5: directory handling (already covered), re-run traversal to confirm skip
    char tar5[1024], out5[1024];
    snprintf(tar5, sizeof(tar5), "%s%ct5.tar", base, PATH_SEP);
    snprintf(out5, sizeof(out5), "%s%cout5", base, PATH_SEP);
    create_traversal_tar(tar5);
    ensure_dirs(out5);
    int c5 = extract_tar_secure(tar5, out5, 10000000);
    printf("Test5 TAR traversal skipped, files extracted: %d\n", c5);

    return 0;
}
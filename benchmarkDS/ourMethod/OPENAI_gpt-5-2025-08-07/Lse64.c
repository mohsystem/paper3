#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define MAX_UPLOAD_SIZE (32 * 1024 * 1024)

static void ensure_uploads(const char* base_dir) {
    struct stat st;
    if (stat(base_dir, &st) != 0) {
        if (errno == ENOENT) {
            if (mkdir(base_dir, 0700) != 0 && errno != EEXIST) {
                perror("mkdir base_dir");
                exit(1);
            }
        }
    }
    if (stat(base_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "base dir not directory\n");
        exit(1);
    }
    char uploads[512];
    snprintf(uploads, sizeof(uploads), "%s/%s", base_dir, "uploads");
    if (stat(uploads, &st) != 0) {
        if (errno == ENOENT) {
            if (mkdir(uploads, 0700) != 0 && errno != EEXIST) {
                perror("mkdir uploads");
                exit(1);
            }
        }
    }
    if (stat(uploads, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "uploads not directory\n");
        exit(1);
    }
    struct stat lst;
    if (lstat(uploads, &lst) == 0 && S_ISLNK(lst.st_mode)) {
        fprintf(stderr, "uploads must not be symlink\n");
        exit(1);
    }
}

static char* sanitize_filename(const char* name) {
    const char* base = strrchr(name, '/');
    const char* base2 = strrchr(name, '\\');
    const char* start = name;
    if (base && base2) start = (base > base2) ? base + 1 : base2 + 1;
    else if (base) start = base + 1;
    else if (base2) start = base2 + 1;

    size_t n = strlen(start);
    char* out = (char*)malloc(n + 10);
    if (!out) exit(1);
    size_t j = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)start[i];
        if (isalnum(c) || c == '.' || c == '_' || c == '-') {
            out[j++] = (char)c;
        } else {
            out[j++] = '_';
        }
    }
    out[j] = '\0';

    // ensure .pdf extension
    size_t len = strlen(out);
    int has_pdf = 0;
    if (len >= 4) {
        char* ext = out + len - 4;
        char e0 = tolower((unsigned char)ext[0]);
        char e1 = tolower((unsigned char)ext[1]);
        char e2 = tolower((unsigned char)ext[2]);
        char e3 = tolower((unsigned char)ext[3]);
        if (e0 == '.' && e1 == 'p' && e2 == 'd' && e3 == 'f') has_pdf = 1;
    }
    if (!has_pdf) {
        char* dot = strrchr(out, '.');
        if (dot) *dot = '\0';
        size_t l2 = strlen(out);
        char* tmp = (char*)realloc(out, l2 + 5);
        if (!tmp) { free(out); exit(1); }
        out = tmp;
        strcat(out, ".pdf");
    }
    // truncate to last 100 chars
    size_t ol = strlen(out);
    if (ol > 100) {
        char* tmp = (char*)malloc(101);
        if (!tmp) { free(out); exit(1); }
        memcpy(tmp, out + (ol - 100), 100);
        tmp[100] = '\0';
        free(out);
        out = tmp;
    }
    return out;
}

static void store_path_mysql_mock(const char* base_dir, const char* rel_path) {
    char idx[512];
    snprintf(idx, sizeof(idx), "%s/%s", base_dir, "mock_mysql_index.txt");
    FILE* f = fopen(idx, "ab");
    if (!f) return;
    fwrite(rel_path, 1, strlen(rel_path), f);
    fwrite("\n", 1, 1, f);
    fclose(f);
}

static char* upload_pdf(const char* base_dir, const char* original_filename, const unsigned char* content, size_t content_len) {
    if (!content || content_len == 0) {
        fprintf(stderr, "empty content\n");
        return NULL;
    }
    if (content_len > MAX_UPLOAD_SIZE) {
        fprintf(stderr, "file too large\n");
        return NULL;
    }
    ensure_uploads(base_dir);
    int basefd = open(base_dir, O_RDONLY | O_CLOEXEC);
    if (basefd < 0) { perror("open base"); return NULL; }
    int ufd = openat(basefd, "uploads", O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (ufd < 0) { perror("open uploads"); close(basefd); return NULL; }

    char* safe = sanitize_filename(original_filename);
    unsigned long long r = 0;
    int ur = open("/dev/urandom", O_RDONLY);
    if (ur >= 0) {
        if (read(ur, &r, sizeof(r)) != (ssize_t)sizeof(r)) r = (unsigned long long)rand() << 32 ^ (unsigned long long)rand();
        close(ur);
    } else {
        r = (unsigned long long)rand() << 32 ^ (unsigned long long)rand();
    }
    char prefix[17]; snprintf(prefix, sizeof(prefix), "%016llx", r);
    char final_name[256];
    snprintf(final_name, sizeof(final_name), "%.12s_%s", prefix, safe);
    char temp_name[300];
    snprintf(temp_name, sizeof(temp_name), "%s.part_%.4s", final_name, prefix + 12);

    int fd = openat(ufd, temp_name, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) {
        perror("create temp");
        free(safe); close(ufd); close(basefd);
        return NULL;
    }
    size_t off = 0;
    while (off < content_len) {
        size_t chunk = content_len - off;
        if (chunk > 65536) chunk = 65536;
        ssize_t n = write(fd, content + off, chunk);
        if (n <= 0) {
            perror("write");
            close(fd);
            unlinkat(ufd, temp_name, 0);
            free(safe); close(ufd); close(basefd);
            return NULL;
        }
        off += (size_t)n;
    }
    fsync(fd);
    close(fd);

    if (renameat(ufd, temp_name, ufd, final_name) != 0) {
        perror("rename");
        unlinkat(ufd, temp_name, 0);
        free(safe); close(ufd); close(basefd);
        return NULL;
    }
    (void)fsync(ufd);

    char* rel = (char*)malloc(9 + strlen(final_name) + 1);
    if (!rel) { free(safe); close(ufd); close(basefd); return NULL; }
    strcpy(rel, "uploads/");
    strcat(rel, final_name);

    store_path_mysql_mock(base_dir, rel);

    free(safe);
    close(ufd);
    close(basefd);
    return rel;
}

static unsigned char* download_pdf(const char* base_dir, const char* rel_path, size_t* out_len) {
    if (!rel_path || rel_path[0] == '/' || strstr(rel_path, "..") != NULL) {
        fprintf(stderr, "invalid stored path\n");
        return NULL;
    }
    if (strncmp(rel_path, "uploads/", 8) != 0) {
        fprintf(stderr, "must be within uploads/\n");
        return NULL;
    }
    int basefd = open(base_dir, O_RDONLY | O_CLOEXEC);
    if (basefd < 0) { perror("open base"); return NULL; }
    int ufd = openat(basefd, "uploads", O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (ufd < 0) { perror("open uploads"); close(basefd); return NULL; }
    const char* fname = rel_path + 8;
    int fd = openat(ufd, fname, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) { perror("open file"); close(ufd); close(basefd); return NULL; }
    struct stat st;
    if (fstat(fd, &st) != 0) { perror("fstat"); close(fd); close(ufd); close(basefd); return NULL; }
    if (!S_ISREG(st.st_mode)) { fprintf(stderr, "not regular file\n"); close(fd); close(ufd); close(basefd); return NULL; }
    if (st.st_size < 0 || st.st_size > MAX_UPLOAD_SIZE) { fprintf(stderr, "invalid size\n"); close(fd); close(ufd); close(basefd); return NULL; }
    size_t sz = (size_t)st.st_size;
    unsigned char* buf = (unsigned char*)malloc(sz ? sz : 1);
    if (!buf) { close(fd); close(ufd); close(basefd); return NULL; }
    size_t off = 0;
    while (off < sz) {
        ssize_t n = read(fd, buf + off, sz - off);
        if (n <= 0) break;
        off += (size_t)n;
    }
    close(fd);
    close(ufd);
    close(basefd);
    *out_len = off;
    return buf;
}

static unsigned char* sample_pdf_bytes(const char* title, size_t* out_len) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "%%PDF-1.4\n"
             "1 0 obj <</Type /Catalog /Pages 2 0 R>> endobj\n"
             "2 0 obj <</Type /Pages /Kids [3 0 R] /Count 1>> endobj\n"
             "3 0 obj <</Type /Page /Parent 2 0 R /MediaBox [0 0 200 200] /Contents 4 0 R>> endobj\n"
             "4 0 obj <</Length 44>> stream\n"
             "BT /F1 12 Tf 72 120 Td (%s) Tj ET\n"
             "endstream endobj\n"
             "xref\n0 5\n0000000000 65535 f \n"
             "trailer <</Root 1 0 R>>\nstartxref\n0\n%%EOF\n",
             title);
    size_t len = strlen(buf);
    unsigned char* out = (unsigned char*)malloc(len);
    if (!out) exit(1);
    memcpy(out, buf, len);
    *out_len = len;
    return out;
}

int main(void) {
    const char* base = "secure_base_c";
    const char* tests[5] = {
        "report.pdf",
        "../evil.pdf",
        "weird name.PDF",
        "semi;colon.pdf",
        "a/bad/path.pdf"
    };
    for (int i = 0; i < 5; i++) {
        char title[32];
        snprintf(title, sizeof(title), "Test %d", i + 1);
        size_t len = 0;
        unsigned char* data = sample_pdf_bytes(title, &len);
        char* stored = upload_pdf(base, tests[i], data, len);
        if (stored) {
            size_t out_len = 0;
            unsigned char* back = download_pdf(base, stored, &out_len);
            if (back) {
                printf("OK: %s -> %s size=%zu\n", tests[i], stored, out_len);
                free(back);
            } else {
                printf("FAIL: %s -> download error\n", tests[i]);
            }
            free(stored);
        } else {
            printf("FAIL: %s -> upload error\n", tests[i]);
        }
        free(data);
    }
    return 0;
}
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_IMAGE_SIZE (1 * 1024 * 1024) /* 1 MiB */
#define NAME_MAX_LEN 255

typedef struct {
    int id;
    char* name;
    char* b64;
} Record;

typedef struct {
    Record* recs;
    size_t size;
    size_t cap;
    int next_id;
} Database;

static void db_init(Database* db) {
    db->recs = NULL;
    db->size = 0;
    db->cap = 0;
    db->next_id = 1;
}

static void db_free(Database* db) {
    if (!db) return;
    for (size_t i = 0; i < db->size; i++) {
        free(db->recs[i].name);
        free(db->recs[i].b64);
    }
    free(db->recs);
    db->recs = NULL;
    db->size = db->cap = 0;
}

static bool is_safe_simple_name(const char* s) {
    if (!s) return false;
    size_t len = strlen(s);
    if (len < 1 || len > NAME_MAX_LEN) return false;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

static char* str_dup(const char* s) {
    size_t n = strlen(s);
    char* p = (char*)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

static int db_insert_image(Database* db, const char* name, const char* b64) {
    if (!is_safe_simple_name(name)) return -1;
    if (!b64 || b64[0] == '\0') return -1;
    size_t b64len = strlen(b64);
    if (b64len > MAX_IMAGE_SIZE * 2) return -1;

    if (db->size == db->cap) {
        size_t newcap = db->cap == 0 ? 8 : db->cap * 2;
        Record* nrecs = (Record*)realloc(db->recs, newcap * sizeof(Record));
        if (!nrecs) return -1;
        db->recs = nrecs;
        db->cap = newcap;
    }
    char* n_name = str_dup(name);
    char* n_b64 = str_dup(b64);
    if (!n_name || !n_b64) {
        free(n_name); free(n_b64);
        return -1;
    }
    int id = db->next_id++;
    db->recs[db->size].id = id;
    db->recs[db->size].name = n_name;
    db->recs[db->size].b64 = n_b64;
    db->size += 1;
    return id;
}

static char* base64_encode(const unsigned char* data, size_t len) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t out_len = 4 * ((len + 2) / 3);
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    size_t i = 0, j = 0;
    while (i + 3 <= len) {
        unsigned int n = ((unsigned int)data[i] << 16) | ((unsigned int)data[i+1] << 8) | data[i+2];
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = tbl[(n >> 6) & 63];
        out[j++] = tbl[n & 63];
        i += 3;
    }
    if (i + 1 == len) {
        unsigned int n = ((unsigned int)data[i] << 16);
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = '=';
        out[j++] = '=';
    } else if (i + 2 == len) {
        unsigned int n = ((unsigned int)data[i] << 16) | ((unsigned int)data[i+1] << 8);
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = tbl[(n >> 6) & 63];
        out[j++] = '=';
    }
    out[j] = '\0';
    return out;
}

// Securely read image from baseDir/relativeFileName, base64 encode, and insert into db
static bool upload_image(const char* baseDir, const char* relativeFileName, const char* imageName, Database* db) {
    if (!is_safe_simple_name(relativeFileName) || !is_safe_simple_name(imageName)) {
        printf("Rejected: invalid name\n");
        return false;
    }
    int dirfd = open(baseDir, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (dirfd < 0) {
        printf("Failed to open base dir: %s\n", strerror(errno));
        return false;
    }
    int fd = openat(dirfd, relativeFileName, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        printf("Failed to open file: %s\n", strerror(errno));
        close(dirfd);
        return false;
    }
    struct stat st;
    if (fstat(fd, &st) != 0) {
        printf("Failed to fstat: %s\n", strerror(errno));
        close(fd); close(dirfd);
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        printf("Rejected: not a regular file\n");
        close(fd); close(dirfd);
        return false;
    }
    if ((size_t)st.st_size > MAX_IMAGE_SIZE) {
        printf("Rejected: file too large\n");
        close(fd); close(dirfd);
        return false;
    }

    unsigned char* buf = (unsigned char*)malloc((size_t)st.st_size);
    if (!buf) {
        printf("Allocation failed\n");
        close(fd); close(dirfd);
        return false;
    }
    size_t total = 0;
    while (total < (size_t)st.st_size) {
        ssize_t n = read(fd, buf + total, (size_t)st.st_size - total);
        if (n < 0) {
            printf("Read error: %s\n", strerror(errno));
            free(buf);
            close(fd); close(dirfd);
            return false;
        }
        if (n == 0) break;
        total += (size_t)n;
        if (total > MAX_IMAGE_SIZE) {
            printf("Rejected: file too large (during read)\n");
            free(buf);
            close(fd); close(dirfd);
            return false;
        }
    }

    char* b64 = base64_encode(buf, total);
    free(buf);
    if (!b64) {
        printf("Base64 encode failed\n");
        close(fd); close(dirfd);
        return false;
    }

    int id = db_insert_image(db, imageName, b64);
    free(b64);
    close(fd); close(dirfd);
    if (id < 0) {
        printf("DB insert failed\n");
        return false;
    }
    return true;
}

static bool write_file(const char* baseDir, const char* name, size_t size) {
    int dirfd = open(baseDir, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (dirfd < 0) return false;
    int fd = openat(dirfd, name, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0) { close(dirfd); return false; }

    unsigned char* chunk = (unsigned char*)malloc(4096);
    if (!chunk) { close(fd); close(dirfd); return false; }
    for (size_t i = 0; i < 4096; i++) chunk[i] = (unsigned char)(rand() % 256);

    size_t remaining = size;
    while (remaining > 0) {
        size_t w = remaining < 4096 ? remaining : 4096;
        ssize_t n = write(fd, chunk, w);
        if (n < 0) { free(chunk); close(fd); close(dirfd); return false; }
        remaining -= (size_t)n;
    }
    free(chunk);
    fsync(fd);
    close(fd);
    close(dirfd);
    return true;
}

int main(void) {
    Database db;
    db_init(&db);

    char templ[] = "/tmp/img_upload_c_XXXXXX";
    char* baseDir = mkdtemp(templ);
    if (!baseDir) {
        printf("mkdtemp failed\n");
        return 1;
    }
    printf("Base directory: %s\n", baseDir);

    write_file(baseDir, "t1.bin", 1024);
    write_file(baseDir, "t2.bin", 2048);
    write_file(baseDir, "t3.bin", 512);
    write_file(baseDir, "big.bin", 2 * 1024 * 1024); /* oversize */

    bool r1 = upload_image(baseDir, "t1.bin", "image_one.png", &db);
    printf("Test1 (valid): %s\n", r1 ? "true" : "false");

    bool r2 = upload_image(baseDir, "t2.bin", "image-two.jpg", &db);
    printf("Test2 (valid): %s\n", r2 ? "true" : "false");

    bool r3 = upload_image(baseDir, "big.bin", "too_big.dat", &db);
    printf("Test3 (oversize): %s\n", r3 ? "true" : "false");

    bool r4 = upload_image(baseDir, "t_missing.bin", "missing.gif", &db);
    printf("Test4 (missing): %s\n", r4 ? "true" : "false");

    bool r5 = upload_image(baseDir, "t3.bin", "safe_name-ok_123.bmp", &db);
    printf("Test5 (valid): %s\n", r5 ? "true" : "false");

    printf("DB count: %zu\n", db.size);
    for (size_t i = 0; i < db.size; i++) {
        printf("Record %d name=%s b64_len=%zu\n",
               db.recs[i].id, db.recs[i].name, strlen(db.recs[i].b64));
    }

    db_free(&db);
    return 0;
}
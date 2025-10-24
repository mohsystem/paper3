#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <errno.h>
#include <limits.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define MAGIC "PWD1"
#define VERSION 1
#define SALT_LEN 16
#define HASH_LEN 32
#define MIN_ITER 210000

typedef struct {
    int iterations;
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
} ParsedRecord;

static int set_owner_only_permissions(const char* path) {
    if (chmod(path, S_IRUSR | S_IWUSR) != 0) {
        return 0;
    }
    return 1;
}

static int constant_time_equal(const unsigned char* a, const unsigned char* b, size_t len) {
    unsigned int diff = 0;
    for (size_t i = 0; i < len; i++) {
        diff |= (unsigned int)(a[i] ^ b[i]);
    }
    return diff == 0;
}

static int pbkdf2_sha256(const char* password, const unsigned char* salt, int salt_len, int iterations, unsigned char* out, int out_len) {
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, salt_len, iterations, EVP_sha256(), out_len, out) != 1) {
        return 0;
    }
    return 1;
}

static int generate_salt(unsigned char* salt, int len) {
    if (RAND_bytes(salt, len) != 1) return 0;
    return 1;
}

static int path_within_base(const char* baseDir, const char* relPath, char* outPath, size_t outSize) {
    if (relPath[0] == '/') return 0;
    if (strstr(relPath, "..") != NULL) return 0;
    char baseReal[PATH_MAX];
    if (realpath(baseDir, baseReal) == NULL) return 0;
    char combined[PATH_MAX];
    snprintf(combined, sizeof(combined), "%s/%s", baseReal, relPath);
    // Normalize combined without requiring target to exist: simplify by removing // and ./ segments
    // Ensure prefix matches baseReal
    size_t blen = strlen(baseReal);
    if (strncmp(combined, baseReal, blen) != 0) return 0;
    if (snprintf(outPath, outSize, "%s", combined) >= (int)outSize) return 0;
    return 1;
}

static int serialize_record(int iterations, const unsigned char* salt, const unsigned char* hash, unsigned char** out, size_t* out_len) {
    *out_len = 4 + 1 + 4 + 1 + 1 + SALT_LEN + HASH_LEN;
    *out = (unsigned char*)malloc(*out_len);
    if (!*out) return 0;
    size_t idx = 0;
    memcpy(*out + idx, MAGIC, 4); idx += 4;
    (*out)[idx++] = VERSION;
    (*out)[idx++] = (iterations >> 24) & 0xFF;
    (*out)[idx++] = (iterations >> 16) & 0xFF;
    (*out)[idx++] = (iterations >> 8) & 0xFF;
    (*out)[idx++] = (iterations) & 0xFF;
    (*out)[idx++] = SALT_LEN;
    (*out)[idx++] = HASH_LEN;
    memcpy(*out + idx, salt, SALT_LEN); idx += SALT_LEN;
    memcpy(*out + idx, hash, HASH_LEN); idx += HASH_LEN;
    return 1;
}

static int parse_record(const unsigned char* data, size_t len, ParsedRecord* rec) {
    if (len < 4 + 1 + 4 + 1 + 1) return 0;
    if (memcmp(data, MAGIC, 4) != 0) return 0;
    if (data[4] != VERSION) return 0;
    int iterations = (data[5]<<24) | (data[6]<<16) | (data[7]<<8) | (data[8]);
    unsigned int sLen = data[9];
    unsigned int hLen = data[10];
    if (sLen != SALT_LEN || hLen != HASH_LEN) return 0;
    size_t expected = 4 + 1 + 4 + 1 + 1 + sLen + hLen;
    if (len != expected) return 0;
    rec->iterations = iterations;
    memcpy(rec->salt, data + 11, SALT_LEN);
    memcpy(rec->hash, data + 11 + SALT_LEN, HASH_LEN);
    return 1;
}

int store_password(const char* baseDir, const char* relPath, const char* password, int iterations) {
    if (!baseDir || !relPath || !password) return 0;
    int iters = iterations < MIN_ITER ? MIN_ITER : iterations;

    char targetPath[PATH_MAX];
    if (!path_within_base(baseDir, relPath, targetPath, sizeof(targetPath))) return 0;

    // Ensure base directory exists
    struct stat st;
    if (stat(baseDir, &st) != 0) {
        if (mkdir(baseDir, 0700) != 0 && errno != EEXIST) return 0;
    }

    unsigned char salt[SALT_LEN];
    if (!generate_salt(salt, SALT_LEN)) return 0;

    unsigned char hash[HASH_LEN];
    if (!pbkdf2_sha256(password, salt, SALT_LEN, iters, hash, HASH_LEN)) return 0;

    unsigned char* content = NULL;
    size_t content_len = 0;
    if (!serialize_record(iters, salt, hash, &content, &content_len)) return 0;

    char tmpPath[PATH_MAX];
    snprintf(tmpPath, sizeof(tmpPath), "%s/.tmp-%d-%ld", baseDir, getpid(), (long)time(NULL));
    int fd = open(tmpPath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        free(content);
        return 0;
    }

    int ok = 1;
    if (flock(fd, LOCK_EX) != 0) ok = 0;
    if (ok) {
        ssize_t w = write(fd, content, content_len);
        if (w < 0 || (size_t)w != content_len) ok = 0;
    }
    if (ok && fsync(fd) != 0) ok = 0;
    close(fd);

    if (!ok) {
        unlink(tmpPath);
        free(content);
        return 0;
    }

    if (rename(tmpPath, targetPath) != 0) {
        unlink(tmpPath);
        free(content);
        return 0;
    }
    set_owner_only_permissions(targetPath);
    free(content);
    return 1;
}

int verify_password(const char* baseDir, const char* relPath, const char* password) {
    if (!baseDir || !relPath || !password) return 0;

    char targetPath[PATH_MAX];
    if (!path_within_base(baseDir, relPath, targetPath, sizeof(targetPath))) return 0;

    struct stat lst;
    if (lstat(targetPath, &lst) != 0) return 0;
    if (!S_ISREG(lst.st_mode)) return 0;
    if (S_ISLNK(lst.st_mode)) return 0;

    int flags = O_RDONLY;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int fd = open(targetPath, flags);
    if (fd < 0) return 0;

    int ok = 0;
    do {
        if (flock(fd, LOCK_SH) != 0) break;
        off_t sz = lseek(fd, 0, SEEK_END);
        if (sz <= 0 || sz > (1<<20)) break;
        if (lseek(fd, 0, SEEK_SET) < 0) break;
        unsigned char* data = (unsigned char*)malloc((size_t)sz);
        if (!data) break;
        ssize_t r = read(fd, data, (size_t)sz);
        if (r < 0 || (size_t)r != (size_t)sz) {
            free(data);
            break;
        }
        ParsedRecord rec;
        if (!parse_record(data, (size_t)sz, &rec)) {
            free(data);
            break;
        }
        if (rec.iterations < MIN_ITER) {
            free(data);
            break;
        }
        unsigned char computed[HASH_LEN];
        if (!pbkdf2_sha256(password, rec.salt, SALT_LEN, rec.iterations, computed, HASH_LEN)) {
            free(data);
            break;
        }
        ok = constant_time_equal(computed, rec.hash, HASH_LEN);
        free(data);
    } while (0);

    close(fd);
    return ok;
}

int main(void) {
    const char* base = "safe_store_c";
    mkdir(base, 0700);

    int t1s = store_password(base, "user1.pwd", "Password#123", 210000);
    int t1v = verify_password(base, "user1.pwd", "Password#123");
    printf("Test1 (store+verify correct): %s\n", (t1s && t1v) ? "1" : "0");

    int t2v = verify_password(base, "user1.pwd", "WrongPassword");
    printf("Test2 (verify incorrect): %s\n", (!t2v) ? "1" : "0");

    int t3v = verify_password(base, "../evil.pwd", "anything");
    printf("Test3 (path traversal blocked): %s\n", (!t3v) ? "1" : "0");

    int t4v = verify_password(base, "does_not_exist.pwd", "Password#123");
    printf("Test4 (non-existent file): %s\n", (!t4v) ? "1" : "0");

    int t5s = store_password(base, "user2.pwd", "Abc!2345", 300000);
    int t5v = verify_password(base, "user2.pwd", "Abc!2345");
    printf("Test5 (second user correct): %s\n", (t5s && t5v) ? "1" : "0");

    return 0;
}
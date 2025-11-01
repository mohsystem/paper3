#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SALT_LEN 16
#define DK_LEN 32
#define PBKDF2_ITERS 210000

static bool is_valid_username(const char* username) {
    if (!username) return false;
    size_t len = strlen(username);
    if (len < 3 || len > 32) return false;
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!(isalnum((unsigned char)c) || c == '_')) return false;
    }
    return true;
}

static bool is_strong_password(const char* password) {
    if (!password) return false;
    size_t len = strlen(password);
    if (len < 12) return false;
    int hasLower = 0, hasUpper = 0, hasDigit = 0, hasSpecial = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (islower(c)) hasLower = 1;
        else if (isupper(c)) hasUpper = 1;
        else if (isdigit(c)) hasDigit = 1;
        else hasSpecial = 1;
    }
    return hasLower && hasUpper && hasDigit && hasSpecial;
}

static char* b64_encode(const unsigned char* data, int len) {
    int out_len = 4 * ((len + 2) / 3);
    unsigned char* out = (unsigned char*)calloc((size_t)out_len + 1, 1);
    if (!out) return NULL;
    int written = EVP_EncodeBlock(out, data, len);
    if (written < 0) {
        free(out);
        return NULL;
    }
    char* s = (char*)out;
    s[written] = '\0';
    return s;
}

static bool ensure_db_file(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) return true;
    int fd = open(path, O_CREAT | O_EXCL | O_WRONLY, 0600);
    if (fd < 0) return false;
    // Optional header write (none)
    close(fd);
    return true;
}

static bool username_exists(const char* path, const char* username) {
    FILE* f = fopen(path, "r");
    if (!f) return false;
    char line[4096];
    bool exists = false;
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '\0') continue;
        // Extract username until '|'
        char* p = strchr(line, '|');
        if (!p) continue;
        *p = '\0';
        if (strcmp(line, username) == 0) {
            exists = true;
            break;
        }
    }
    fclose(f);
    return exists;
}

static bool hash_password(const unsigned char* pass, size_t pass_len, char** outSaltB64, char** outHashB64) {
    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return false;
    }
    unsigned char dk[DK_LEN];
    if (PKCS5_PBKDF2_HMAC((const char*)pass, (int)pass_len, salt, SALT_LEN, PBKDF2_ITERS, EVP_sha256(), DK_LEN, dk) != 1) {
        OPENSSL_cleanse(salt, sizeof(salt));
        return false;
    }
    char* saltB64 = b64_encode(salt, SALT_LEN);
    char* hashB64 = b64_encode(dk, DK_LEN);
    OPENSSL_cleanse(salt, sizeof(salt));
    OPENSSL_cleanse(dk, sizeof(dk));
    if (!saltB64 || !hashB64) {
        free(saltB64);
        free(hashB64);
        return false;
    }
    *outSaltB64 = saltB64;
    *outHashB64 = hashB64;
    return true;
}

static bool insert_user(const char* db_path, const char* username, const char* password) {
    if (!is_valid_username(username)) return false;
    if (!is_strong_password(password)) return false;
    if (!ensure_db_file(db_path)) return false;
    if (username_exists(db_path, username)) return false;

    unsigned char* passBuf = NULL;
    size_t pass_len = strlen(password);
    passBuf = (unsigned char*)malloc(pass_len);
    if (!passBuf) return false;
    memcpy(passBuf, password, pass_len);

    char* saltB64 = NULL;
    char* hashB64 = NULL;
    bool ok = hash_password(passBuf, pass_len, &saltB64, &hashB64);
    OPENSSL_cleanse(passBuf, pass_len);
    free(passBuf);
    if (!ok) return false;

    // created at
    char ts[64];
    time_t t = time(NULL);
    struct tm tm;
#if defined(_WIN32) && !defined(__MINGW32__)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", &tm);

    FILE* f = fopen(db_path, "a");
    if (!f) {
        free(saltB64);
        free(hashB64);
        return false;
    }
    int n = fprintf(f, "%s|%d|%s|%s|%s\n", username, PBKDF2_ITERS, saltB64, hashB64, ts);
    fflush(f);
    int ferr = ferror(f);
    fclose(f);
    free(saltB64);
    free(hashB64);
    return (n > 0 && ferr == 0);
}

int main(void) {
    const char* db = "users_c.db";
    unlink(db);

    struct TestCase { const char* u; const char* p; };
    struct TestCase tests[5] = {
        {"alice_01", "StrongPass!234"},
        {"bob_02", "Another$trongP4ss"},
        {"alice_01", "DupUserGoodP@ss1"}, // duplicate username
        {"ab", "ValidButUserTooShort1!"},
        {"charlie_03", "weakpass"} // weak password
    };

    for (int i = 0; i < 5; i++) {
        bool ok = insert_user(db, tests[i].u, tests[i].p);
        printf("Test %d insert(%s): %s\n", i+1, tests[i].u, ok ? "OK" : "FAIL");
    }

    // Count records
    FILE* f = fopen(db, "r");
    int count = 0;
    if (f) {
        char line[4096];
        while (fgets(line, sizeof(line), f)) {
            if (line[0] != '\0') count++;
        }
        fclose(f);
    }
    printf("Total records in DB file: %d\n", count);
    return 0;
}
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SALT_LEN 16
#define ITERATIONS 100000
#define KEY_LEN 32
#define MAX_USERS 100
#define USERNAME_MAX 64

typedef struct {
    char username[USERNAME_MAX];
    char *stored; // "salt_hex:hash_hex"
} UserRecord;

static UserRecord DATABASE[MAX_USERS];
static size_t DB_SIZE = 0;

static int generate_salt(unsigned char *buf, size_t len) {
    return RAND_bytes(buf, (int)len) == 1 ? 1 : 0;
}

static int derive_key(const char *password, const unsigned char *salt, size_t salt_len,
                      unsigned char *out_key, size_t key_len, int iterations) {
    return PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                             salt, (int)salt_len,
                             iterations, EVP_sha256(),
                             (int)key_len, out_key) == 1 ? 1 : 0;
}

static char *to_hex(const unsigned char *data, size_t len) {
    static const char *hex = "0123456789abcdef";
    char *out = (char *)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[i * 2] = hex[(data[i] >> 4) & 0xF];
        out[i * 2 + 1] = hex[data[i] & 0xF];
    }
    out[len * 2] = '\0';
    return out;
}

static int username_exists(const char *username) {
    for (size_t i = 0; i < DB_SIZE; ++i) {
        if (strncmp(DATABASE[i].username, username, USERNAME_MAX) == 0) return 1;
    }
    return 0;
}

char *register_user(const char *username, const char *password) {
    if (!username || !password || username[0] == '\0' || password[0] == '\0') return NULL;
    if (username_exists(username)) return NULL;
    if (DB_SIZE >= MAX_USERS) return NULL;

    unsigned char salt[SALT_LEN];
    unsigned char key[KEY_LEN];
    if (!generate_salt(salt, SALT_LEN)) return NULL;
    if (!derive_key(password, salt, SALT_LEN, key, KEY_LEN, ITERATIONS)) return NULL;

    char *salt_hex = to_hex(salt, SALT_LEN);
    char *key_hex = to_hex(key, KEY_LEN);
    if (!salt_hex || !key_hex) {
        free(salt_hex);
        free(key_hex);
        return NULL;
    }

    size_t stored_len = strlen(salt_hex) + 1 + strlen(key_hex) + 1;
    char *stored = (char *)malloc(stored_len);
    if (!stored) {
        free(salt_hex);
        free(key_hex);
        return NULL;
    }
    snprintf(stored, stored_len, "%s:%s", salt_hex, key_hex);

    strncpy(DATABASE[DB_SIZE].username, username, USERNAME_MAX - 1);
    DATABASE[DB_SIZE].username[USERNAME_MAX - 1] = '\0';
    DATABASE[DB_SIZE].stored = strdup(stored);
    DB_SIZE++;

    free(salt_hex);
    free(key_hex);
    return stored; // caller responsible to free
}

int main(void) {
    // 5 test cases
    char *res1 = register_user("alice", "Password1!");
    printf("Register alice: %s\n", res1 ? res1 : "(null)");
    char *res2 = register_user("bob", "S3cure#Pass");
    printf("Register bob: %s\n", res2 ? res2 : "(null)");
    char *res3 = register_user("carol", "anotherPass123");
    printf("Register carol: %s\n", res3 ? res3 : "(null)");
    char *res4 = register_user("alice", "newPass!");
    printf("Register alice again (should fail): %s\n", res4 ? res4 : "(null)");
    char *res5 = register_user("dave", "p@ssW0rd");
    printf("Register dave: %s\n", res5 ? res5 : "(null)");

    free(res1);
    free(res2);
    free(res3);
    free(res4);
    free(res5);

    // free database allocated strings
    for (size_t i = 0; i < DB_SIZE; ++i) {
        free(DATABASE[i].stored);
    }
    return 0;
}
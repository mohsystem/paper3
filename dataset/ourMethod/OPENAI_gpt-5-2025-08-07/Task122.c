#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SALT_LEN 16
#define DK_LEN 32
#define ITERATIONS 210000

typedef struct {
    char* username;
    unsigned char salt[SALT_LEN];
    unsigned char hash[DK_LEN];
    int iterations;
} UserRecord;

typedef struct {
    UserRecord* records;
    size_t count;
    size_t capacity;
} UserStore;

static int init_store(UserStore* store, size_t capacity) {
    if (!store || capacity == 0) return 0;
    store->records = (UserRecord*)calloc(capacity, sizeof(UserRecord));
    if (!store->records) return 0;
    store->count = 0;
    store->capacity = capacity;
    return 1;
}

static void free_store(UserStore* store) {
    if (!store) return;
    for (size_t i = 0; i < store->count; i++) {
        if (store->records[i].username) {
            // best-effort clear then free
            size_t n = strlen(store->records[i].username);
            memset(store->records[i].username, 0, n);
            free(store->records[i].username);
            store->records[i].username = NULL;
        }
        // Clear sensitive data
        OPENSSL_cleanse(store->records[i].salt, SALT_LEN);
        OPENSSL_cleanse(store->records[i].hash, DK_LEN);
    }
    free(store->records);
    store->records = NULL;
    store->count = 0;
    store->capacity = 0;
}

static int valid_username(const char* u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len < 1 || len > 64) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int validate_password_policy(const char* p) {
    if (!p) return 0;
    size_t len = strlen(p);
    if (len < 12 || len > 128) return 0;
    int hasU=0, hasL=0, hasD=0, hasS=0;
    const char* specials = "!@#$%^&*()-_=+[]{};:'\",.<>/?`~\\|";
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)p[i];
        if (c == 0 || c < 32) return 0;
        if (isupper(c)) hasU = 1;
        else if (islower(c)) hasL = 1;
        else if (isdigit(c)) hasD = 1;
        else if (strchr(specials, (int)c) != NULL) hasS = 1;
    }
    return (hasU && hasL && hasD && hasS) ? 1 : 0;
}

static int user_exists(const UserStore* store, const char* username) {
    if (!store || !username) return 0;
    for (size_t i = 0; i < store->count; i++) {
        if (store->records[i].username && strcmp(store->records[i].username, username) == 0) {
            return 1;
        }
    }
    return 0;
}

static int derive_key_pbkdf2(const char* password, const unsigned char* salt, int iterations,
                             unsigned char* out, int out_len) {
    if (!password || !salt || !out) return 0;
    int ok = PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                               salt, SALT_LEN, iterations,
                               EVP_sha256(), out_len, out);
    return ok == 1 ? 1 : 0;
}

static int signup(UserStore* store, const char* username, const char* password) {
    if (!store || !username || !password) return 0;
    if (!valid_username(username)) return 0;
    if (!validate_password_policy(password)) return 0;
    if (user_exists(store, username)) return 0;
    if (store->count >= store->capacity) return 0;

    UserRecord rec;
    memset(&rec, 0, sizeof(rec));
    if (RAND_bytes(rec.salt, SALT_LEN) != 1) {
        return 0;
    }
    if (!derive_key_pbkdf2(password, rec.salt, ITERATIONS, rec.hash, DK_LEN)) {
        return 0;
    }
    rec.iterations = ITERATIONS;

    size_t ulen = strlen(username);
    rec.username = (char*)malloc(ulen + 1);
    if (!rec.username) {
        OPENSSL_cleanse(rec.salt, SALT_LEN);
        OPENSSL_cleanse(rec.hash, DK_LEN);
        return 0;
    }
    memcpy(rec.username, username, ulen);
    rec.username[ulen] = '\0';

    store->records[store->count++] = rec;
    return 1;
}

int main(void) {
    UserStore store;
    if (!init_store(&store, 100)) {
        printf("init failed\n");
        return 1;
    }

    struct Test { const char* u; const char* p; } tests[5] = {
        {"alice", "ValidPassw0rd!"},
        {"bob", "short"},
        {"alice", "AnotherValid1!"},
        {"carol", "Another$trong1"},
        {"dave", "NoSpecialChar1"}
    };

    for (int i = 0; i < 5; i++) {
        int res = signup(&store, tests[i].u, tests[i].p);
        printf("%s: %s\n", tests[i].u, res ? "signup ok" : "signup failed");
    }

    free_store(&store);
    return 0;
}
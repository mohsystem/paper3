#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define MAX_USERS 100
#define MAX_USER_LEN 128
#define HASH_HEX_LEN 16  // 64-bit hex without 0x
#define HASH_STR_LEN (HASH_HEX_LEN + 1)

typedef struct {
    char username[MAX_USER_LEN];
    char hash[HASH_STR_LEN];
} UserEntry;

static UserEntry DB[MAX_USERS];
static size_t DB_COUNT = 0;

// FNV-1a 64-bit (for demonstration). Not cryptographically secure.
static char* hash_password(const char* password) {
    const uint64_t FNV_OFFSET_BASIS = 1469598103934665603ULL;
    const uint64_t FNV_PRIME = 1099511628211ULL;
    uint64_t h = FNV_OFFSET_BASIS;
    if (password) {
        for (const unsigned char* p = (const unsigned char*)password; *p; ++p) {
            h ^= *p;
            h *= FNV_PRIME;
        }
    }
    char* hex = (char*)malloc(HASH_STR_LEN);
    if (!hex) return NULL;
    snprintf(hex, HASH_STR_LEN, "%016" PRIx64, h);
    return hex;
}

int insert_user(const char* username, const char* password) {
    if (!username || !password) return 0;
    // Check if exists
    for (size_t i = 0; i < DB_COUNT; ++i) {
        if (strncmp(DB[i].username, username, MAX_USER_LEN) == 0) {
            return 0; // already exists
        }
    }
    if (DB_COUNT >= MAX_USERS) return 0;

    char* h = hash_password(password);
    if (!h) return 0;

    strncpy(DB[DB_COUNT].username, username, MAX_USER_LEN - 1);
    DB[DB_COUNT].username[MAX_USER_LEN - 1] = '\0';

    strncpy(DB[DB_COUNT].hash, h, HASH_STR_LEN - 1);
    DB[DB_COUNT].hash[HASH_STR_LEN - 1] = '\0';

    free(h);
    DB_COUNT++;
    return 1;
}

const char* get_hashed_password(const char* username) {
    if (!username) return NULL;
    for (size_t i = 0; i < DB_COUNT; ++i) {
        if (strncmp(DB[i].username, username, MAX_USER_LEN) == 0) {
            return DB[i].hash;
        }
    }
    return NULL;
}

int main(void) {
    struct { const char* u; const char* p; } tests[5] = {
        {"alice", "password123"},
        {"bob", "s3cr3t!"},
        {"charlie", "Pa$$w0rd"},
        {"alice", "newpass"},
        {"eve", ""}
    };

    for (int i = 0; i < 5; ++i) {
        int ok = insert_user(tests[i].u, tests[i].p);
        const char* h = get_hashed_password(tests[i].u);
        printf("Insert(%s): %s | hash=%s\n", tests[i].u, ok ? "true" : "false", h ? h : "(null)");
    }
    printf("Total users: %zu\n", DB_COUNT);
    return 0;
}
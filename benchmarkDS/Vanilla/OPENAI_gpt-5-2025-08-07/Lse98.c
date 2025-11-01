#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_USERS 100
#define USERNAME_MAX 64
#define HASH_HEX_LEN 16  /* 64-bit hex length without leading zeros consideration */
#define HASH_HEX_BUF (HASH_HEX_LEN + 1)

static const char* SALT = "Lse98$Salt";

typedef struct {
    char username[USERNAME_MAX];
    char hash[HASH_HEX_BUF];
} Record;

static Record DB[MAX_USERS];
static int DB_COUNT = 0;

static uint64_t djb2_update(uint64_t h, const char* s) {
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        h = ((h << 5) + h) ^ *p; /* djb2 variant */
    }
    return h;
}

static void hash_password(const char* password, char out_hex[HASH_HEX_BUF]) {
    uint64_t h = 5381ULL;
    h = djb2_update(h, SALT);
    h = djb2_update(h, ":");
    h = djb2_update(h, password);
    /* Convert to hex; ensure null-terminated */
    /* Print without leading zeros trimming concern: use lowercase hex */
    /* Note: snprintf will not pad; that's acceptable for this example */
    snprintf(out_hex, HASH_HEX_BUF, "%llx", (unsigned long long)h);
}

static int find_user_index(const char* username) {
    for (int i = 0; i < DB_COUNT; ++i) {
        if (strcmp(DB[i].username, username) == 0) return i;
    }
    return -1;
}

/* Returns 1 on success, 0 on failure */
int register_user(const char* username, const char* password) {
    if (!username || !password || username[0] == '\0' || password[0] == '\0') return 0;
    if (find_user_index(username) != -1) return 0;
    if (DB_COUNT >= MAX_USERS) return 0;
    char hex[HASH_HEX_BUF];
    hash_password(password, hex);
    /* store */
    strncpy(DB[DB_COUNT].username, username, USERNAME_MAX - 1);
    DB[DB_COUNT].username[USERNAME_MAX - 1] = '\0';
    strncpy(DB[DB_COUNT].hash, hex, HASH_HEX_BUF - 1);
    DB[DB_COUNT].hash[HASH_HEX_BUF - 1] = '\0';
    DB_COUNT++;
    return 1;
}

const char* get_stored_hash(const char* username) {
    int idx = find_user_index(username);
    if (idx == -1) return NULL;
    return DB[idx].hash;
}

int main(void) {
    printf("Test1 (alice): %s\n", register_user("alice", "password123") ? "true" : "false");
    printf("Test2 (bob): %s\n", register_user("bob", "secret") ? "true" : "false");
    printf("Test3 (duplicate alice): %s\n", register_user("alice", "another") ? "true" : "false");
    printf("Test4 (empty username): %s\n", register_user("", "nopass") ? "true" : "false");
    printf("Test5 (empty password): %s\n", register_user("charlie", "") ? "true" : "false");
    printf("alice hash: %s\n", get_stored_hash("alice"));
    printf("bob hash: %s\n", get_stored_hash("bob"));
    printf("charlie hash: %s\n", get_stored_hash("charlie"));
    return 0;
}
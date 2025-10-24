#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SALT "Task102_SALT"
#define MAX_USERS 10
#define USERNAME_MAX 64

typedef struct {
    char username[USERNAME_MAX];
    char hash[17]; /* 16 hex chars + null */
    int used;
} Entry;

typedef struct {
    Entry entries[MAX_USERS];
    int count;
} DB;

static void fnv1a64_hex(const char* s, char out_hex[17]) {
    unsigned long long offset = 14695981039346656037ULL;
    unsigned long long prime = 1099511628211ULL;
    unsigned long long h = offset;
    const unsigned char* p = (const unsigned char*)s;
    while (*p) {
        h ^= (unsigned long long)(*p++);
        h *= prime;
    }
    /* format to 16 hex digits */
    snprintf(out_hex, 17, "%016llx", h);
}

void hash_password(const char* password, char out_hex[17]) {
    size_t salt_len = strlen(SALT);
    size_t pass_len = password ? strlen(password) : 0;
    size_t total = salt_len + pass_len;
    char* buf = (char*)malloc(total + 1);
    if (!buf) {
        out_hex[0] = '\0';
        return;
    }
    memcpy(buf, SALT, salt_len);
    if (password) memcpy(buf + salt_len, password, pass_len);
    buf[total] = '\0';
    fnv1a64_hex(buf, out_hex);
    free(buf);
}

static int find_user(DB* db, const char* username) {
    for (int i = 0; i < db->count; ++i) {
        if (db->entries[i].used && strcmp(db->entries[i].username, username) == 0) return i;
    }
    return -1;
}

int update_password(DB* db, const char* username, const char* new_password) {
    if (!db || !username || !new_password || new_password[0] == '\0') return 0;
    int idx = find_user(db, username);
    if (idx < 0) return 0;
    char hashed[17];
    hash_password(new_password, hashed);
    strncpy(db->entries[idx].hash, hashed, sizeof(db->entries[idx].hash));
    db->entries[idx].hash[16] = '\0';
    return 1;
}

const char* get_password_hash(DB* db, const char* username) {
    int idx = find_user(db, username);
    if (idx < 0) return NULL;
    return db->entries[idx].hash;
}

void init_db(DB* db) {
    db->count = 0;
    for (int i = 0; i < MAX_USERS; ++i) db->entries[i].used = 0;

    const char* users[] = {"alice", "bob", "charlie"};
    const char* pwds[]  = {"old1",  "old2", "old3"};
    for (int i = 0; i < 3; ++i) {
        strncpy(db->entries[i].username, users[i], USERNAME_MAX - 1);
        db->entries[i].username[USERNAME_MAX - 1] = '\0';
        hash_password(pwds[i], db->entries[i].hash);
        db->entries[i].used = 1;
        db->count++;
    }
}

int main(void) {
    DB db;
    init_db(&db);

    printf("Initial DB:\n");
    for (int i = 0; i < db.count; ++i) {
        if (db.entries[i].used) {
            printf("  %s=%s\n", db.entries[i].username, db.entries[i].hash);
        }
    }

    int r1 = update_password(&db, "alice", "newPass1");
    printf("Test1 update alice: %s hash=%s\n", r1 ? "true" : "false", get_password_hash(&db, "alice"));

    int r2 = update_password(&db, "bob", "P@ssw0rd!");
    printf("Test2 update bob: %s hash=%s\n", r2 ? "true" : "false", get_password_hash(&db, "bob"));

    int r3 = update_password(&db, "david", "something");
    printf("Test3 update david: %s\n", r3 ? "true" : "false");

    int r4 = update_password(&db, "charlie", "");
    const char* ch = get_password_hash(&db, "charlie");
    printf("Test4 update charlie with empty: %s hash=%s\n", r4 ? "true" : "false", ch ? ch : "NULL");

    int r5 = update_password(&db, "alice", "another123");
    printf("Test5 update alice again: %s hash=%s\n", r5 ? "true" : "false", get_password_hash(&db, "alice"));

    return 0;
}
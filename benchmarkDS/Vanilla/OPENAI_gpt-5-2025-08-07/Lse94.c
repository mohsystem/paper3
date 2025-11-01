#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_USERS 128
#define SALT_LEN 16

typedef struct {
    char username[128];
    char salt_hex[SALT_LEN * 2 + 1];
    char hash_hex[16 + 1]; /* 64-bit FNV-1a -> 16 hex chars */
    bool success;
} UserRecord;

typedef struct {
    UserRecord records[MAX_USERS];
    size_t count;
} Database;

static void random_bytes(uint8_t* buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        buf[i] = (uint8_t)(rand() & 0xFF);
    }
}

static void bytes_to_hex(const uint8_t* bytes, size_t len, char* out_hex) {
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < len; ++i) {
        out_hex[i*2] = hex[(bytes[i] >> 4) & 0xF];
        out_hex[i*2 + 1] = hex[bytes[i] & 0xF];
    }
    out_hex[len*2] = '\0';
}

static void u64_to_hex(uint64_t v, char* out_hex) {
    static const char* hex = "0123456789abcdef";
    for (int i = 15; i >= 0; --i) {
        out_hex[i] = hex[v & 0xF];
        v >>= 4;
    }
    out_hex[16] = '\0';
}

static uint64_t fnv1a64_update(uint64_t hash, const uint8_t* data, size_t len) {
    const uint64_t prime = 1099511628211ULL;
    for (size_t i = 0; i < len; ++i) {
        hash ^= data[i];
        hash *= prime;
    }
    return hash;
}

static void hash_password_with_salt(const char* password, const uint8_t* salt, size_t salt_len, char* out_hash_hex) {
    uint64_t hash = 1469598103934665603ULL;
    hash = fnv1a64_update(hash, salt, salt_len);
    hash = fnv1a64_update(hash, (const uint8_t*)password, strlen(password));
    u64_to_hex(hash, out_hash_hex);
}

static bool db_insert(Database* db, const UserRecord* rec) {
    if (db->count >= MAX_USERS) return false;
    for (size_t i = 0; i < db->count; ++i) {
        if (strcmp(db->records[i].username, rec->username) == 0) return false;
    }
    db->records[db->count++] = *rec;
    return true;
}

UserRecord register_user(Database* db, const char* username, const char* password) {
    UserRecord rec;
    memset(&rec, 0, sizeof(rec));
    strncpy(rec.username, username, sizeof(rec.username)-1);

    uint8_t salt[SALT_LEN];
    random_bytes(salt, SALT_LEN);
    bytes_to_hex(salt, SALT_LEN, rec.salt_hex);

    hash_password_with_salt(password, salt, SALT_LEN, rec.hash_hex);

    rec.success = db_insert(db, &rec);
    if (!rec.success) {
        UserRecord fail;
        memset(&fail, 0, sizeof(fail));
        fail.success = false;
        return fail;
    }
    return rec;
}

int main(void) {
    Database db;
    db.count = 0;
    srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)&db);

    const char* users[5] = {"alice","bob","charlie","dana","eve"};
    const char* passes[5] = {"p@ssw0rd","hunter2","letmein","qwerty123","correct horse battery staple"};

    for (int i = 0; i < 5; ++i) {
        UserRecord r = register_user(&db, users[i], passes[i]);
        if (r.success) {
            printf("Inserted: username='%s', salt='%s', hash='%s'\n", r.username, r.salt_hex, r.hash_hex);
        } else {
            printf("Insert failed for %s\n", users[i]);
        }
    }

    printf("All records in DB:\n");
    for (size_t i = 0; i < db.count; ++i) {
        printf("UserRecord(username='%s', salt='%s', hash='%s')\n",
               db.records[i].username, db.records[i].salt_hex, db.records[i].hash_hex);
    }

    return 0;
}
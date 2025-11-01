#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

typedef struct {
    char** keys;
    char** values;
    size_t size;
    size_t capacity;
} DB;

void db_init(DB* db, size_t capacity) {
    db->keys = (char**)calloc(capacity, sizeof(char*));
    db->values = (char**)calloc(capacity, sizeof(char*));
    db->size = 0;
    db->capacity = capacity;
}

void db_free(DB* db) {
    if (!db) return;
    for (size_t i = 0; i < db->size; ++i) {
        free(db->keys[i]);
        free(db->values[i]);
    }
    free(db->keys);
    free(db->values);
    db->keys = NULL;
    db->values = NULL;
    db->size = db->capacity = 0;
}

void db_put(DB* db, const char* key, const char* value) {
    if (db->size >= db->capacity) {
        size_t newcap = db->capacity * 2 + 1;
        db->keys = (char**)realloc(db->keys, newcap * sizeof(char*));
        db->values = (char**)realloc(db->values, newcap * sizeof(char*));
        db->capacity = newcap;
    }
    db->keys[db->size] = strdup(key);
    db->values[db->size] = strdup(value);
    db->size += 1;
}

static void random_bytes(unsigned char* buf, size_t len) {
    static int seeded = 0;
    if (!seeded) {
        seeded = 1;
        srand((unsigned)time(NULL));
    }
    for (size_t i = 0; i < len; ++i) {
        buf[i] = (unsigned char)(rand() % 256);
    }
}

char* bytes_to_hex(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    char* out = (char*)malloc(len * 2 + 1);
    for (size_t i = 0; i < len; ++i) {
        out[2*i]   = hex[(data[i] >> 4) & 0xF];
        out[2*i+1] = hex[data[i] & 0xF];
    }
    out[len*2] = '\0';
    return out;
}

char* generate_salt(int length_bytes) {
    if (length_bytes <= 0) length_bytes = 16;
    unsigned char* buf = (unsigned char*)malloc((size_t)length_bytes);
    random_bytes(buf, (size_t)length_bytes);
    char* hex = bytes_to_hex(buf, (size_t)length_bytes);
    free(buf);
    return hex;
}

char* fnv1a_hash(const char* input) {
    const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325ULL;
    const uint64_t FNV_PRIME = 0x100000001b3ULL;
    uint64_t h = FNV_OFFSET_BASIS;
    const unsigned char* p = (const unsigned char*)input;
    while (*p) {
        h ^= (uint64_t)(*p);
        h *= FNV_PRIME;
        p++;
    }
    char* out = (char*)malloc(16 + 1);
    snprintf(out, 17, "%016llx", (unsigned long long)h);
    return out;
}

char* register_user(DB* db, const char* username, const char* password) {
    char* hashed = fnv1a_hash(password);
    char* salt = generate_salt(16);
    size_t len = strlen(salt) + 1 + strlen(hashed);
    char* stored = (char*)malloc(len + 1);
    sprintf(stored, "%s:%s", salt, hashed);
    db_put(db, username, stored);
    free(hashed);
    free(salt);
    return stored;
}

int main(void) {
    DB db;
    db_init(&db, 8);

    const char* users[5] = {"alice","bob","charlie","diana","eve"};
    const char* passes[5] = {"password123","S3cure!","letmein","hunter2","correcthorsebatterystaple"};

    for (int i = 0; i < 5; ++i) {
        char* stored = register_user(&db, users[i], passes[i]);
        printf("%s -> %s\n", users[i], stored);
        free(stored);
    }

    db_free(&db);
    return 0;
}
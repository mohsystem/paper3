#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

typedef struct {
    char username[128];
    char salt_b64[64];
    char hash_b64[128];
} UserRecord;

typedef struct {
    UserRecord records[128];
    size_t count;
} InMemoryDatabase;

static int db_insert(InMemoryDatabase* db, const UserRecord* rec) {
    if (!db || !rec || !rec->username[0]) return 0;
    for (size_t i = 0; i < db->count; ++i) {
        if (strcmp(db->records[i].username, rec->username) == 0) return 0;
    }
    if (db->count >= 128) return 0;
    db->records[db->count++] = *rec;
    return 1;
}

static const UserRecord* db_get(const InMemoryDatabase* db, const char* username) {
    if (!db || !username) return NULL;
    for (size_t i = 0; i < db->count; ++i) {
        if (strcmp(db->records[i].username, username) == 0) return &db->records[i];
    }
    return NULL;
}

static int b64encode(const unsigned char* in, int inlen, char* out, int outcap) {
    int needed = 4 * ((inlen + 2) / 3);
    if (outcap < needed + 1) return -1;
    int written = EVP_EncodeBlock((unsigned char*)out, in, inlen);
    if (written < 0) return -1;
    out[written] = '\0';
    return written;
}

static int generate_salt(unsigned char* buf, int len) {
    if (!buf || len <= 0) return 0;
    return RAND_bytes(buf, len) == 1;
}

static int hash_password(const char* password, const unsigned char* salt, int saltlen,
                         int iterations, int dkLen, unsigned char* out) {
    if (!password || !salt || !out) return 0;
    if (iterations < 10000 || dkLen < 16) return 0;
    return PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                             salt, saltlen, iterations, EVP_sha256(), dkLen, out) == 1;
}

int register_user(const char* username, const char* password, InMemoryDatabase* db) {
    if (!db || !username || !password) return 0;
    if (username[0] == '\0' || password[0] == '\0') return 0;

    unsigned char salt[16];
    if (!generate_salt(salt, sizeof(salt))) return 0;

    unsigned char dk[32];
    if (!hash_password(password, salt, (int)sizeof(salt), 100000, (int)sizeof(dk), dk)) return 0;

    UserRecord rec;
    memset(&rec, 0, sizeof(rec));
    // Copy username safely
    snprintf(rec.username, sizeof(rec.username), "%s", username);

    if (b64encode(salt, (int)sizeof(salt), rec.salt_b64, (int)sizeof(rec.salt_b64)) < 0) return 0;
    if (b64encode(dk, (int)sizeof(dk), rec.hash_b64, (int)sizeof(rec.hash_b64)) < 0) return 0;

    return db_insert(db, &rec);
}

int main(void) {
    InMemoryDatabase db;
    memset(&db, 0, sizeof(db));

    const char* users[5] = {"alice", "bob", "charlie", "diana", "eve"};
    const char* passes[5] = {"S3cure!Pass1", "An0ther$Pass2", "Str0ng#Key3", "My$Passw0rd4", "Ultr@Safe5"};

    for (int i = 0; i < 5; ++i) {
        int ok = register_user(users[i], passes[i], &db);
        printf("Insert user %s: %s\n", users[i], ok ? "OK" : "FAIL");
    }

    for (int i = 0; i < 5; ++i) {
        const UserRecord* r = db_get(&db, users[i]);
        if (r) {
            printf("%s -> salt=%s, hash=%s\n", r->username, r->salt_b64, r->hash_b64);
        }
    }

    return 0;
}
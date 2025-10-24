#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define MAX_USERS 64
#define USERNAME_MAX 32
#define SALT_LEN 16
#define HASH_LEN 32
#define ITERATIONS 210000

typedef struct {
    char username[USERNAME_MAX + 1];
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    int iterations;
    int in_use;
} UserRecord;

typedef struct {
    UserRecord records[MAX_USERS];
} UserDatabase;

static int valid_username(const char* username) {
    if (username == NULL) return 0;
    size_t len = strnlen(username, USERNAME_MAX + 1);
    if (len < 3 || len > USERNAME_MAX) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!(isalnum((unsigned char)c) || c == '_')) return 0;
    }
    return 1;
}

static int strong_password(const char* pwd) {
    if (pwd == NULL) return 0;
    size_t len = strnlen(pwd, 129);
    if (len < 12 || len > 128) return 0;
    int hasL = 0, hasU = 0, hasD = 0, hasS = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)pwd[i];
        if (isspace(c)) return 0;
        if (islower(c)) hasL = 1;
        else if (isupper(c)) hasU = 1;
        else if (isdigit(c)) hasD = 1;
        else hasS = 1;
    }
    return hasL && hasU && hasD && hasS;
}

static int find_user_index(UserDatabase* db, const char* username) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (db->records[i].in_use && strncmp(db->records[i].username, username, USERNAME_MAX) == 0) {
            return i;
        }
    }
    return -1;
}

static int add_user(UserDatabase* db, const char* username, const char* password) {
    if (!valid_username(username) || !strong_password(password)) return 0;
    if (find_user_index(db, username) != -1) return 0;
    int idx = -1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (!db->records[i].in_use) { idx = i; break; }
    }
    if (idx == -1) return 0;

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) return 0;

    unsigned char hash[HASH_LEN];
    if (strlen(password) > INT_MAX) return 0;
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LEN, ITERATIONS, EVP_sha256(), HASH_LEN, hash) != 1) {
        return 0;
    }

    UserRecord* rec = &db->records[idx];
    memset(rec, 0, sizeof(*rec));
    snprintf(rec->username, sizeof(rec->username), "%s", username);
    memcpy(rec->salt, salt, SALT_LEN);
    memcpy(rec->hash, hash, HASH_LEN);
    rec->iterations = ITERATIONS;
    rec->in_use = 1;
    return 1;
}

static int update_password(UserDatabase* db, const char* username, const char* new_password) {
    if (!valid_username(username) || !strong_password(new_password)) return 0;
    int idx = find_user_index(db, username);
    if (idx < 0) return 0;

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) return 0;

    unsigned char hash[HASH_LEN];
    size_t pwlen = strlen(new_password);
    if (pwlen > INT_MAX) return 0;
    if (PKCS5_PBKDF2_HMAC(new_password, (int)pwlen, salt, SALT_LEN, ITERATIONS, EVP_sha256(), HASH_LEN, hash) != 1) {
        return 0;
    }

    UserRecord* rec = &db->records[idx];
    memcpy(rec->salt, salt, SALT_LEN);
    memcpy(rec->hash, hash, HASH_LEN);
    rec->iterations = ITERATIONS;
    return 1;
}

static int verify(UserDatabase* db, const char* username, const char* password) {
    if (!valid_username(username) || password == NULL) return 0;
    int idx = find_user_index(db, username);
    if (idx < 0) return 0;

    UserRecord* rec = &db->records[idx];
    size_t pwlen = strlen(password);
    if (pwlen > INT_MAX) return 0;

    unsigned char hash[HASH_LEN];
    if (PKCS5_PBKDF2_HMAC(password, (int)pwlen, rec->salt, SALT_LEN, rec->iterations, EVP_sha256(), HASH_LEN, hash) != 1) {
        return 0;
    }
    return CRYPTO_memcmp(hash, rec->hash, HASH_LEN) == 0;
}

int main(void) {
    OpenSSL_add_all_algorithms();
    UserDatabase db;
    memset(&db, 0, sizeof(db));

    int c1 = add_user(&db, "alice", "Start#Pass1234");
    int c2 = add_user(&db, "bob_01", "Init$Secure5678");
    int c3 = add_user(&db, "charlie_2", "My$trongPassw0rd!");
    int c4 = add_user(&db, "dora", "DoraThe#Expl0rer");
    int c5 = add_user(&db, "eve_user", "S3cure&EveUser!");
    printf("Create users: %d\n", (c1 && c2 && c3 && c4 && c5) ? 1 : 0);

    int t1u = update_password(&db, "alice", "NewStrong#Pass1234");
    int t1v = verify(&db, "alice", "NewStrong#Pass1234");
    printf("Test1 update+verify: %d\n", (t1u && t1v) ? 1 : 0);

    int t2u = update_password(&db, "alice", "short");
    printf("Test2 weak password rejected: %d\n", (!t2u) ? 1 : 0);

    int t3u = update_password(&db, "unknown_user", "Valid#Password123");
    printf("Test3 non-existing user: %d\n", (!t3u) ? 1 : 0);

    int t4u = update_password(&db, "bob_01", "Another$trongP4ss!");
    int t4v = verify(&db, "bob_01", "WrongPassword!234");
    printf("Test4 update ok, wrong verify fails: %d\n", (t4u && !t4v) ? 1 : 0);

    char longPass[200];
    longPass[0] = '\0';
    snprintf(longPass, sizeof(longPass), "%s", "A1!");
    for (int i = 0; i < 20; i++) {
        strncat(longPass, "xY9#", sizeof(longPass) - strlen(longPass) - 1);
    }
    strncat(longPass, "Zz@2", sizeof(longPass) - strlen(longPass) - 1);
    int t5u = update_password(&db, "charlie_2", longPass);
    int t5v = verify(&db, "charlie_2", longPass);
    printf("Test5 long password update+verify: %d\n", (t5u && t5v) ? 1 : 0);

    return 0;
}
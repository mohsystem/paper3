#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define ITERATIONS 210000
#define SALT_LEN 16
#define KEY_LEN 32
#define TOKEN_LEN 32
#define TOKEN_TTL 900 /* seconds */

typedef struct {
    char username[33];
    unsigned char salt[SALT_LEN];
    unsigned char pwd_hash[KEY_LEN];
    time_t pwd_changed_at;

    unsigned char token_salt[SALT_LEN];
    unsigned char token_hash[KEY_LEN];
    time_t token_expiry;
    int has_token;

    int used;
} UserRecord;

typedef struct {
    UserRecord users[16];
    size_t count;
} UserDB;

static void init_db(UserDB* db) {
    db->count = 0;
    for (size_t i = 0; i < 16; i++) {
        db->users[i].used = 0;
        db->users[i].has_token = 0;
    }
}

static int validate_username(const char* username) {
    if (!username) return 0;
    size_t len = strlen(username);
    if (len < 3 || len > 32) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!(isalnum((unsigned char)c) || c == '_')) return 0;
    }
    return 1;
}

static int is_strong_password(const char* password, const char* username) {
    if (!password) return 0;
    size_t len = strlen(password);
    if (len < 12 || len > 128) return 0;
    int up=0, lo=0, di=0, sp=0;
    for (size_t i=0;i<len;i++){
        unsigned char c = (unsigned char)password[i];
        if (isupper(c)) up=1;
        else if (islower(c)) lo=1;
        else if (isdigit(c)) di=1;
        else if (!isspace(c)) sp=1;
        if (isspace(c)) return 0;
    }
    if (!(up && lo && di && sp)) return 0;

    if (username && *username) {
        char u[33]; char p[129];
        size_t ul = strlen(username); if (ul > 32) ul = 32;
        size_t pl = len; if (pl > 128) pl = 128;
        for (size_t i=0;i<ul;i++) u[i] = (char)tolower((unsigned char)username[i]);
        u[ul]='\0';
        for (size_t i=0;i<pl;i++) p[i] = (char)tolower((unsigned char)password[i]);
        p[pl]='\0';
        if (strstr(p, u) != NULL) return 0;
    }
    const char* common[] = {"password","123456","qwerty","letmein","admin","welcome","iloveyou","monkey","abc123"};
    for (size_t i=0;i<sizeof(common)/sizeof(common[0]);i++){
        if (strstr(password, common[i]) != NULL) return 0;
    }
    return 1;
}

static int pbkdf2(const unsigned char* input, size_t input_len,
                  const unsigned char* salt, size_t salt_len,
                  unsigned char* out, size_t out_len) {
    if (!input || !salt || !out) return 0;
    if (PKCS5_PBKDF2_HMAC((const char*)input, (int)input_len,
                          salt, (int)salt_len,
                          ITERATIONS, EVP_sha256(),
                          (int)out_len, out) != 1) {
        return 0;
    }
    return 1;
}

static int rand_bytes(unsigned char* buf, size_t len) {
    if (!buf || len == 0) return 0;
    return RAND_bytes(buf, (int)len) == 1;
}

static void to_hex(const unsigned char* in, size_t in_len, char* out_hex, size_t out_len) {
    static const char* HEX = "0123456789abcdef";
    if (!in || !out_hex) return;
    size_t need = in_len * 2 + 1;
    if (out_len < need) return;
    for (size_t i=0;i<in_len;i++){
        out_hex[2*i] = HEX[(in[i] >> 4) & 0xF];
        out_hex[2*i+1] = HEX[in[i] & 0xF];
    }
    out_hex[in_len*2] = '\0';
}

static int from_hex(const char* hex, unsigned char* out, size_t out_len) {
    if (!hex || !out) return 0;
    size_t len = strlen(hex);
    if (len % 2 != 0) return 0;
    if (out_len < len / 2) return 0;
    for (size_t i=0;i<len/2;i++) {
        char c1 = hex[2*i], c2 = hex[2*i+1];
        int hi = (c1 >= '0' && c1 <= '9') ? c1 - '0' :
                 (c1 >= 'a' && c1 <= 'f') ? c1 - 'a' + 10 :
                 (c1 >= 'A' && c1 <= 'F') ? c1 - 'A' + 10 : -1;
        int lo = (c2 >= '0' && c2 <= '9') ? c2 - '0' :
                 (c2 >= 'a' && c2 <= 'f') ? c2 - 'a' + 10 :
                 (c2 >= 'A' && c2 <= 'F') ? c2 - 'A' + 10 : -1;
        if (hi < 0 || lo < 0) return 0;
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    return 1;
}

static UserRecord* find_user(UserDB* db, const char* username) {
    if (!db || !username) return NULL;
    for (size_t i=0;i<db->count;i++){
        if (db->users[i].used && strcmp(db->users[i].username, username) == 0) {
            return &db->users[i];
        }
    }
    return NULL;
}

static int register_user(UserDB* db, const char* username, const char* password) {
    if (!db || !validate_username(username) || !is_strong_password(password, username)) return 0;
    if (find_user(db, username) != NULL) return 0;
    if (db->count >= 16) return 0;

    UserRecord* u = &db->users[db->count];
    memset(u, 0, sizeof(*u));
    strncpy(u->username, username, 32);
    u->username[32] = '\0';
    if (!rand_bytes(u->salt, SALT_LEN)) return 0;
    if (!pbkdf2((const unsigned char*)password, strlen(password), u->salt, SALT_LEN, u->pwd_hash, KEY_LEN)) return 0;
    u->pwd_changed_at = time(NULL);
    u->has_token = 0;
    u->used = 1;
    db->count++;
    return 1;
}

static int authenticate(UserDB* db, const char* username, const char* password) {
    if (!db || !validate_username(username) || !password) return 0;
    UserRecord* u = find_user(db, username);
    if (!u) return 0;
    unsigned char derived[KEY_LEN];
    if (!pbkdf2((const unsigned char*)password, strlen(password), u->salt, SALT_LEN, derived, KEY_LEN)) return 0;
    int eq = (CRYPTO_memcmp(derived, u->pwd_hash, KEY_LEN) == 0);
    OPENSSL_cleanse(derived, KEY_LEN);
    return eq;
}

static int request_password_reset(UserDB* db, const char* username, char* out_token_hex, size_t out_len) {
    if (!db || !validate_username(username) || !out_token_hex) return 0;
    UserRecord* u = find_user(db, username);
    if (!u) {
        if (out_len > 0) out_token_hex[0] = '\0';
        return 0;
    }
    unsigned char token[TOKEN_LEN];
    unsigned char token_salt[SALT_LEN];
    if (!rand_bytes(token, TOKEN_LEN)) return 0;
    if (!rand_bytes(token_salt, SALT_LEN)) return 0;
    unsigned char token_hash[KEY_LEN];
    if (!pbkdf2(token, TOKEN_LEN, token_salt, SALT_LEN, token_hash, KEY_LEN)) return 0;
    memcpy(u->token_salt, token_salt, SALT_LEN);
    memcpy(u->token_hash, token_hash, KEY_LEN);
    u->token_expiry = time(NULL) + TOKEN_TTL;
    u->has_token = 1;
    to_hex(token, TOKEN_LEN, out_token_hex, out_len);
    OPENSSL_cleanse(token, TOKEN_LEN);
    OPENSSL_cleanse(token_hash, KEY_LEN);
    OPENSSL_cleanse(token_salt, SALT_LEN);
    return 1;
}

static int reset_password(UserDB* db, const char* username, const char* token_hex, const char* new_password) {
    if (!db || !validate_username(username) || !token_hex || !new_password) return 0;
    UserRecord* u = find_user(db, username);
    if (!u || !u->has_token) return 0;
    if (time(NULL) > u->token_expiry) return 0;

    unsigned char token[TOKEN_LEN];
    if (!from_hex(token_hex, token, TOKEN_LEN)) return 0;

    unsigned char provided_hash[KEY_LEN];
    if (!pbkdf2(token, TOKEN_LEN, u->token_salt, SALT_LEN, provided_hash, KEY_LEN)) {
        OPENSSL_cleanse(token, TOKEN_LEN);
        return 0;
    }
    int token_ok = (CRYPTO_memcmp(provided_hash, u->token_hash, KEY_LEN) == 0);
    OPENSSL_cleanse(provided_hash, KEY_LEN);
    OPENSSL_cleanse(token, TOKEN_LEN);
    if (!token_ok) return 0;

    if (!is_strong_password(new_password, username)) return 0;

    // Prevent reusing the same password
    unsigned char new_with_old_salt[KEY_LEN];
    if (!pbkdf2((const unsigned char*)new_password, strlen(new_password), u->salt, SALT_LEN, new_with_old_salt, KEY_LEN)) return 0;
    int same_as_old = (CRYPTO_memcmp(new_with_old_salt, u->pwd_hash, KEY_LEN) == 0);
    OPENSSL_cleanse(new_with_old_salt, KEY_LEN);
    if (same_as_old) return 0;

    // Update password with fresh salt
    unsigned char new_salt[SALT_LEN];
    unsigned char new_hash[KEY_LEN];
    if (!rand_bytes(new_salt, SALT_LEN)) return 0;
    if (!pbkdf2((const unsigned char*)new_password, strlen(new_password), new_salt, SALT_LEN, new_hash, KEY_LEN)) return 0;

    memcpy(u->salt, new_salt, SALT_LEN);
    memcpy(u->pwd_hash, new_hash, KEY_LEN);
    u->pwd_changed_at = time(NULL);
    u->has_token = 0;
    u->token_expiry = 0;
    OPENSSL_cleanse(u->token_salt, SALT_LEN);
    OPENSSL_cleanse(u->token_hash, KEY_LEN);

    OPENSSL_cleanse(new_salt, SALT_LEN);
    OPENSSL_cleanse(new_hash, KEY_LEN);
    return 1;
}

int main(void) {
    UserDB db;
    init_db(&db);

    int t1 = register_user(&db, "alice", "Str0ng!Passw0rd");
    int t2 = authenticate(&db, "alice", "Str0ng!Passw0rd");

    char token_hex[TOKEN_LEN * 2 + 1];
    int req_ok = request_password_reset(&db, "alice", token_hex, sizeof(token_hex));
    int t3 = 0;
    if (req_ok) {
        t3 = reset_password(&db, "alice", token_hex, "An0ther!StrongPwd");
        if (t3) {
            t3 = authenticate(&db, "alice", "An0ther!StrongPwd");
        }
    }

    int t4 = reset_password(&db, "alice", "deadbeef", "Yet@N0therStrong1");

    char token_hex2[TOKEN_LEN * 2 + 1];
    int req_ok2 = request_password_reset(&db, "alice", token_hex2, sizeof(token_hex2));
    int t5 = 0;
    if (req_ok2) {
        t5 = reset_password(&db, "alice", token_hex2, "weakpass");
    }

    printf("Test1_RegisterStrongPassword: %s\n", t1 ? "true" : "false");
    printf("Test2_AuthenticateCorrect: %s\n", t2 ? "true" : "false");
    printf("Test3_ResetWithValidTokenAndAuth: %s\n", t3 ? "true" : "false");
    printf("Test4_ResetWithInvalidToken: %s\n", t4 ? "true" : "false");
    printf("Test5_AttemptWeakPasswordReset: %s\n", t5 ? "true" : "false");
    return 0;
}
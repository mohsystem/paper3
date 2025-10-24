#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_USERS 100
#define USERNAME_MAX 33
#define RECORD_MAX 512
#define SALT_LEN 16
#define KEY_LEN 32
#define ITERATIONS 200000

typedef struct {
    char username[USERNAME_MAX];
    char record[RECORD_MAX]; // "v1$ITER$SALT_B64$HASH_B64"
} UserEntry;

typedef struct {
    UserEntry users[MAX_USERS];
    size_t count;
} SecureUserStoreC;

static int validate_username(const char* username) {
    if (username == NULL) return 0;
    size_t len = strlen(username);
    if (len < 3 || len > 32) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)username[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int validate_password(const char* password) {
    if (password == NULL) return 0;
    size_t len = strlen(password);
    if (len < 10) return 0;
    int hasU=0, hasL=0, hasD=0, hasS=0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)password[i];
        if (isspace(c)) return 0;
        if (isupper(c)) hasU = 1;
        else if (islower(c)) hasL = 1;
        else if (isdigit(c)) hasD = 1;
        else hasS = 1;
    }
    return hasU && hasL && hasD && hasS;
}

static int find_user_index(const SecureUserStoreC* store, const char* username) {
    for (size_t i = 0; i < store->count; ++i) {
        if (strncmp(store->users[i].username, username, USERNAME_MAX) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static int b64_encode(const unsigned char* in, int in_len, char* out, int out_len) {
    int need = 4 * ((in_len + 2) / 3);
    if (out_len < need + 1) return -1;
    int actual = EVP_EncodeBlock((unsigned char*)out, in, in_len);
    out[actual] = '\0';
    return actual;
}

static int b64_decode(const char* in, unsigned char* out, int out_len) {
    int in_len = (int)strlen(in);
    int max_dec = 3 * (in_len / 4) + 3;
    if (out_len < max_dec) return -1;
    int actual = EVP_DecodeBlock(out, (const unsigned char*)in, in_len);
    if (actual < 0) return -1;
    int pad = 0;
    if (in_len > 0 && in[in_len-1] == '=') pad++;
    if (in_len > 1 && in[in_len-2] == '=') pad++;
    actual -= pad;
    return actual;
}

static int pbkdf2_sha256(const char* password, const unsigned char* salt, int salt_len, int iterations, unsigned char* out, int out_len) {
    return PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                             salt, salt_len, iterations, EVP_sha256(),
                             out_len, out) == 1;
}

int signup(SecureUserStoreC* store, const char* username, const char* password) {
    if (!store || !validate_username(username) || !validate_password(password)) return 0;
    if (find_user_index(store, username) != -1) return 0;
    if (store->count >= MAX_USERS) return 0;

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) return 0;

    unsigned char dk[KEY_LEN];
    if (!pbkdf2_sha256(password, salt, SALT_LEN, ITERATIONS, dk, KEY_LEN)) return 0;

    char salt_b64[4 * ((SALT_LEN + 2) / 3) + 1];
    char dk_b64[4 * ((KEY_LEN + 2) / 3) + 1];

    if (b64_encode(salt, SALT_LEN, salt_b64, sizeof(salt_b64)) < 0) return 0;
    if (b64_encode(dk, KEY_LEN, dk_b64, sizeof(dk_b64)) < 0) return 0;

    UserEntry* e = &store->users[store->count];
    snprintf(e->username, USERNAME_MAX, "%s", username);
    snprintf(e->record, RECORD_MAX, "v1$%d$%s$%s", ITERATIONS, salt_b64, dk_b64);
    store->count++;
    return 1;
}

static int constant_time_eq(const unsigned char* a, const unsigned char* b, size_t len) {
    unsigned int diff = 0;
    for (size_t i = 0; i < len; ++i) diff |= (a[i] ^ b[i]);
    return diff == 0;
}

int verify(SecureUserStoreC* store, const char* username, const char* password) {
    if (!store || !username || !password) return 0;
    int idx = find_user_index(store, username);
    if (idx < 0) return 0;

    char* rec = store->users[idx].record;
    // split by $
    char* parts[4] = {0};
    int part_idx = 0;
    char buf[RECORD_MAX];
    snprintf(buf, sizeof(buf), "%s", rec);
    char* token = strtok(buf, "$");
    while (token && part_idx < 4) {
        parts[part_idx++] = token;
        token = strtok(NULL, "$");
    }
    if (part_idx != 4) return 0;
    if (strcmp(parts[0], "v1") != 0) return 0;

    int iters = atoi(parts[1]);

    unsigned char salt_dec[64];
    int salt_len = b64_decode(parts[2], salt_dec, (int)sizeof(salt_dec));
    if (salt_len <= 0) return 0;

    unsigned char expected_dec[128];
    int exp_len = b64_decode(parts[3], expected_dec, (int)sizeof(expected_dec));
    if (exp_len <= 0) return 0;

    unsigned char actual[128];
    if (!pbkdf2_sha256(password, salt_dec, salt_len, iters, actual, exp_len)) return 0;

    int ok = constant_time_eq(expected_dec, actual, (size_t)exp_len);
    // Securely zero sensitive buffers
    OPENSSL_cleanse(actual, sizeof(actual));
    return ok;
}

const char* get_record(SecureUserStoreC* store, const char* username) {
    int idx = find_user_index(store, username);
    if (idx < 0) return NULL;
    return store->users[idx].record;
}

int main(void) {
    SecureUserStoreC store;
    memset(&store, 0, sizeof(store));

    printf("Test1 (valid signup): %s\n", signup(&store, "alice", "Str0ngPass!") ? "true" : "false");
    printf("Test2 (duplicate username): %s\n", signup(&store, "alice", "An0ther#Pass") ? "true" : "false");
    printf("Test3 (invalid username): %s\n", signup(&store, "bob space", "Str0ngPass!") ? "true" : "false");
    printf("Test4 (weak password): %s\n", signup(&store, "carol", "weak") ? "true" : "false");
    printf("Test5 (another valid signup): %s\n", signup(&store, "charlie", "G00d#Password") ? "true" : "false");

    printf("Verify alice: %s\n", verify(&store, "alice", "Str0ngPass!") ? "true" : "false");
    printf("Verify alice wrong: %s\n", verify(&store, "alice", "WrongPass!") ? "true" : "false");

    return 0;
}
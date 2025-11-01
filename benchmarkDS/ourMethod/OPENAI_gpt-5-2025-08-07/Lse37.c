#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#ifdef _MSC_VER
#include <windows.h>
#define secure_zero(buf, len) SecureZeroMemory((buf), (len))
#else
#if defined(__STDC_LIB_EXT1__)
#define secure_zero(buf, len) memset_s((buf), (len), 0, (len))
#else
static void secure_zero(void* buf, size_t len) {
    volatile unsigned char* p = (volatile unsigned char*)buf;
    while (len--) *p++ = 0;
}
#endif
#endif

#define MAX_USERS 32
#define MAX_USERNAME 64
#define MIN_PASSWORD 8
#define MAX_PASSWORD 128
#define SALT_LEN 16
#define HASH_LEN 32
#define ITERATIONS 210000

typedef struct {
    char username[MAX_USERNAME + 1];
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    int iterations;
    int in_use;
} UserRecord;

typedef struct {
    UserRecord users[MAX_USERS];
} AuthService;

static int validate_username(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len < 1 || len > MAX_USERNAME) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '.' || c == '@' || c == '-')) {
            return 0;
        }
    }
    return 1;
}

static int validate_password(const char* p) {
    if (p == NULL) return 0;
    size_t len = strlen(p);
    if (len < MIN_PASSWORD || len > MAX_PASSWORD) return 0;
    int hasU=0, hasL=0, hasD=0, hasS=0;
    const char* specials = "!@#$%^&*()-_=+[]{};:,.?/~";
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)p[i];
        if (isupper(c)) hasU = 1;
        else if (islower(c)) hasL = 1;
        else if (isdigit(c)) hasD = 1;
        if (strchr(specials, (int)c) != NULL) hasS = 1;
    }
    return hasU && hasL && hasD && hasS;
}

static int derive_key(const char* password, const unsigned char* salt, int iterations, unsigned char* out, int out_len) {
    if (!password || !salt || !out) return 0;
    const EVP_MD* md = EVP_sha256();
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LEN, iterations, md, out_len, out) != 1) {
        return 0;
    }
    return 1;
}

static void auth_init(AuthService* svc) {
    if (!svc) return;
    memset(svc, 0, sizeof(*svc));
}

static int register_user(AuthService* svc, const char* username, const char* password) {
    if (!svc) return 0;
    if (!validate_username(username) || !validate_password(password)) return 0;
    // Check duplicates
    for (int i = 0; i < MAX_USERS; i++) {
        if (svc->users[i].in_use && strncmp(svc->users[i].username, username, MAX_USERNAME) == 0) {
            return 0;
        }
    }
    // Find slot
    int idx = -1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (!svc->users[i].in_use) { idx = i; break; }
    }
    if (idx < 0) return 0;

    UserRecord* rec = &svc->users[idx];
    if (RAND_bytes(rec->salt, SALT_LEN) != 1) {
        return 0;
    }
    unsigned char hash[HASH_LEN];
    if (!derive_key(password, rec->salt, ITERATIONS, hash, HASH_LEN)) {
        return 0;
    }
    // Store
    strncpy(rec->username, username, MAX_USERNAME);
    rec->username[MAX_USERNAME] = '\0';
    memcpy(rec->hash, hash, HASH_LEN);
    rec->iterations = ITERATIONS;
    rec->in_use = 1;
    secure_zero(hash, sizeof(hash));
    return 1;
}

static const char* authenticate(AuthService* svc, const char* username, const char* password) {
    if (!svc || !username || !password) return "cant_login";
    if (!validate_username(username) || strlen(password) < 1 || strlen(password) > MAX_PASSWORD) return "cant_login";
    // Find user
    const UserRecord* rec = NULL;
    for (int i = 0; i < MAX_USERS; i++) {
        if (svc->users[i].in_use && strncmp(svc->users[i].username, username, MAX_USERNAME) == 0) {
            rec = &svc->users[i];
            break;
        }
    }
    if (!rec) return "cant_login";
    if (rec->iterations < 1) return "cant_login";
    if (memcmp(rec->salt, rec->salt, SALT_LEN) != 0) { /* no-op to reference salt and avoid warnings */ }

    unsigned char cand[HASH_LEN];
    if (!derive_key(password, rec->salt, rec->iterations, cand, HASH_LEN)) {
        secure_zero(cand, sizeof(cand));
        return "internal_server_error";
    }
    int diff = CRYPTO_memcmp(cand, rec->hash, HASH_LEN);
    secure_zero(cand, sizeof(cand));
    return (diff == 0) ? "success" : "cant_login";
}

// Helper: corrupt a user's record to simulate internal error or invalid state
static void corrupt_user_record(AuthService* svc, const char* username) {
    if (!svc || !username) return;
    for (int i = 0; i < MAX_USERS; i++) {
        if (svc->users[i].in_use && strncmp(svc->users[i].username, username, MAX_USERNAME) == 0) {
            // Set iterations to 0 to simulate invalid configuration
            svc->users[i].iterations = 0;
            break;
        }
    }
}

static int rand_index(size_t bound) {
    if (bound <= 1) return 0;
    unsigned char buf[4];
    if (RAND_bytes(buf, sizeof(buf)) != 1) return 0;
    unsigned int val = ((unsigned int)buf[0] << 24) | ((unsigned int)buf[1] << 16) |
                       ((unsigned int)buf[2] << 8) | ((unsigned int)buf[3]);
    return (int)(val % bound);
}

static void shuffle_chars(char* arr, size_t n) {
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = (size_t)rand_index(i + 1);
        char tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

static void generate_strong_password(char* out, size_t out_sz, size_t length) {
    if (!out || out_sz == 0) return;
    if (length < 12) length = 12;
    const char* U = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char* L = "abcdefghijklmnopqrstuvwxyz";
    const char* D = "0123456789";
    const char* S = "!@#$%^&*()-_=+[]{};:,.?/~";
    const char* ALL = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_=+[]{};:,.?/~";

    if (length + 1 > out_sz) length = out_sz - 1;

    char* pwd = (char*)malloc(length);
    if (!pwd) {
        if (out_sz > 0) out[0] = '\0';
        return;
    }
    pwd[0] = U[rand_index(strlen(U))];
    pwd[1] = L[rand_index(strlen(L))];
    pwd[2] = D[rand_index(strlen(D))];
    pwd[3] = S[rand_index(strlen(S))];
    for (size_t i = 4; i < length; i++) {
        pwd[i] = ALL[rand_index(strlen(ALL))];
    }
    shuffle_chars(pwd, length);
    memcpy(out, pwd, length);
    out[length] = '\0';
    free(pwd);
}

int main(void) {
    AuthService auth;
    auth_init(&auth);

    const char* user1 = "alice@example.com";
    const char* user2 = "bob_user-1";

    char pass1[64];
    char pass2[64];
    generate_strong_password(pass1, sizeof(pass1), 14);
    generate_strong_password(pass2, sizeof(pass2), 16);

    int r1 = register_user(&auth, user1, pass1);
    int r2 = register_user(&auth, user2, pass2);

    // Test 1: correct login
    const char* t1 = authenticate(&auth, user1, pass1);
    // Test 2: wrong password (mutate first char)
    char wrong[64]; strncpy(wrong, pass1, sizeof(wrong) - 1); wrong[sizeof(wrong) - 1] = '\0';
    if (strlen(wrong) > 0) wrong[0] = (wrong[0] == 'A') ? 'B' : 'A';
    const char* t2 = authenticate(&auth, user1, wrong);
    // Test 3: non-existent user
    char random_pw[64]; generate_strong_password(random_pw, sizeof(random_pw), 12);
    const char* t3 = authenticate(&auth, "charlie", random_pw);
    // Test 4: invalid username format
    const char* t4 = authenticate(&auth, "invalid user name with spaces", random_pw);
    // Test 5: simulate internal error by corrupting record
    corrupt_user_record(&auth, user2);
    const char* t5 = authenticate(&auth, user2, pass2);

    printf("Test1: %s\n", r1 ? t1 : "internal_server_error");
    printf("Test2: %s\n", t2);
    printf("Test3: %s\n", t3);
    printf("Test4: %s\n", t4);
    printf("Test5: %s\n", r2 ? t5 : "internal_server_error");

    secure_zero(pass1, sizeof(pass1));
    secure_zero(pass2, sizeof(pass2));
    secure_zero(wrong, sizeof(wrong));
    secure_zero(random_pw, sizeof(random_pw));
    return 0;
}
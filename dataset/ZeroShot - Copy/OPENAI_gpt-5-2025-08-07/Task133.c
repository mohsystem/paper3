// Requires OpenSSL development libraries for compilation and linking.
// Example: gcc -std=c11 task.c -lssl -lcrypto
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

#define TOKEN_TTL_SECONDS (15 * 60)
#define PBKDF2_ITERATIONS 150000
#define SALT_LEN 16
#define DK_LEN 32
#define MAX_USERS 32

typedef struct {
    char* email;
    char* pass_hash;        // format: pbkdf2$sha256$iter$saltHex$hashHex
    char* token_hash_hex;   // sha256 hex of token
    time_t token_expiry;
    int in_use;
} User;

typedef struct {
    User users[MAX_USERS];
} Task133;

static char* str_to_lower(const char* s) {
    size_t n = strlen(s);
    char* out = (char*)malloc(n+1);
    if (!out) return NULL;
    for (size_t i=0;i<n;i++) out[i] = (char)tolower((unsigned char)s[i]);
    out[n] = '\0';
    return out;
}

static int is_common_password(const char* lw) {
    const char* commons[] = {
        "password","123456","123456789","qwerty","111111","12345678",
        "abc123","password1","1234567","12345","letmein","admin",
        "welcome","monkey","login"
    };
    size_t cnt = sizeof(commons)/sizeof(commons[0]);
    for (size_t i=0;i<cnt;i++) {
        if (strcmp(lw, commons[i]) == 0) return 1;
    }
    return 0;
}

static int is_strong_password(const char* pw, const char* email) {
    if (!pw) return 0;
    size_t n = strlen(pw);
    if (n < 12) return 0;
    int up=0, lo=0, di=0, sy=0;
    for (size_t i=0;i<n;i++) {
        unsigned char c = (unsigned char)pw[i];
        if (isupper(c)) up=1;
        else if (islower(c)) lo=1;
        else if (isdigit(c)) di=1;
        else sy=1;
    }
    if (!(up && lo && di && sy)) return 0;
    char* lw = str_to_lower(pw);
    if (!lw) return 0;
    int res = 1;
    if (is_common_password(lw)) { res = 0; goto out; }
    if (email) {
        const char* at = strchr(email, '@');
        if (at) {
            size_t userlen = (size_t)(at - email);
            if (userlen > 0) {
                char* user = (char*)malloc(userlen+1);
                if (!user) { res = 0; goto out; }
                memcpy(user, email, userlen);
                user[userlen] = '\0';
                for (size_t i=0;i<userlen;i++) user[i] = (char)tolower((unsigned char)user[i]);
                if (strstr(lw, user) != NULL) { res = 0; }
                free(user);
            }
        }
    }
out:
    free(lw);
    return res;
}

static char* hex_encode(const unsigned char* data, size_t len) {
    static const char* hexd = "0123456789abcdef";
    char* out = (char*)malloc(len*2 + 1);
    if (!out) return NULL;
    for (size_t i=0;i<len;i++) {
        out[2*i] = hexd[(data[i] >> 4) & 0xF];
        out[2*i+1] = hexd[data[i] & 0xF];
    }
    out[len*2] = '\0';
    return out;
}

static unsigned char* hex_decode(const char* hex, size_t* out_len) {
    size_t n = strlen(hex);
    if (n % 2 != 0) return NULL;
    unsigned char* out = (unsigned char*)malloc(n/2);
    if (!out) return NULL;
    for (size_t i=0;i<n/2;i++) {
        unsigned int v;
        if (sscanf(hex + 2*i, "%02x", &v) != 1) { free(out); return NULL; }
        out[i] = (unsigned char)v;
    }
    *out_len = n/2;
    return out;
}

static char* sha256_hex(const char* s) {
    unsigned char dig[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, s, strlen(s));
    SHA256_Final(dig, &ctx);
    return hex_encode(dig, sizeof(dig));
}

static int constant_time_eq_hex(const char* a, const char* b) {
    size_t na = strlen(a), nb = strlen(b);
    if (na != nb) return 0;
    return CRYPTO_memcmp(a, b, na) == 0;
}

static char* hash_password(const char* pw) {
    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, sizeof(salt)) != 1) return NULL;
    unsigned char dk[DK_LEN];
    if (PKCS5_PBKDF2_HMAC(pw, (int)strlen(pw), salt, sizeof(salt),
                          PBKDF2_ITERATIONS, EVP_sha256(), DK_LEN, dk) != 1) return NULL;
    char* saltHex = hex_encode(salt, sizeof(salt));
    char* hashHex = hex_encode(dk, sizeof(dk));
    if (!saltHex || !hashHex) { free(saltHex); free(hashHex); return NULL; }
    // pbkdf2$sha256$iter$saltHex$hashHex
    char iterStr[32];
    snprintf(iterStr, sizeof(iterStr), "%d", PBKDF2_ITERATIONS);
    size_t total = strlen("pbkdf2$sha256$") + strlen(iterStr) + 1 + strlen(saltHex) + 1 + strlen(hashHex) + 1;
    char* out = (char*)malloc(total);
    if (!out) { free(saltHex); free(hashHex); return NULL; }
    snprintf(out, total, "pbkdf2$sha256$%s$%s$%s", iterStr, saltHex, hashHex);
    free(saltHex);
    free(hashHex);
    return out;
}

static int verify_password(const char* pw, const char* stored) {
    // parse format
    const char* p1 = strchr(stored, '$'); if (!p1) return 0;
    const char* p2 = strchr(p1+1, '$'); if (!p2) return 0;
    const char* p3 = strchr(p2+1, '$'); if (!p3) return 0;
    const char* p4 = strchr(p3+1, '$'); if (!p4) return 0;

    size_t aLen = (size_t)(p1 - stored);
    size_t hLen = (size_t)(p2 - (p1+1));
    char a[16], h[16];
    if (aLen >= sizeof(a) || hLen >= sizeof(h)) return 0;
    memcpy(a, stored, aLen); a[aLen] = '\0';
    memcpy(h, p1+1, hLen); h[hLen] = '\0';
    if (strcmp(a, "pbkdf2") != 0 || strcmp(h, "sha256") != 0) return 0;

    char iterStr[32];
    size_t iterLen = (size_t)(p3 - (p2+1));
    if (iterLen >= sizeof(iterStr)) return 0;
    memcpy(iterStr, p2+1, iterLen); iterStr[iterLen] = '\0';
    int iter = atoi(iterStr);

    size_t saltHexLen = (size_t)(p4 - (p3+1));
    char* saltHex = (char*)malloc(saltHexLen + 1);
    if (!saltHex) return 0;
    memcpy(saltHex, p3+1, saltHexLen); saltHex[saltHexLen] = '\0';

    const char* hashHex = p4+1;

    size_t saltLen=0, expLen=0;
    unsigned char* salt = hex_decode(saltHex, &saltLen);
    unsigned char* expected = hex_decode(hashHex, &expLen);
    free(saltHex);
    if (!salt || !expected) { free(salt); free(expected); return 0; }

    unsigned char* got = (unsigned char*)malloc(expLen);
    if (!got) { free(salt); free(expected); return 0; }
    if (PKCS5_PBKDF2_HMAC(pw, (int)strlen(pw), salt, (int)saltLen,
                          iter, EVP_sha256(), (int)expLen, got) != 1) {
        free(salt); free(expected); free(got); return 0;
    }
    int ok = (CRYPTO_memcmp(expected, got, expLen) == 0);
    free(salt); free(expected); free(got);
    return ok;
}

static int find_user_index(Task133* t, const char* email) {
    for (int i=0;i<MAX_USERS;i++) {
        if (t->users[i].in_use && strcmp(t->users[i].email, email) == 0) return i;
    }
    return -1;
}

static int register_user(Task133* t, const char* email, const char* password) {
    if (!email || !password) return 0;
    char* el = str_to_lower(email);
    if (!el) return 0;
    if (!is_strong_password(password, el)) { free(el); return 0; }
    if (find_user_index(t, el) >= 0) { free(el); return 0; }
    int idx = -1;
    for (int i=0;i<MAX_USERS;i++) {
        if (!t->users[i].in_use) { idx = i; break; }
    }
    if (idx < 0) { free(el); return 0; }
    char* ph = hash_password(password);
    if (!ph) { free(el); return 0; }
    t->users[idx].email = el;
    t->users[idx].pass_hash = ph;
    t->users[idx].token_hash_hex = NULL;
    t->users[idx].token_expiry = 0;
    t->users[idx].in_use = 1;
    return 1;
}

static char* generate_token() {
    unsigned char buf[32];
    if (RAND_bytes(buf, sizeof(buf)) != 1) return NULL;
    return hex_encode(buf, sizeof(buf));
}

static char* request_password_reset(Task133* t, const char* email) {
    if (!email) return NULL;
    char* el = str_to_lower(email);
    if (!el) return NULL;
    char* token = generate_token();
    if (!token) { free(el); return NULL; }
    char* token_hash = sha256_hex(token);
    time_t expiry = time(NULL) + TOKEN_TTL_SECONDS;

    int idx = find_user_index(t, el);
    if (idx >= 0) {
        if (t->users[idx].token_hash_hex) free(t->users[idx].token_hash_hex);
        t->users[idx].token_hash_hex = token_hash;
        t->users[idx].token_expiry = expiry;
    } else {
        free(token_hash);
    }
    free(el);
    return token; // caller must free
}

static int reset_password(Task133* t, const char* email, const char* token, const char* new_password) {
    if (!email || !token || !new_password) return 0;
    char* el = str_to_lower(email);
    if (!el) return 0;
    int idx = find_user_index(t, el);
    free(el);
    if (idx < 0) return 0;
    User* u = &t->users[idx];
    time_t now = time(NULL);
    if (!u->token_hash_hex || now > u->token_expiry) return 0;
    char* prov_hash = sha256_hex(token);
    if (!prov_hash) return 0;
    int ok = constant_time_eq_hex(u->token_hash_hex, prov_hash);
    free(prov_hash);
    if (!ok) return 0;
    if (!is_strong_password(new_password, u->email)) return 0;
    if (verify_password(new_password, u->pass_hash)) return 0;
    char* ph = hash_password(new_password);
    if (!ph) return 0;
    free(u->pass_hash);
    u->pass_hash = ph;
    free(u->token_hash_hex);
    u->token_hash_hex = NULL;
    u->token_expiry = 0;
    return 1;
}

static int verify_login(Task133* t, const char* email, const char* password) {
    char* el = str_to_lower(email);
    if (!el) return 0;
    int idx = find_user_index(t, el);
    free(el);
    if (idx < 0) return 0;
    return verify_password(password, t->users[idx].pass_hash);
}

int main(void) {
    Task133 t = {0};
    printf("Register Alice: %d\n", register_user(&t, "alice@example.com", "InitialPassw0rd!"));
    printf("Register Bob:   %d\n", register_user(&t, "bob@example.com", "Secur3P@ssw0rd!"));
    printf("Register Charlie: %d\n", register_user(&t, "charlie@example.com", "Another$tr0ng1"));
    printf("Register Dave:  %d\n", register_user(&t, "dave@example.com", "Y3t@Str0ngPass!"));
    printf("Register Eve:   %d\n", register_user(&t, "eve@example.com", "InitialPassw0rd!"));

    // 1) Successful reset
    char* tok1 = request_password_reset(&t, "alice@example.com");
    int r1 = reset_password(&t, "alice@example.com", tok1, "NewStrongP@ssw0rd!");
    printf("Test1 success reset: %d | login new: %d\n", r1, verify_login(&t, "alice@example.com", "NewStrongP@ssw0rd!"));
    free(tok1);

    // 2) Expired token
    char* tok2 = request_password_reset(&t, "bob@example.com");
    // Simulate expiry
    int bobIdx = find_user_index(&t, "bob@example.com");
    if (bobIdx >= 0) t.users[bobIdx].token_expiry = time(NULL) - 1;
    int r2 = reset_password(&t, "bob@example.com", tok2, "AnotherN3wP@ss!");
    printf("Test2 expired token result: %d\n", r2);
    free(tok2);

    // 3) Weak password attempt
    char* tok3 = request_password_reset(&t, "charlie@example.com");
    int r3 = reset_password(&t, "charlie@example.com", tok3, "password");
    printf("Test3 weak password result: %d\n", r3);
    free(tok3);

    // 4) Invalid token
    (void)request_password_reset(&t, "dave@example.com");
    int r4 = reset_password(&t, "dave@example.com", "INVALID_TOKEN", "Th!sIsStr0ngP@ss");
    printf("Test4 invalid token result: %d\n", r4);

    // 5) Reuse old password not allowed
    char* tok5 = request_password_reset(&t, "eve@example.com");
    int r5 = reset_password(&t, "eve@example.com", tok5, "InitialPassw0rd!");
    printf("Test5 reuse old password result: %d\n", r5);
    free(tok5);

    // Free allocated user data
    for (int i=0;i<MAX_USERS;i++) {
        if (t.users[i].in_use) {
            free(t.users[i].email);
            free(t.users[i].pass_hash);
            if (t.users[i].token_hash_hex) free(t.users[i].token_hash_hex);
        }
    }
    return 0;
}
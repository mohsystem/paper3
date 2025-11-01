/* Chain-of-Through step 1: Problem understanding
   Create a function to register a user by hashing the password securely and returning salt/hash.
   Step 2: Security requirements
   - Validate input; use OpenSSL PBKDF2 with SHA-256, strong iterations, random salt.
   Step 3: Secure coding generation (below)
   Step 4: Code review
   Step 5: Secure code output */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

typedef struct {
    char username[64];
    char algorithm[32];
    int iterations;
    char salt_hex[16 * 2 + 1];
    char hash_hex[32 * 2 + 1];
    int ok; /* 1 success, 0 failure */
    char error[128];
} RegistrationResult;

static int validate_username(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len < 3 || len > 32) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '.' || c == '-')) {
            return 0;
        }
    }
    return 1;
}

static int validate_password(const char* p) {
    if (p == NULL) return 0;
    size_t len = strlen(p);
    if (len < 12) return 0;
    int up=0, lo=0, di=0, sp=0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)p[i];
        if (isupper(c)) up = 1;
        else if (islower(c)) lo = 1;
        else if (isdigit(c)) di = 1;
        else sp = 1;
    }
    return up && lo && di && sp;
}

static void to_hex(const unsigned char* data, size_t len, char* out_hex) {
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < len; ++i) {
        out_hex[2*i] = hex[(data[i] >> 4) & 0xF];
        out_hex[2*i + 1] = hex[data[i] & 0xF];
    }
    out_hex[2*len] = '\0';
}

RegistrationResult register_user(const char* username, const char* password) {
    RegistrationResult res;
    memset(&res, 0, sizeof(res));

    if (!validate_username(username)) {
        snprintf(res.error, sizeof(res.error), "Invalid username. Use 3-32 chars: letters, digits, _ . -");
        res.ok = 0;
        return res;
    }
    if (!validate_password(password)) {
        snprintf(res.error, sizeof(res.error), "Weak password. Min 12 chars with upper, lower, digit, special.");
        res.ok = 0;
        return res;
    }

    const int salt_len = 16;
    const int iterations = 200000;
    const int dk_len = 32;

    unsigned char salt[salt_len];
    if (RAND_bytes(salt, salt_len) != 1) {
        snprintf(res.error, sizeof(res.error), "Failed to generate salt");
        res.ok = 0;
        return res;
    }

    unsigned char dk[dk_len];
    /* Copy password to mutable buffer for potential wiping */
    size_t pwlen = strlen(password);
    unsigned char* pwbuf = (unsigned char*)malloc(pwlen);
    if (!pwbuf) {
        snprintf(res.error, sizeof(res.error), "Memory allocation failed");
        res.ok = 0;
        return res;
    }
    memcpy(pwbuf, password, pwlen);

    int ok = PKCS5_PBKDF2_HMAC((const char*)pwbuf, (int)pwlen,
                               salt, salt_len, iterations,
                               EVP_sha256(), dk_len, dk);
    /* wipe pwbuf */
    memset(pwbuf, 0, pwlen);
    free(pwbuf);

    if (ok != 1) {
        snprintf(res.error, sizeof(res.error), "PBKDF2 failed");
        res.ok = 0;
        return res;
    }

    strncpy(res.username, username, sizeof(res.username)-1);
    strncpy(res.algorithm, "PBKDF2-HMAC-SHA256", sizeof(res.algorithm)-1);
    res.iterations = iterations;
    to_hex(salt, salt_len, res.salt_hex);
    to_hex(dk, dk_len, res.hash_hex);
    /* wipe dk */
    memset(dk, 0, dk_len);
    memset(salt, 0, salt_len);

    res.ok = 1;
    return res;
}

int main(void) {
    const char* users[5] = {"alice_1", "Bob-User", "charlie.user", "dave_2025", "eve.test"};
    const char* passes[5] = {"StrongPassw0rd!", "Another$tr0ngPass!", "Y3tAn0ther#Pass!", "S0methingComp!ex", "Unbr3akable@Key!"};

    for (int i = 0; i < 5; ++i) {
        RegistrationResult r = register_user(users[i], passes[i]);
        if (r.ok) {
            printf("{\"username\":\"%s\",\"algorithm\":\"%s\",\"iterations\":%d,\"salt_hex\":\"%s\",\"hash_hex\":\"%s\"}\n",
                   r.username, r.algorithm, r.iterations, r.salt_hex, r.hash_hex);
        } else {
            printf("{\"error\":\"%s\"}\n", r.error);
        }
    }
    return 0;
}
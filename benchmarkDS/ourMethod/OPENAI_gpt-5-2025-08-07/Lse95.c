#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SALT_LEN 16
#define KEY_LEN 32
#define ITERATIONS 210000
#define MAX_USERS 100
#define USERNAME_MAX 32

// stored format: hex(hash)[64] + hex(salt)[32] -> 96 chars + null
#define STORED_MAX (KEY_LEN*2 + SALT_LEN*2)

typedef struct {
    char username[USERNAME_MAX + 1];
    char stored[STORED_MAX + 1]; // hex(hash)+hex(salt)
    int in_use;
} UserEntry;

static UserEntry DB[MAX_USERS];

static int is_valid_username(const char* username) {
    if (username == NULL) return 0;
    size_t len = strlen(username);
    if (len < 3 || len > USERNAME_MAX) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!(isalnum((unsigned char)c) || c == '_')) return 0;
    }
    return 1;
}

static int is_strong_password(const char* password) {
    if (password == NULL) return 0;
    size_t len = strlen(password);
    if (len < 12) return 0;
    int has_upper=0, has_lower=0, has_digit=0, has_special=0;
    const char* specials = "!@#$%^&*()-_=+[]{}|;:',.<>/?`~\"\\ ";
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (isupper(c)) has_upper = 1;
        else if (islower(c)) has_lower = 1;
        else if (isdigit(c)) has_digit = 1;
        else if (strchr(specials, c) != NULL) has_special = 1;
    }
    return has_upper && has_lower && has_digit && has_special;
}

static void to_hex(const unsigned char* in, size_t inlen, char* out, size_t outlen) {
    static const char* HEX = "0123456789abcdef";
    if (outlen < inlen*2 + 1) return;
    for (size_t i = 0; i < inlen; i++) {
        out[2*i]   = HEX[(in[i] >> 4) & 0xF];
        out[2*i+1] = HEX[in[i] & 0xF];
    }
    out[inlen*2] = '\0';
}

static int from_hex(const char* hex, unsigned char* out, size_t outlen) {
    size_t len = strlen(hex);
    if (len % 2 != 0 || outlen < len/2) return 0;
    for (size_t i = 0; i < len/2; i++) {
        char h = hex[2*i], l = hex[2*i+1];
        int hi = (h >= '0' && h <= '9') ? h - '0' :
                 (h >= 'a' && h <= 'f') ? h - 'a' + 10 :
                 (h >= 'A' && h <= 'F') ? h - 'A' + 10 : -1;
        int lo = (l >= '0' && l <= '9') ? l - '0' :
                 (l >= 'a' && l <= 'f') ? l - 'a' + 10 :
                 (l >= 'A' && l <= 'F') ? l - 'A' + 10 : -1;
        if (hi < 0 || lo < 0) return 0;
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    return 1;
}

static int find_user_index(const char* username) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (DB[i].in_use && strncmp(DB[i].username, username, USERNAME_MAX) == 0) {
            return i;
        }
    }
    return -1;
}

int register_user(const char* username, const char* password) {
    if (!is_valid_username(username) || !is_strong_password(password)) {
        return 0;
    }
    if (find_user_index(username) >= 0) {
        return 0;
    }
    int slot = -1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (!DB[i].in_use) { slot = i; break; }
    }
    if (slot < 0) return 0;

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) return 0;

    unsigned char key[KEY_LEN];
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LEN, ITERATIONS, EVP_sha256(), KEY_LEN, key) != 1) {
        OPENSSL_cleanse(salt, sizeof(salt));
        return 0;
    }

    char hash_hex[KEY_LEN*2 + 1];
    char salt_hex[SALT_LEN*2 + 1];
    to_hex(key, KEY_LEN, hash_hex, sizeof(hash_hex));
    to_hex(salt, SALT_LEN, salt_hex, sizeof(salt_hex));

    // store appended: hex(hash) + hex(salt)
    char stored[STORED_MAX + 1];
    if (snprintf(stored, sizeof(stored), "%s%s", hash_hex, salt_hex) >= (int)sizeof(stored)) {
        OPENSSL_cleanse(salt, sizeof(salt));
        OPENSSL_cleanse(key, sizeof(key));
        return 0;
    }

    strncpy(DB[slot].username, username, USERNAME_MAX);
    DB[slot].username[USERNAME_MAX] = '\0';
    strncpy(DB[slot].stored, stored, STORED_MAX);
    DB[slot].stored[STORED_MAX] = '\0';
    DB[slot].in_use = 1;

    OPENSSL_cleanse(salt, sizeof(salt));
    OPENSSL_cleanse(key, sizeof(key));
    OPENSSL_cleanse(hash_hex, sizeof(hash_hex));
    OPENSSL_cleanse(salt_hex, sizeof(salt_hex));
    OPENSSL_cleanse(stored, sizeof(stored));
    return 1;
}

int verify_login(const char* username, const char* password) {
    int idx = find_user_index(username);
    if (idx < 0) return 0;
    const char* stored = DB[idx].stored;
    size_t slen = strlen(stored);
    if (slen < SALT_LEN*2) return 0;

    const char* salt_hex = stored + (slen - SALT_LEN*2);
    char hash_hex[KEY_LEN*2 + 1];
    size_t hlen = slen - SALT_LEN*2;
    if (hlen != KEY_LEN*2 || hlen >= sizeof(hash_hex)) return 0;
    memcpy(hash_hex, stored, hlen);
    hash_hex[hlen] = '\0';

    unsigned char salt[SALT_LEN];
    unsigned char expected[KEY_LEN];
    if (!from_hex(salt_hex, salt, sizeof(salt)) || !from_hex(hash_hex, expected, sizeof(expected))) {
        OPENSSL_cleanse(salt, sizeof(salt));
        OPENSSL_cleanse(expected, sizeof(expected));
        return 0;
    }

    unsigned char key[KEY_LEN];
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LEN, ITERATIONS, EVP_sha256(), KEY_LEN, key) != 1) {
        OPENSSL_cleanse(salt, sizeof(salt));
        OPENSSL_cleanse(expected, sizeof(expected));
        OPENSSL_cleanse(key, sizeof(key));
        return 0;
    }

    int ok = (CRYPTO_memcmp(expected, key, KEY_LEN) == 0) ? 1 : 0;
    OPENSSL_cleanse(salt, sizeof(salt));
    OPENSSL_cleanse(expected, sizeof(expected));
    OPENSSL_cleanse(key, sizeof(key));
    OPENSSL_cleanse(hash_hex, sizeof(hash_hex));
    return ok;
}

int main(void) {
    // 1) Successful registration
    printf("Test1 register alice: %s\n", register_user("alice_01", "Str0ng!Passw0rd") ? "true" : "false");
    // 2) Successful login
    printf("Test2 login alice correct: %s\n", verify_login("alice_01", "Str0ng!Passw0rd") ? "true" : "false");
    // 3) Failed login due to wrong password
    printf("Test3 login alice wrong: %s\n", verify_login("alice_01", "WrongPassword!1") ? "true" : "false");
    // 4) Duplicate registration should fail
    printf("Test4 duplicate register alice: %s\n", register_user("alice_01", "An0ther!Passw0rd") ? "true" : "false");
    // 5) Weak password should fail registration
    printf("Test5 register weak bob: %s\n", register_user("bob", "weakpass") ? "true" : "false");
    return 0;
}
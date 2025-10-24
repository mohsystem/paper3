#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static int is_password_strong(const char *pwd) {
    if (pwd == NULL) return 0;
    size_t len = strlen(pwd);
    if (len < 12) return 0;
    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)pwd[i];
        if (isupper(c)) hasUpper = 1;
        else if (islower(c)) hasLower = 1;
        else if (isdigit(c)) hasDigit = 1;
        else hasSpecial = 1;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

static unsigned char* generate_salt(size_t length) {
    if (length == 0 || length > 1024) return NULL;
    unsigned char *salt = (unsigned char*)malloc(length);
    if (!salt) return NULL;
    if (RAND_bytes(salt, (int)length) != 1) {
        OPENSSL_cleanse(salt, length);
        free(salt);
        return NULL;
    }
    return salt;
}

static char* to_hex(const unsigned char *data, size_t len) {
    if (!data || len == 0) return NULL;
    size_t out_len = len * 2;
    char *hex = (char*)malloc(out_len + 1);
    if (!hex) return NULL;
    static const char *digits = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        hex[2*i]     = digits[(data[i] >> 4) & 0xF];
        hex[2*i + 1] = digits[data[i] & 0xF];
    }
    hex[out_len] = '\0';
    return hex;
}

static char* hash_password(const char *password,
                           const unsigned char *salt,
                           size_t salt_len,
                           int iterations,
                           size_t dk_len) {
    if (!password || password[0] == '\0') return NULL;
    if (!is_password_strong(password)) return NULL;
    if (!salt || salt_len < 8 || salt_len > 1024) return NULL;
    if (iterations < 100000 || iterations > 5000000) return NULL;
    if (dk_len < 16 || dk_len > 64) return NULL;

    unsigned char *dk = (unsigned char*)malloc(dk_len);
    if (!dk) return NULL;

    int rc = PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                               salt, (int)salt_len,
                               iterations, EVP_sha256(),
                               (int)dk_len, dk);
    if (rc != 1) {
        OPENSSL_cleanse(dk, dk_len);
        free(dk);
        return NULL;
    }

    char *hex = to_hex(dk, dk_len);
    OPENSSL_cleanse(dk, dk_len);
    free(dk);
    return hex;
}

int main(void) {
    const char *passwords[5] = {
        "Str0ngPassw0rd!",
        "An0ther$ecurePwd",
        "C0mpl3x#Pass123",
        "S@feAndS0und2025",
        "R0bust_P@ssw0rd!!"
    };

    for (int i = 0; i < 5; i++) {
        unsigned char *salt = generate_salt(16);
        if (!salt) {
            fprintf(stderr, "Error.\n");
            return 1;
        }
        char *hash = hash_password(passwords[i], salt, 16, 210000, 32);
        OPENSSL_cleanse(salt, 16);
        free(salt);
        if (!hash) {
            fprintf(stderr, "Error.\n");
            return 1;
        }
        printf("Test %d hash: %s\n", i + 1, hash);
        OPENSSL_cleanse(hash, strlen(hash));
        free(hash);
    }
    return 0;
}
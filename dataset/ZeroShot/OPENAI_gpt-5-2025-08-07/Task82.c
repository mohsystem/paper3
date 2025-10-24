#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

char* hash_password_with_salt(const char* password, const char* salt) {
    if (password == NULL || salt == NULL) {
        return NULL;
    }
    size_t salt_len = strlen(salt);
    size_t pw_len = strlen(password);
    if (salt_len > (size_t)INT_MAX) {
        return NULL;
    }
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;
    unsigned char* res = HMAC(EVP_sha256(),
                              (const unsigned char*)salt,
                              (int)salt_len,
                              (const unsigned char*)password,
                              pw_len,
                              md,
                              &md_len);
    if (res == NULL || md_len == 0) {
        return NULL;
    }
    static const char hex_digits[] = "0123456789abcdef";
    char* hex = (char*)malloc(md_len * 2 + 1);
    if (!hex) {
        return NULL;
    }
    for (unsigned int i = 0; i < md_len; ++i) {
        hex[i * 2] = hex_digits[(md[i] >> 4) & 0xF];
        hex[i * 2 + 1] = hex_digits[md[i] & 0xF];
    }
    hex[md_len * 2] = '\0';
    return hex;
}

int main(void) {
    const char* tests[5][2] = {
        {"password123", "NaCl"},
        {"correcthorsebatterystaple", "pepper"},
        {"", "salt"},
        {"p@ssw0rd!#%", "S@1t"},
        {"unicode-パスワード", "ソルト"}
    };
    for (int i = 0; i < 5; ++i) {
        char* out = hash_password_with_salt(tests[i][0], tests[i][1]);
        if (out) {
            printf("%s\n", out);
            free(out);
        } else {
            printf("error\n");
        }
    }
    return 0;
}
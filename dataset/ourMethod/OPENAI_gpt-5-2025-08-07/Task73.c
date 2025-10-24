#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

typedef unsigned char u8;

static int meets_password_policy(const char *pwd) {
    if (pwd == NULL) return 0;
    size_t len = strnlen(pwd, 1024);
    if (len < 12 || len > 128) return 0;

    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)pwd[i];
        if (c < 0x20 || c > 0x7E) return 0; // control/non-ASCII
        if (isspace(c)) return 0;           // disallow spaces
        if (isupper(c)) hasUpper = 1;
        else if (islower(c)) hasLower = 1;
        else if (isdigit(c)) hasDigit = 1;
        else hasSpecial = 1;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

static int constant_time_equals(const u8 *a, size_t alen, const u8 *b, size_t blen) {
    size_t n = (alen > blen) ? alen : blen;
    u8 diff = 0;
    for (size_t i = 0; i < n; ++i) {
        u8 av = (i < alen) ? a[i] : 0;
        u8 bv = (i < blen) ? b[i] : 0;
        diff |= (u8)(av ^ bv);
    }
    return (diff == 0) && (alen == blen);
}

static int hex_to_bytes(const char *hex, u8 **out, size_t *out_len) {
    if (!hex || !out || !out_len) return 0;
    size_t len = strlen(hex);
    if (len == 0 || len > 4096 || (len % 2) != 0) return 0;

    *out_len = len / 2;
    *out = (u8 *)malloc(*out_len);
    if (!*out) return 0;

    for (size_t i = 0; i < *out_len; ++i) {
        char h = hex[2*i];
        char l = hex[2*i + 1];
        int hi, lo;
        if (h >= '0' && h <= '9') hi = h - '0';
        else if (h >= 'a' && h <= 'f') hi = 10 + (h - 'a');
        else if (h >= 'A' && h <= 'F') hi = 10 + (h - 'A');
        else { free(*out); *out = NULL; *out_len = 0; return 0; }

        if (l >= '0' && l <= '9') lo = l - '0';
        else if (l >= 'a' && l <= 'f') lo = 10 + (l - 'a');
        else if (l >= 'A' && l <= 'F') lo = 10 + (l - 'A');
        else { free(*out); *out = NULL; *out_len = 0; return 0; }

        (*out)[i] = (u8)((hi << 4) | lo);
    }
    return 1;
}

static int bytes_to_hex(const u8 *in, size_t in_len, char **out_hex) {
    static const char *digits = "0123456789abcdef";
    if (!in || in_len == 0 || !out_hex) return 0;
    size_t out_len = in_len * 2;
    *out_hex = (char *)malloc(out_len + 1);
    if (!*out_hex) return 0;
    for (size_t i = 0; i < in_len; ++i) {
        (*out_hex)[2*i]     = digits[in[i] >> 4];
        (*out_hex)[2*i + 1] = digits[in[i] & 0x0F];
    }
    (*out_hex)[out_len] = '\0';
    return 1;
}

static int derive_key_pbkdf2(const char *password,
                             const u8 *salt, size_t salt_len,
                             int iterations,
                             size_t dk_len,
                             u8 *out) {
    if (!password || !salt || !out) return 0;
    if (iterations < 10000 || iterations > 10000000) return 0;
    if (salt_len < 16 || dk_len == 0 || dk_len > 1024) return 0;
    int rc = PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                               salt, (int)salt_len,
                               iterations, EVP_sha256(),
                               (int)dk_len, out);
    return rc == 1;
}

static int secure_random_bytes(u8 *buf, size_t n) {
    if (n == 0) return 1;
    if (!buf) return 0;
    int rc = RAND_bytes(buf, (int)n);
    return rc == 1;
}

static void secure_shuffle(char *s, size_t len) {
    if (!s || len < 2) return;
    for (size_t i = len - 1; i >= 1; --i) {
        unsigned int r = 0;
        RAND_bytes((u8 *)&r, sizeof(r));
        size_t j = (size_t)(r % (i + 1));
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        if (i == 0) break; // safety (though loop stops at i>=1)
    }
}

static int generate_policy_compliant_password(size_t length, char **out_pwd) {
    const char *lowers = "abcdefghijklmnopqrstuvwxyz";
    const char *uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *digits = "0123456789";
    const char *specials = "!@#$%^&*()-_=+[]{};:,<.>/?\\|`~";
    char all[256];
    snprintf(all, sizeof(all), "%s%s%s%s", lowers, uppers, digits, specials);

    if (length < 12) length = 12;
    char *pwd = (char *)malloc(length + 1);
    if (!pwd) return 0;

    auto pick_from = [](const char *set) -> char {
        size_t n = strlen(set);
        unsigned int r = 0;
        RAND_bytes((u8 *)&r, sizeof(r));
        return set[r % n];
    };

    // Ensure complexity
    size_t pos = 0;
    pwd[pos++] = pick_from(lowers);
    pwd[pos++] = pick_from(uppers);
    pwd[pos++] = pick_from(digits);
    pwd[pos++] = pick_from(specials);

    while (pos < length) {
        pwd[pos++] = pick_from(all);
    }
    pwd[length] = '\0';
    secure_shuffle(pwd, length);
    *out_pwd = pwd;
    return 1;
}

static int set_env_var(const char *name, const char *value) {
#if defined(_WIN32)
    return _putenv_s(name, value) == 0;
#else
    return setenv(name, value, 1) == 0;
#endif
}

static int verify_admin_password(const char *password) {
    if (!meets_password_policy(password)) return 0;

    const char *saltHex = getenv("ADMIN_PW_SALT");
    const char *hashHex = getenv("ADMIN_PW_HASH");
    const char *iterStr = getenv("ADMIN_PW_ITERS");
    if (!saltHex || !hashHex) return 0;

    long long iterations = 120000;
    if (iterStr) {
        char *endp = NULL;
        iterations = strtoll(iterStr, &endp, 10);
        if (endp == iterStr || iterations < 10000 || iterations > 10000000) return 0;
    }

    u8 *salt = NULL, *stored = NULL;
    size_t salt_len = 0, stored_len = 0;
    if (!hex_to_bytes(saltHex, &salt, &salt_len)) return 0;
    if (!hex_to_bytes(hashHex, &stored, &stored_len)) {
        OPENSSL_cleanse(salt, salt_len);
        free(salt);
        return 0;
    }
    if (salt_len < 16 || stored_len < 16 || stored_len > 64) {
        OPENSSL_cleanse(salt, salt_len);
        free(salt);
        OPENSSL_cleanse(stored, stored_len);
        free(stored);
        return 0;
    }

    u8 *derived = (u8 *)malloc(stored_len);
    if (!derived) {
        OPENSSL_cleanse(salt, salt_len);
        free(salt);
        OPENSSL_cleanse(stored, stored_len);
        free(stored);
        return 0;
    }
    int ok = derive_key_pbkdf2(password, salt, salt_len, (int)iterations, stored_len, derived);
    int eq = 0;
    if (ok) {
        eq = constant_time_equals(derived, stored_len, stored, stored_len);
    }

    OPENSSL_cleanse(derived, stored_len);
    free(derived);
    OPENSSL_cleanse(salt, salt_len);
    free(salt);
    OPENSSL_cleanse(stored, stored_len);
    free(stored);

    return ok && eq;
}

int main(void) {
    // Setup admin credentials securely for testing
    u8 salt[16];
    if (!secure_random_bytes(salt, sizeof(salt))) {
        printf("RNG failure\n");
        return 1;
    }

    char *adminPwd = NULL;
    if (!generate_policy_compliant_password(16, &adminPwd)) {
        printf("Password gen failure\n");
        return 1;
    }

    int iterations = 160000;
    u8 adminHash[32];
    if (!derive_key_pbkdf2(adminPwd, salt, sizeof(salt), iterations, sizeof(adminHash), adminHash)) {
        printf("Derive failed\n");
        OPENSSL_cleanse(adminPwd, strlen(adminPwd));
        free(adminPwd);
        return 1;
    }

    char *saltHex = NULL;
    char *hashHex = NULL;
    if (!bytes_to_hex(salt, sizeof(salt), &saltHex) ||
        !bytes_to_hex(adminHash, sizeof(adminHash), &hashHex)) {
        printf("Hex encode failed\n");
        if (saltHex) free(saltHex);
        if (hashHex) free(hashHex);
        OPENSSL_cleanse(adminPwd, strlen(adminPwd));
        free(adminPwd);
        return 1;
    }

    char iterBuf[32];
    snprintf(iterBuf, sizeof(iterBuf), "%d", iterations);

    if (!set_env_var("ADMIN_PW_SALT", saltHex) ||
        !set_env_var("ADMIN_PW_HASH", hashHex) ||
        !set_env_var("ADMIN_PW_ITERS", iterBuf)) {
        printf("Env setup failed\n");
        free(saltHex);
        free(hashHex);
        OPENSSL_cleanse(adminPwd, strlen(adminPwd));
        free(adminPwd);
        return 1;
    }

    // Prepare test passwords
    char *correct = adminPwd;

    // wrong password: copy and mutate last char
    char wrongBuf[256];
    memset(wrongBuf, 0, sizeof(wrongBuf));
    size_t cLen = strnlen(correct, sizeof(wrongBuf) - 1);
    memcpy(wrongBuf, correct, cLen);
    if (cLen > 0) {
        unsigned char ch = (unsigned char)wrongBuf[cLen - 1];
        if (isdigit(ch)) wrongBuf[cLen - 1] = (ch == '9') ? '0' : (char)(ch + 1);
        else if (isalpha(ch)) {
            if (isupper(ch)) wrongBuf[cLen - 1] = (ch == 'Z') ? 'A' : (char)(ch + 1);
            else wrongBuf[cLen - 1] = (ch == 'z') ? 'a' : (char)(ch + 1);
        } else wrongBuf[cLen - 1] = (ch == '!') ? '@' : '!';
    }

    const char *tooShort = "Aa1!Aa1!Aa"; // len 10
    const char *noUpper = "lowercase1!more"; // no uppercase

    // Tamper hash
    char *tamperedHash = strdup(hashHex);
    if (tamperedHash) {
        if (tamperedHash[0] != '\0') tamperedHash[0] = (tamperedHash[0] == 'f') ? '0' : 'f';
    }

    int passed = 0, total = 5;

    int r1 = verify_admin_password(correct);
    printf("Test 1 (correct password): %s\n", r1 ? "PASS" : "FAIL");
    passed += r1 ? 1 : 0;

    int r2 = verify_admin_password(wrongBuf);
    printf("Test 2 (wrong password): %s\n", (!r2) ? "PASS" : "FAIL");
    passed += (!r2) ? 1 : 0;

    int r3 = verify_admin_password(tooShort);
    printf("Test 3 (too short): %s\n", (!r3) ? "PASS" : "FAIL");
    passed += (!r3) ? 1 : 0;

    int r4 = verify_admin_password(noUpper);
    printf("Test 4 (missing uppercase): %s\n", (!r4) ? "PASS" : "FAIL");
    passed += (!r4) ? 1 : 0;

    if (tamperedHash) {
        set_env_var("ADMIN_PW_HASH", tamperedHash);
    }
    int r5 = verify_admin_password(correct);
    printf("Test 5 (tampered hash): %s\n", (!r5) ? "PASS" : "FAIL");
    passed += (!r5) ? 1 : 0;

    // Restore hash
    set_env_var("ADMIN_PW_HASH", hashHex);

    printf("Passed %d of %d tests\n", passed, total);

    // Clean sensitive data
    OPENSSL_cleanse(adminHash, sizeof(adminHash));
    OPENSSL_cleanse(salt, sizeof(salt));
    if (adminPwd) {
        OPENSSL_cleanse(adminPwd, strlen(adminPwd));
        free(adminPwd);
    }
    if (saltHex) free(saltHex);
    if (hashHex) free(hashHex);
    if (tamperedHash) free(tamperedHash);

    return (passed == total) ? 0 : 2;
}
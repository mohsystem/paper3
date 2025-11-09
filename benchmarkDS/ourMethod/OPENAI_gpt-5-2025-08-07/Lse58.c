// Simple secure login system in C with email change functionality.
// Uses PBKDF2-HMAC-SHA256 with per-user random salt and strong password policy.
// Requires OpenSSL libcrypto for PBKDF2, RNG, and constant-time compare.
//
// Build example (may vary by system):
//   gcc -Wall -Wextra -O2 login_email_change.c -o login_email_change -lcrypto
//
// This file is self-contained and includes a main() with 5 test cases.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SALT_LEN 16u
#define HASH_LEN 32u
#define PBKDF2_ITERS 210000

#define EMAIL_MAX 255u
#define USER_EMAIL_BUF (EMAIL_MAX + 1u)

typedef struct {
    char email[USER_EMAIL_BUF];
    unsigned char salt[SALT_LEN];
    unsigned char pw_hash[HASH_LEN];
    int iterations;
    int logged_in;
} User;

enum {
    ERR_OK = 0,
    ERR_INVALID_ARG = 1,
    ERR_WEAK_PASSWORD = 2,
    ERR_INVALID_EMAIL = 3,
    ERR_CRYPTO = 4,
    ERR_AUTH_FAILED = 5,
    ERR_NOT_LOGGED_IN = 6,
    ERR_EMAIL_MISMATCH = 7
};

static void secure_zero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    OPENSSL_cleanse(ptr, len);
}

static int generate_salt(unsigned char *salt, size_t salt_len) {
    if (salt == NULL || salt_len == 0) return ERR_INVALID_ARG;
    if (RAND_bytes(salt, (int)salt_len) != 1) return ERR_CRYPTO;
    return ERR_OK;
}

static int derive_key(const char *password,
                      const unsigned char *salt, size_t salt_len,
                      int iterations,
                      unsigned char *out_key, size_t out_len) {
    if (!password || !salt || !out_key || salt_len == 0 || out_len == 0 || iterations <= 0) {
        return ERR_INVALID_ARG;
    }
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                          salt, (int)salt_len,
                          iterations,
                          EVP_sha256(),
                          (int)out_len, out_key) != 1) {
        return ERR_CRYPTO;
    }
    return ERR_OK;
}

static int validate_email(const char *email) {
    if (email == NULL) return 0;
    size_t len = strnlen(email, USER_EMAIL_BUF);
    if (len == 0 || len > EMAIL_MAX) return 0;

    // Basic format checks: must contain exactly one '@', local and domain parts valid
    const char *at = strchr(email, '@');
    if (at == NULL) return 0;
    if (strchr(at + 1, '@') != NULL) return 0; // more than one '@'

    size_t local_len = (size_t)(at - email);
    size_t domain_len = len - local_len - 1u;
    if (local_len == 0 || domain_len < 3) return 0; // domain like a.b

    // Allowed chars: local [A-Za-z0-9._%+-], domain [A-Za-z0-9.-], domain must contain '.'
    for (size_t i = 0; i < local_len; i++) {
        unsigned char c = (unsigned char)email[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-')) {
            return 0;
        }
    }
    const char *domain = at + 1;
    if (domain[0] == '-' || domain[0] == '.' || domain[domain_len - 1] == '-' || domain[domain_len - 1] == '.') {
        return 0;
    }
    int dot_found = 0;
    for (size_t i = 0; i < domain_len; i++) {
        unsigned char c = (unsigned char)domain[i];
        if (c == '.') dot_found = 1;
        if (!(isalnum(c) || c == '.' || c == '-')) {
            return 0;
        }
    }
    if (!dot_found) return 0;
    return 1;
}

static int password_is_strong(const char *pwd) {
    if (pwd == NULL) return 0;
    size_t len = strnlen(pwd, 1024);
    if (len < 12 || len > 256) return 0;
    int has_lower = 0, has_upper = 0, has_digit = 0, has_special = 0;
    const char *specials = "!@#$%^&*()-_=+[]{};:,.?/";
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)pwd[i];
        if (isspace(c)) return 0; // disallow whitespace
        if (islower(c)) has_lower = 1;
        else if (isupper(c)) has_upper = 1;
        else if (isdigit(c)) has_digit = 1;
        else {
            // check if in allowed specials
            for (const char *p = specials; *p; ++p) {
                if (c == (unsigned char)*p) { has_special = 1; break; }
            }
        }
    }
    return has_lower && has_upper && has_digit && has_special;
}

int create_user(User *u, const char *email, const char *password) {
    if (u == NULL || email == NULL || password == NULL) return ERR_INVALID_ARG;
    memset(u, 0, sizeof(*u));

    if (!validate_email(email)) return ERR_INVALID_EMAIL;
    if (!password_is_strong(password)) return ERR_WEAK_PASSWORD;

    int rc = generate_salt(u->salt, SALT_LEN);
    if (rc != ERR_OK) return rc;

    u->iterations = PBKDF2_ITERS;

    rc = derive_key(password, u->salt, SALT_LEN, u->iterations, u->pw_hash, HASH_LEN);
    if (rc != ERR_OK) {
        secure_zero(u->salt, SALT_LEN);
        secure_zero(u->pw_hash, HASH_LEN);
        u->iterations = 0;
        return rc;
    }

    // Store email safely
    int n = snprintf(u->email, sizeof(u->email), "%s", email);
    if (n < 0 || (size_t)n >= sizeof(u->email)) {
        secure_zero(u->salt, SALT_LEN);
        secure_zero(u->pw_hash, HASH_LEN);
        u->iterations = 0;
        memset(u->email, 0, sizeof(u->email));
        return ERR_INVALID_EMAIL;
    }

    u->logged_in = 0;
    return ERR_OK;
}

int login(User *u, const char *email, const char *password) {
    if (u == NULL || email == NULL || password == NULL) return ERR_INVALID_ARG;
    // Email check first
    size_t given_len = strnlen(email, USER_EMAIL_BUF);
    if (given_len == 0 || given_len > EMAIL_MAX) return ERR_INVALID_EMAIL;
    if (strcmp(u->email, email) != 0) return ERR_AUTH_FAILED;

    unsigned char dk[HASH_LEN];
    int rc = derive_key(password, u->salt, SALT_LEN, u->iterations, dk, HASH_LEN);
    if (rc != ERR_OK) {
        secure_zero(dk, sizeof(dk));
        return rc;
    }
    int same = (CRYPTO_memcmp(dk, u->pw_hash, HASH_LEN) == 0);
    secure_zero(dk, sizeof(dk));
    if (!same) {
        u->logged_in = 0;
        return ERR_AUTH_FAILED;
    }
    u->logged_in = 1;
    return ERR_OK;
}

int logout(User *u) {
    if (u == NULL) return ERR_INVALID_ARG;
    u->logged_in = 0;
    return ERR_OK;
}

int change_email(User *u, const char *old_email, const char *password, const char *new_email) {
    if (u == NULL || old_email == NULL || password == NULL || new_email == NULL) return ERR_INVALID_ARG;
    if (!u->logged_in) return ERR_NOT_LOGGED_IN;

    if (strcmp(u->email, old_email) != 0) return ERR_EMAIL_MISMATCH;

    unsigned char dk[HASH_LEN];
    int rc = derive_key(password, u->salt, SALT_LEN, u->iterations, dk, HASH_LEN);
    if (rc != ERR_OK) {
        secure_zero(dk, sizeof(dk));
        return rc;
    }
    int same = (CRYPTO_memcmp(dk, u->pw_hash, HASH_LEN) == 0);
    secure_zero(dk, sizeof(dk));
    if (!same) return ERR_AUTH_FAILED;

    if (!validate_email(new_email)) return ERR_INVALID_EMAIL;
    int n = snprintf(u->email, sizeof(u->email), "%s", new_email);
    if (n < 0 || (size_t)n >= sizeof(u->email)) {
        return ERR_INVALID_EMAIL;
    }
    return ERR_OK;
}

// Utility: generate a strong random password meeting the policy.
// out_len must be >= 16 to reliably include all categories; we will use 16.
static int generate_strong_password(char *out, size_t out_len) {
    if (out == NULL || out_len < 16) return 0;
    const char *lower = "abcdefghijklmnopqrstuvwxyz";
    const char *upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *digits = "0123456789";
    const char *special = "!@#$%^&*()-_=+[]{};:,.?/";
    const char *all = "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "0123456789"
                      "!@#$%^&*()-_=+[]{};:,.?/";

    size_t L = strlen(lower), U = strlen(upper), D = strlen(digits), S = strlen(special), A = strlen(all);
    unsigned char rnd[64];
    if (out_len > sizeof(rnd)) return 0;

    // Ensure at least one from each category
    if (RAND_bytes(rnd, 4) != 1) return 0;
    out[0] = lower[rnd[0] % L];
    out[1] = upper[rnd[1] % U];
    out[2] = digits[rnd[2] % D];
    out[3] = special[rnd[3] % S];

    size_t idx = 4;
    size_t remaining = out_len - 1; // leave space for NUL
    if (remaining < 4) return 0;

    remaining -= 4;
    if (remaining > 0) {
        if (RAND_bytes(rnd, (int)remaining) != 1) return 0;
        for (size_t i = 0; i < remaining; i++) {
            out[idx++] = all[rnd[i] % A];
        }
    }

    // Simple shuffle (Fisher-Yates) to avoid predictable placement
    size_t total_chars = out_len - 1;
    if (RAND_bytes(rnd, (int)total_chars) != 1) return 0;
    for (size_t i = total_chars - 1; i > 0; i--) {
        size_t j = rnd[i] % (i + 1);
        char tmp = out[i];
        out[i] = out[j];
        out[j] = tmp;
    }

    out[out_len - 1] = '\0';
    secure_zero(rnd, sizeof(rnd));
    return password_is_strong(out);
}

// Simple test assertion printer without leaking secrets.
static void print_result(const char *name, int condition) {
    printf("[TEST] %s: %s\n", name, condition ? "PASS" : "FAIL");
}

int main(void) {
    // Test setup: create a user with a strong random password
    User u;
    char strong_pwd[32];
    if (!generate_strong_password(strong_pwd, sizeof(strong_pwd))) {
        printf("Failed to generate strong password. Aborting tests.\n");
        return 1;
    }

    const char *initial_email = "user@example.com";
    int rc = create_user(&u, initial_email, strong_pwd);
    print_result("Create user with strong password", rc == ERR_OK);
    if (rc != ERR_OK) return 1;

    // Test 1: Successful login and email change with correct old email and password
    rc = login(&u, initial_email, strong_pwd);
    print_result("Login with correct credentials (initial email)", rc == ERR_OK);

    const char *new_email_1 = "new.user@example.org";
    int rc_change = change_email(&u, initial_email, strong_pwd, new_email_1);
    print_result("Change email with correct old email and password", rc_change == ERR_OK);
    int email_changed_ok = (strcmp(u.email, new_email_1) == 0);
    print_result("Email updated verification", email_changed_ok);

    // Test 2: Attempt change with wrong old email (should fail)
    // Ensure logged in with current email
    rc = login(&u, new_email_1, strong_pwd);
    print_result("Re-login with updated email", rc == ERR_OK);
    const char *wrong_old = "wrong@example.com";
    rc_change = change_email(&u, wrong_old, strong_pwd, "another@example.net");
    print_result("Change email with wrong old email (should fail)", rc_change == ERR_EMAIL_MISMATCH);
    print_result("Email remains unchanged after failed attempt", strcmp(u.email, new_email_1) == 0);

    // Test 3: Attempt change with wrong password (should fail)
    rc = login(&u, new_email_1, strong_pwd);
    print_result("Login before wrong-password change attempt", rc == ERR_OK);
    const char *bad_pwd = "BadPassword123!";
    rc_change = change_email(&u, new_email_1, bad_pwd, "third@example.com");
    print_result("Change email with wrong password (should fail)", rc_change == ERR_AUTH_FAILED);
    print_result("Email remains unchanged after wrong password attempt", strcmp(u.email, new_email_1) == 0);

    // Test 4: Attempt change without being logged in (should fail)
    rc = logout(&u);
    print_result("Logout user", rc == ERR_OK);
    rc_change = change_email(&u, new_email_1, strong_pwd, "logoutchange@example.com");
    print_result("Change email while logged out (should fail)", rc_change == ERR_NOT_LOGGED_IN);
    print_result("Email remains unchanged after logged-out attempt", strcmp(u.email, new_email_1) == 0);

    // Test 5: Weak password is rejected at user creation
    User u2;
    const char *weak_pwd = "weakpwd";
    rc = create_user(&u2, "weak@example.com", weak_pwd);
    print_result("Create user with weak password (should fail)", rc == ERR_WEAK_PASSWORD);

    // Clear sensitive password buffers before exit
    secure_zero(strong_pwd, sizeof(strong_pwd));

    return 0;
}
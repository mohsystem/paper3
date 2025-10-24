#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#define SALT_LEN 16
#define HASH_LEN 32
#define OTP_LEN 6
#define PBKDF2_ITERS 210000
#define OTP_TTL_SEC 120

#define MAX_USERS 16
#define MAX_CHALLENGES 64
#define USERNAME_MAX 64
#define CONTACT_MAX 128
#define CHALLENGE_ID_HEX_LEN 32

typedef struct {
    char username[USERNAME_MAX];
    char contact[CONTACT_MAX];
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    int iterations;
    int used;
} User;

typedef struct {
    char id[CHALLENGE_ID_HEX_LEN + 1];
    char username[USERNAME_MAX];
    char otp[OTP_LEN + 1];
    time_t expires_at;
    int used;
} Challenge;

typedef struct {
    User users[MAX_USERS];
    size_t user_count;
    Challenge challenges[MAX_CHALLENGES];
    size_t challenge_count;
} AuthState;

static int secure_random(unsigned char* buf, size_t len) {
    if (buf == NULL || len == 0) return 0;
    return RAND_bytes(buf, (int)len) == 1 ? 1 : 0;
}

static int derive_key_pbkdf2(const char* password,
                             const unsigned char* salt, size_t salt_len,
                             int iterations,
                             unsigned char* out, size_t out_len) {
    if (!password || !salt || !out || salt_len == 0 || out_len == 0 || iterations < 100000) return 0;
    return PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                              salt, (int)salt_len,
                              iterations, EVP_sha256(),
                              (int)out_len, out) == 1 ? 1 : 0;
}

static int constant_time_eq(const unsigned char* a, const unsigned char* b, size_t len) {
    if (!a || !b) return 0;
    return CRYPTO_memcmp(a, b, len) == 0 ? 1 : 0;
}

static int constant_time_eq_chars(const char* a, const char* b, size_t len) {
    if (!a || !b) return 0;
    return CRYPTO_memcmp(a, b, len) == 0 ? 1 : 0;
}

static int to_hex(const unsigned char* in, size_t len, char* out, size_t out_size) {
    if (!in || !out || out_size < (len * 2 + 1)) return 0;
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < len; ++i) {
        out[2*i]   = hex[(in[i] >> 4) & 0xF];
        out[2*i+1] = hex[in[i] & 0xF];
    }
    out[len*2] = '\0';
    return 1;
}

static int check_password_policy(const char* pwd) {
    if (!pwd) return 0;
    size_t n = strlen(pwd);
    if (n < 12) return 0;
    int hasLower = 0, hasUpper = 0, hasDigit = 0, hasSpecial = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)pwd[i];
        if (islower(c)) hasLower = 1;
        else if (isupper(c)) hasUpper = 1;
        else if (isdigit(c)) hasDigit = 1;
        else if (ispunct(c) || isspace(c)) hasSpecial = 1;
    }
    return (hasLower && hasUpper && hasDigit && hasSpecial) ? 1 : 0;
}

static int generate_otp(char* out, size_t out_size) {
    if (!out || out_size < OTP_LEN + 1) return 0;
    size_t produced = 0;
    while (produced < OTP_LEN) {
        unsigned char byte = 0;
        if (!secure_random(&byte, 1)) return 0;
        if (byte >= 250) continue;
        out[produced++] = (char)('0' + (byte % 10));
    }
    out[OTP_LEN] = '\0';
    return 1;
}

static const User* find_user(const AuthState* state, const char* username) {
    if (!state || !username) return NULL;
    for (size_t i = 0; i < state->user_count; ++i) {
        if (strncmp(state->users[i].username, username, USERNAME_MAX) == 0) return &state->users[i];
    }
    return NULL;
}

static User* find_user_mutable(AuthState* state, const char* username) {
    if (!state || !username) return NULL;
    for (size_t i = 0; i < state->user_count; ++i) {
        if (strncmp(state->users[i].username, username, USERNAME_MAX) == 0) return &state->users[i];
    }
    return NULL;
}

static int create_user(AuthState* state, const char* username, const char* password, const char* contact) {
    if (!state || !username || !password || !contact) return 0;
    if (state->user_count >= MAX_USERS) return 0;
    if (strlen(username) == 0 || strlen(contact) == 0) return 0;
    if (!check_password_policy(password)) return 0;
    if (find_user(state, username) != NULL) return 0;

    User* u = &state->users[state->user_count];
    memset(u, 0, sizeof(*u));
    snprintf(u->username, USERNAME_MAX, "%s", username);
    snprintf(u->contact, CONTACT_MAX, "%s", contact);
    u->iterations = PBKDF2_ITERS;
    u->used = 1;

    if (!secure_random(u->salt, SALT_LEN)) return 0;
    if (!derive_key_pbkdf2(password, u->salt, SALT_LEN, u->iterations, u->hash, HASH_LEN)) return 0;

    state->user_count++;
    return 1;
}

static Challenge* find_challenge(AuthState* state, const char* challenge_id) {
    if (!state || !challenge_id) return NULL;
    for (size_t i = 0; i < state->challenge_count; ++i) {
        if (strncmp(state->challenges[i].id, challenge_id, CHALLENGE_ID_HEX_LEN) == 0) return &state->challenges[i];
    }
    return NULL;
}

static int start_login(AuthState* state, const char* username, const char* password,
                       time_t now, char* out_challenge_id, size_t id_size) {
    if (!state || !username || !password || !out_challenge_id || id_size < (CHALLENGE_ID_HEX_LEN + 1)) return 0;
    const User* u = find_user(state, username);
    if (!u) return 0;

    unsigned char candidate[HASH_LEN];
    memset(candidate, 0, sizeof(candidate));
    if (!derive_key_pbkdf2(password, u->salt, SALT_LEN, u->iterations, candidate, HASH_LEN)) {
        OPENSSL_cleanse(candidate, sizeof(candidate));
        return 0;
    }
    int ok = constant_time_eq(candidate, u->hash, HASH_LEN);
    OPENSSL_cleanse(candidate, sizeof(candidate));
    if (!ok) return 0;

    if (state->challenge_count >= MAX_CHALLENGES) return 0;

    Challenge* c = &state->challenges[state->challenge_count];
    memset(c, 0, sizeof(*c));

    unsigned char rnd[16];
    if (!secure_random(rnd, sizeof(rnd))) return 0;
    if (!to_hex(rnd, sizeof(rnd), c->id, sizeof(c->id))) return 0;

    snprintf(c->username, USERNAME_MAX, "%s", username);
    if (!generate_otp(c->otp, sizeof(c->otp))) return 0;
    c->expires_at = now + OTP_TTL_SEC;
    c->used = 0;

    // Output challenge ID
    snprintf(out_challenge_id, id_size, "%s", c->id);
    state->challenge_count++;
    return 1;
}

static int verify_otp(AuthState* state, const char* challenge_id, const char* otp_input, time_t now) {
    if (!state || !challenge_id || !otp_input) return 0;
    if (strlen(otp_input) != OTP_LEN) return 0;

    Challenge* c = find_challenge(state, challenge_id);
    if (!c) return 0;
    if (c->used) return 0;
    if (now > c->expires_at) {
        OPENSSL_cleanse(c->otp, sizeof(c->otp));
        c->used = 1;
        return 0;
    }
    int match = constant_time_eq_chars(c->otp, otp_input, OTP_LEN);
    OPENSSL_cleanse(c->otp, sizeof(c->otp));
    c->used = 1;
    return match;
}

// Test helper: retrieve OTP without printing (for automated tests only)
static int get_test_otp(AuthState* state, const char* challenge_id, char* out_otp, size_t out_size) {
    if (!state || !challenge_id || !out_otp || out_size < OTP_LEN + 1) return 0;
    Challenge* c = find_challenge(state, challenge_id);
    if (!c || c->used) return 0;
    memcpy(out_otp, c->otp, OTP_LEN + 1);
    out_otp[OTP_LEN] = '\0';
    return 1;
}

int main(void) {
    AuthState state;
    memset(&state, 0, sizeof(state));

    const char* username = "alice";
    const char* contact = "alice@example.com";
    const char* strongPwd = "Str0ngP@ssw0rd!";

    if (!create_user(&state, username, strongPwd, contact)) {
        printf("Setup failed\n");
        return 1;
    }

    time_t now = time(NULL);
    int passed = 0, total = 0;

    // Test 1: Wrong password should fail
    {
        total++;
        char cid[CHALLENGE_ID_HEX_LEN + 1];
        memset(cid, 0, sizeof(cid));
        int ok = start_login(&state, username, "WrongP@ssw0rd!", now, cid, sizeof(cid));
        if (!ok && cid[0] == '\0') {
            printf("Test1 PASS\n");
            passed++;
        } else {
            printf("Test1 FAIL\n");
        }
    }

    // Test 2: Correct password but wrong OTP should fail
    {
        total++;
        char cid[CHALLENGE_ID_HEX_LEN + 1];
        memset(cid, 0, sizeof(cid));
        int test_ok = 0;
        if (start_login(&state, username, strongPwd, now, cid, sizeof(cid))) {
            char realOtp[OTP_LEN + 1];
            if (get_test_otp(&state, cid, realOtp, sizeof(realOtp))) {
                char wrong[OTP_LEN + 1];
                memcpy(wrong, realOtp, OTP_LEN + 1);
                wrong[0] = (wrong[0] == '0') ? '1' : '0';
                if (!verify_otp(&state, cid, wrong, now)) {
                    test_ok = 1;
                }
            }
        }
        printf("%s\n", test_ok ? "Test2 PASS" : "Test2 FAIL");
        if (test_ok) passed++;
    }

    // Test 3: Correct password and OTP but expired should fail
    {
        total++;
        char cid[CHALLENGE_ID_HEX_LEN + 1];
        memset(cid, 0, sizeof(cid));
        int test_ok = 0;
        if (start_login(&state, username, strongPwd, now, cid, sizeof(cid))) {
            char realOtp[OTP_LEN + 1];
            if (get_test_otp(&state, cid, realOtp, sizeof(realOtp))) {
                time_t later = now + 1000;
                if (!verify_otp(&state, cid, realOtp, later)) {
                    test_ok = 1;
                }
            }
        }
        printf("%s\n", test_ok ? "Test3 PASS" : "Test3 FAIL");
        if (test_ok) passed++;
    }

    // Test 4: Correct password and OTP should succeed
    {
        total++;
        char cid[CHALLENGE_ID_HEX_LEN + 1];
        memset(cid, 0, sizeof(cid));
        int test_ok = 0;
        if (start_login(&state, username, strongPwd, now, cid, sizeof(cid))) {
            char realOtp[OTP_LEN + 1];
            if (get_test_otp(&state, cid, realOtp, sizeof(realOtp))) {
                if (verify_otp(&state, cid, realOtp, now)) {
                    test_ok = 1;
                }
            }
        }
        printf("%s\n", test_ok ? "Test4 PASS" : "Test4 FAIL");
        if (test_ok) passed++;
    }

    // Test 5: OTP reuse should fail
    {
        total++;
        char cid[CHALLENGE_ID_HEX_LEN + 1];
        memset(cid, 0, sizeof(cid));
        int test_ok = 0;
        if (start_login(&state, username, strongPwd, now, cid, sizeof(cid))) {
            char realOtp[OTP_LEN + 1];
            if (get_test_otp(&state, cid, realOtp, sizeof(realOtp))) {
                int first = verify_otp(&state, cid, realOtp, now);
                int second = verify_otp(&state, cid, realOtp, now);
                if (first && !second) test_ok = 1;
            }
        }
        printf("%s\n", test_ok ? "Test5 PASS" : "Test5 FAIL");
        if (test_ok) passed++;
    }

    printf("Passed %d of %d tests\n", passed, total);
    return 0;
}
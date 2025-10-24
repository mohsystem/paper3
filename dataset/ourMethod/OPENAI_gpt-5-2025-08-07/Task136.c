#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define SALT_LEN 16
#define HASH_LEN 32
#define ITERATIONS 210000
#define PASSWORD_EXPIRATION_SECONDS (90L * 24L * 60L * 60L) /* 90 days */
#define MAX_USERS 100
#define USERNAME_MAX 32

typedef struct {
    char username[USERNAME_MAX + 1];
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    int iterations;
    time_t createdAt;
} User;

typedef struct {
    User users[MAX_USERS];
    int count;
} UserStore;

static int constant_time_equal(const unsigned char* a, const unsigned char* b, size_t len) {
    return CRYPTO_memcmp(a, b, len) == 0;
}

static int validate_username(const char* username, char* err, size_t errsz) {
    if (username == NULL) {
        snprintf(err, errsz, "username required");
        return 0;
    }
    size_t n = strlen(username);
    if (n < 3 || n > USERNAME_MAX) {
        snprintf(err, errsz, "username length must be 3-32");
        return 0;
    }
    for (size_t i = 0; i < n; i++) {
        char c = username[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '-')) {
            snprintf(err, errsz, "username may contain letters, digits, '_' or '-'");
            return 0;
        }
    }
    err[0] = '\0';
    return 1;
}

static int is_common_password(const char* lower) {
    const char* blacklist[] = {
        "password", "123456", "123456789", "qwerty", "letmein", "welcome", "admin", "iloveyou"
    };
    for (size_t i = 0; i < sizeof(blacklist)/sizeof(blacklist[0]); i++) {
        if (strcmp(lower, blacklist[i]) == 0) return 1;
    }
    return 0;
}

static int validate_password_policy(const char* username, const char* password, char* err, size_t errsz) {
    if (password == NULL) {
        snprintf(err, errsz, "password required");
        return 0;
    }
    size_t n = strlen(password);
    if (n < 12 || n > 128) {
        snprintf(err, errsz, "password length must be 12-128");
        return 0;
    }
    int hasLower = 0, hasUpper = 0, hasDigit = 0, hasSpecial = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)password[i];
        if (islower(c)) hasLower = 1;
        else if (isupper(c)) hasUpper = 1;
        else if (isdigit(c)) hasDigit = 1;
        else hasSpecial = 1;
    }
    if (!(hasLower && hasUpper && hasDigit && hasSpecial)) {
        snprintf(err, errsz, "password must contain upper, lower, digit, and special character");
        return 0;
    }
    char lowerPass[129];
    size_t lp = n < sizeof(lowerPass)-1 ? n : sizeof(lowerPass)-1;
    for (size_t i = 0; i < lp; i++) lowerPass[i] = (char)tolower((unsigned char)password[i]);
    lowerPass[lp] = '\0';

    char lowerUser[USERNAME_MAX + 1];
    size_t lu = strlen(username);
    for (size_t i = 0; i < lu && i < USERNAME_MAX; i++) lowerUser[i] = (char)tolower((unsigned char)username[i]);
    lowerUser[lu] = '\0';

    if (strstr(lowerPass, lowerUser) != NULL) {
        snprintf(err, errsz, "password must not contain the username");
        return 0;
    }
    if (is_common_password(lowerPass)) {
        snprintf(err, errsz, "password too common");
        return 0;
    }
    err[0] = '\0';
    return 1;
}

static int derive_key_pbkdf2(const char* password, const unsigned char* salt, int iterations, unsigned char* out, size_t outlen) {
    int ok = PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LEN, iterations, EVP_sha256(), (int)outlen, out);
    return ok == 1;
}

static void simulate_work(const char* password) {
    unsigned char salt[SALT_LEN];
    RAND_bytes(salt, SALT_LEN);
    unsigned char out[HASH_LEN];
    derive_key_pbkdf2(password, salt, ITERATIONS, out, HASH_LEN);
}

static int is_expired(const User* u) {
    time_t now = time(NULL);
    return (now - u->createdAt) > PASSWORD_EXPIRATION_SECONDS;
}

static const char* register_user(UserStore* store, const char* username, const char* password) {
    char err[128];
    if (!validate_username(username, err, sizeof(err))) {
        static char msg1[160];
        snprintf(msg1, sizeof(msg1), "ERROR: %s", err);
        return msg1;
    }
    for (int i = 0; i < store->count; i++) {
        if (strncmp(store->users[i].username, username, USERNAME_MAX) == 0) {
            return "ERROR: username already exists";
        }
    }
    if (!validate_password_policy(username, password, err, sizeof(err))) {
        static char msg2[160];
        snprintf(msg2, sizeof(msg2), "ERROR: %s", err);
        return msg2;
    }
    if (store->count >= MAX_USERS) {
        return "ERROR: user store full";
    }
    User* u = &store->users[store->count];
    memset(u, 0, sizeof(*u));
    snprintf(u->username, sizeof(u->username), "%s", username);
    if (RAND_bytes(u->salt, SALT_LEN) != 1) {
        return "ERROR: internal error";
    }
    if (!derive_key_pbkdf2(password, u->salt, ITERATIONS, u->hash, HASH_LEN)) {
        return "ERROR: internal error";
    }
    u->iterations = ITERATIONS;
    u->createdAt = time(NULL);
    store->count += 1;
    return "OK";
}

static int authenticate(UserStore* store, const char* username, const char* password) {
    for (int i = 0; i < store->count; i++) {
        User* u = &store->users[i];
        if (strncmp(u->username, username, USERNAME_MAX) == 0) {
            if (is_expired(u)) {
                simulate_work(password);
                return 0;
            }
            unsigned char test[HASH_LEN];
            if (!derive_key_pbkdf2(password, u->salt, u->iterations, test, HASH_LEN)) return 0;
            return constant_time_equal(test, u->hash, HASH_LEN);
        }
    }
    simulate_work(password);
    return 0;
}

int main(void) {
    UserStore store;
    memset(&store, 0, sizeof(store));

    // Test 1: Register alice with strong password
    const char* t1 = register_user(&store, "alice", "Str0ng!Passw0rd");
    printf("Test1 register alice: %s\n", t1);

    // Test 2: Authenticate alice with correct password
    int t2 = authenticate(&store, "alice", "Str0ng!Passw0rd");
    printf("Test2 auth alice correct: %s\n", t2 ? "true" : "false");

    // Test 3: Authenticate alice with wrong password
    int t3 = authenticate(&store, "alice", "WrongPass!123");
    printf("Test3 auth alice wrong: %s\n", t3 ? "true" : "false");

    // Test 4: Register bob with weak password
    const char* t4 = register_user(&store, "bob", "password");
    printf("Test4 register bob weak: %s\n", t4);

    // Test 5: Register alice again (duplicate)
    const char* t5 = register_user(&store, "alice", "An0ther$tr0ngOne");
    printf("Test5 register alice duplicate: %s\n", t5);

    return 0;
}
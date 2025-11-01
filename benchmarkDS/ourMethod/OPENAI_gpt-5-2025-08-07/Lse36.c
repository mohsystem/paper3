#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SALT_LEN 16
#define HASH_LEN 32
#define PBKDF2_ITERATIONS 210000
#define PASSWORD_MAX_AGE_SECONDS (90 * 24 * 60 * 60)
#define MAX_PASSWORD_LEN 256
#define MAX_USERNAME_LEN 64
#define MIN_USERNAME_LEN 3
#define DB_CAPACITY 128

static const char *REDIRECT_URL = "https://example.com/home";

typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    unsigned char salt[SALT_LEN];
    unsigned char password_hash[HASH_LEN];
    int iterations;
    time_t created_at;
    int in_use;
} User;

typedef struct {
    User users[DB_CAPACITY];
    size_t count;
} InMemoryDb;

static void db_init(InMemoryDb *db) {
    if (!db) return;
    memset(db, 0, sizeof(*db));
}

static User* db_find_user(InMemoryDb *db, const char *username) {
    if (!db || !username) return NULL;
    for (size_t i = 0; i < DB_CAPACITY; i++) {
        if (db->users[i].in_use && strcmp(db->users[i].username, username) == 0) {
            return &db->users[i];
        }
    }
    return NULL;
}

static int is_valid_username(const char *username) {
    size_t len;
    if (!username) return 0;
    len = strlen(username);
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '.' || c == '-')) {
            return 0;
        }
    }
    return 1;
}

static int is_strong_password(const char *password) {
    size_t len;
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
    const char *specials = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?`~\\ ";
    if (!password) return 0;
    len = strlen(password);
    if (len < 12 || len > MAX_PASSWORD_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (isupper(c)) has_upper = 1;
        else if (islower(c)) has_lower = 1;
        else if (isdigit(c)) has_digit = 1;
        else if (strchr(specials, (int)c) != NULL) has_special = 1;
    }
    return has_upper && has_lower && has_digit && has_special;
}

static int pbkdf2_hash(const char *password, const unsigned char *salt, int iterations, unsigned char *out_hash, size_t out_len) {
    size_t pwlen = strlen(password);
    // Copy password to wipe after use
    unsigned char *pwbuf = (unsigned char*)OPENSSL_malloc(pwlen);
    if (!pwbuf) return 0;
    memcpy(pwbuf, password, pwlen);
    int ok = PKCS5_PBKDF2_HMAC((const char*)pwbuf, (int)pwlen, salt, SALT_LEN, iterations, EVP_sha256(), (int)out_len, out_hash);
    OPENSSL_cleanse(pwbuf, pwlen);
    OPENSSL_free(pwbuf);
    return ok == 1;
}

static int create_user(InMemoryDb *db, const char *username, const char *password) {
    if (!db || !username || !password) return 0;
    if (!is_valid_username(username)) return 0;
    if (!is_strong_password(password)) return 0;
    if (db_find_user(db, username) != NULL) return 0;

    // Find free slot
    size_t idx;
    for (idx = 0; idx < DB_CAPACITY; idx++) {
        if (!db->users[idx].in_use) break;
    }
    if (idx == DB_CAPACITY) return 0;

    User *u = &db->users[idx];
    memset(u, 0, sizeof(*u));
    strncpy(u->username, username, MAX_USERNAME_LEN);
    u->username[MAX_USERNAME_LEN] = '\0';
    if (RAND_bytes(u->salt, SALT_LEN) != 1) {
        memset(u, 0, sizeof(*u));
        return 0;
    }
    if (!pbkdf2_hash(password, u->salt, PBKDF2_ITERATIONS, u->password_hash, HASH_LEN)) {
        memset(u, 0, sizeof(*u));
        return 0;
    }
    u->iterations = PBKDF2_ITERATIONS;
    u->created_at = time(NULL);
    u->in_use = 1;
    db->count++;
    return 1;
}

// Returns heap-allocated redirect URL on success, or NULL on failure. Caller must free().
static char* login(InMemoryDb *db, const char *username, const char *password) {
    if (!db || !username || !password) return NULL;
    if (!is_valid_username(username)) return NULL;
    if (strlen(password) == 0 || strlen(password) > MAX_PASSWORD_LEN) return NULL;

    User *u = db_find_user(db, username);
    if (!u) return NULL;

    unsigned char computed[HASH_LEN];
    if (!pbkdf2_hash(password, u->salt, u->iterations, computed, HASH_LEN)) {
        return NULL;
    }
    int match = (CRYPTO_memcmp(computed, u->password_hash, HASH_LEN) == 0);
    OPENSSL_cleanse(computed, sizeof(computed));

    time_t now = time(NULL);
    int not_expired = (now <= (u->created_at + PASSWORD_MAX_AGE_SECONDS));

    if (match && not_expired) {
        size_t len = strlen(REDIRECT_URL);
        char *out = (char*)malloc(len + 1);
        if (!out) return NULL;
        memcpy(out, REDIRECT_URL, len + 1);
        return out;
    }
    return NULL;
}

static char* generate_strong_password(size_t length) {
    if (length < 12) length = 12;
    const char *upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *lower = "abcdefghijklmnopqrstuvwxyz";
    const char *digits = "0123456789";
    const char *special = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?`~\\";
    const char *all = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_=+[]{}|;:'\",.<>/?`~\\";
    size_t all_len = strlen(all);

    char *pwd = (char*)malloc(length + 1);
    if (!pwd) return NULL;

    // Ensure one from each category
    unsigned char r;
    RAND_bytes(&r, 1); pwd[0] = upper[r % strlen(upper)];
    RAND_bytes(&r, 1); pwd[1] = lower[r % strlen(lower)];
    RAND_bytes(&r, 1); pwd[2] = digits[r % strlen(digits)];
    RAND_bytes(&r, 1); pwd[3] = special[r % strlen(special)];
    for (size_t i = 4; i < length; i++) {
        RAND_bytes(&r, 1);
        pwd[i] = all[r % all_len];
    }
    // Fisher-Yates shuffle
    for (size_t i = length - 1; i > 0; i--) {
        RAND_bytes(&r, 1);
        size_t j = r % (i + 1);
        char tmp = pwd[i];
        pwd[i] = pwd[j];
        pwd[j] = tmp;
    }
    pwd[length] = '\0';
    return pwd;
}

int main(void) {
    InMemoryDb db;
    db_init(&db);

    // Test 1: Create user and successful login
    const char *user1 = "alice_1";
    char *pass1 = generate_strong_password(16);
    int created1 = create_user(&db, user1, pass1);
    char *res1 = login(&db, user1, pass1);
    printf("Test1_created=%s loginSuccess=%s\n", created1 ? "true" : "false", (res1 != NULL) ? "true" : "false");
    if (res1) free(res1);
    OPENSSL_cleanse(pass1, strlen(pass1)); free(pass1);

    // Test 2: Wrong password
    char *res2 = login(&db, user1, "WrongPass123!");
    printf("Test2_wrongPassword=%s\n", (res2 == NULL) ? "true" : "false");
    if (res2) free(res2);

    // Test 3: Non-existent user
    char *res3 = login(&db, "nonexistent", "SomeStrong!2345");
    printf("Test3_nonexistent=%s\n", (res3 == NULL) ? "true" : "false");
    if (res3) free(res3);

    // Test 4: Invalid username format
    char *res4 = login(&db, "../baduser", "ValidPass!2345");
    printf("Test4_invalidUsername=%s\n", (res4 == NULL) ? "true" : "false");
    if (res4) free(res4);

    // Test 5: Expired password
    const char *user2 = "bob-2";
    char *pass2 = generate_strong_password(18);
    int created2 = create_user(&db, user2, pass2);
    User *u2 = db_find_user(&db, user2);
    if (u2) {
        u2->created_at = time(NULL) - (PASSWORD_MAX_AGE_SECONDS + 3600);
    }
    char *res5 = login(&db, user2, pass2);
    printf("Test5_expiredPassword_userCreated=%s loginBlocked=%s\n", created2 ? "true" : "false", (res5 == NULL) ? "true" : "false");
    if (res5) free(res5);
    OPENSSL_cleanse(pass2, strlen(pass2)); free(pass2);

    return 0;
}
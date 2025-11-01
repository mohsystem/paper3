#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#define ITERATIONS_C 120000
#define KEY_LEN_C 32
#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 128

typedef struct {
    const char *username;
    unsigned char salt[16];
    unsigned char hash[KEY_LEN_C];
    int iterations;
    int initialized;
} UserRecordC;

static UserRecordC db_c[8]; // simple fixed-size "database"
static size_t db_c_count = 0;

static int pbkdf2_sha256_c(const char *password, const unsigned char *salt, int salt_len,
                           int iterations, int key_len, unsigned char *out) {
    return PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, salt_len,
                             iterations, EVP_sha256(), key_len, out);
}

static int constant_time_equals_c(const unsigned char *a, size_t alen,
                                  const unsigned char *b, size_t blen) {
    size_t maxlen = alen > blen ? alen : blen;
    unsigned int diff = 0;
    for (size_t i = 0; i < maxlen; ++i) {
        unsigned char x = (i < alen) ? a[i] : 0;
        unsigned char y = (i < blen) ? b[i] : 0;
        diff |= (unsigned int)(x ^ y);
    }
    return (diff == 0) && (alen == blen);
}

static void add_user_c(const char *username, const char *password, const unsigned char salt[16], int iterations) {
    if (db_c_count >= sizeof(db_c)/sizeof(db_c[0])) return;
    UserRecordC *rec = &db_c[db_c_count++];
    rec->username = username;
    memcpy(rec->salt, salt, 16);
    rec->iterations = iterations;
    rec->initialized = 1;
    pbkdf2_sha256_c(password, rec->salt, 16, rec->iterations, KEY_LEN_C, rec->hash);
}

static UserRecordC* find_user_c(const char *username) {
    for (size_t i = 0; i < db_c_count; ++i) {
        if (db_c[i].initialized && strcmp(db_c[i].username, username) == 0) {
            return &db_c[i];
        }
    }
    return NULL;
}

// returns "/home" on success, otherwise returns NULL
static const char* login(const char *username, const char *password) {
    if (!username || !password) return NULL;
    size_t ulen = strlen(username);
    size_t plen = strlen(password);
    if (ulen == 0 || ulen > MAX_USERNAME_LEN) return NULL;
    if (plen > MAX_PASSWORD_LEN) return NULL;

    UserRecordC *rec = find_user_c(username);
    if (!rec) {
        static const unsigned char dummy_salt[16] = {
            0xA3,0x51,0x5C,0x09,0x7E,0x22,0xA9,0x41,0x13,0xF1,0xB2,0x08,0xCD,0x44,0x9A,0x7F
        };
        unsigned char d1[KEY_LEN_C], d2[KEY_LEN_C];
        pbkdf2_sha256_c(password, dummy_salt, 16, ITERATIONS_C, KEY_LEN_C, d1);
        pbkdf2_sha256_c(password, dummy_salt, 16, ITERATIONS_C, KEY_LEN_C, d2);
        (void)constant_time_equals_c(d1, KEY_LEN_C, d2, KEY_LEN_C);
        return NULL;
    }

    unsigned char computed[KEY_LEN_C];
    pbkdf2_sha256_c(password, rec->salt, 16, rec->iterations, KEY_LEN_C, computed);
    if (constant_time_equals_c(computed, KEY_LEN_C, rec->hash, KEY_LEN_C)) {
        return "/home";
    }
    return NULL;
}

static void initialize_db_c(void) {
    const unsigned char saltAlice[16] = {
        0x10,0x2F,0x3A,0x4B,0x5C,0x6D,0x7E,0x8F,0x90,0xAB,0xBC,0xCD,0xDE,0xEF,0x01,0x23
    };
    add_user_c("alice", "correct horse battery staple", saltAlice, ITERATIONS_C);

    const unsigned char saltBob[16] = {
        0x33,0x55,0x77,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x12,0x34,0x56,0x78,0x9A,0xBC
    };
    add_user_c("bob", "Tr0ub4dor&3", saltBob, ITERATIONS_C);
}

int main(void) {
    initialize_db_c();
    struct Test { const char *label; const char *user; const char *pwd; } tests[] = {
        {"Test1 - correct credentials", "alice", "correct horse battery staple"},
        {"Test2 - wrong password", "alice", "wrong password"},
        {"Test3 - unknown user", "charlie", "anything"},
        {"Test4 - empty username", "", "any"},
        {"Test5 - injection-like username", "alice' OR '1'='1", "foo"},
    };
    size_t n = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < n; ++i) {
        const char *res = login(tests[i].user, tests[i].pwd);
        printf("%s: %s\n", tests[i].label, (res && strcmp(res, "/home") == 0) ? "redirect:/home" : "no-redirect");
    }
    return 0;
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define MAX_USERS 32
#define MAX_SESSIONS 32
#define SALT_LEN 16
#define HASH_LEN 32
#define TOKEN_LEN 32 /* 32 bytes -> 64 hex chars */
#define PBKDF2_ITERS 210000

typedef struct {
    char *email; /* dynamically allocated */
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    int in_use;
} User;

typedef struct {
    char token[2 * TOKEN_LEN + 1]; /* hex string */
    int user_index; /* index into users array */
    int in_use;
} Session;

static User users[MAX_USERS];
static Session sessions[MAX_SESSIONS];

static void secure_zero(void *v, size_t n) {
    if (v && n) OPENSSL_cleanse(v, n);
}

static int hex_encode(const unsigned char *in, size_t inlen, char *out, size_t outlen) {
    static const char *hex = "0123456789abcdef";
    if (outlen < inlen * 2 + 1) return 0;
    for (size_t i = 0; i < inlen; i++) {
        out[2*i] = hex[(in[i] >> 4) & 0xF];
        out[2*i + 1] = hex[in[i] & 0xF];
    }
    out[inlen * 2] = '\0';
    return 1;
}

static int is_valid_email(const char *email) {
    if (!email) return 0;
    size_t len = strlen(email);
    if (len < 5 || len > 254) return 0;
    int at_count = 0;
    const char *at_pos = NULL;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)email[i];
        if (c < 33 || c > 126) {
            if (c != ' ') return 0;
        }
        if (email[i] == '@') {
            at_count++;
            at_pos = &email[i];
        }
    }
    if (at_count != 1 || !at_pos) return 0;
    const char *dot = strrchr(at_pos, '.');
    if (!dot || dot <= at_pos + 1 || dot >= email + len - 2) return 0;
    return 1;
}

static int is_strong_password(const char *pwd) {
    if (!pwd) return 0;
    size_t len = strlen(pwd);
    if (len < 10 || len > 512) return 0;
    int up=0, lo=0, di=0, sp=0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)pwd[i];
        if (isupper(c)) up = 1;
        else if (islower(c)) lo = 1;
        else if (isdigit(c)) di = 1;
        else sp = 1;
    }
    return up && lo && di && sp;
}

static int find_user_by_email(const char *email) {
    if (!email) return -1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].in_use && users[i].email && strcasecmp(users[i].email, email) == 0) {
            return i;
        }
    }
    return -1;
}

static int allocate_user_slot(void) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].in_use) return i;
    }
    return -1;
}

static int allocate_session_slot(void) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!sessions[i].in_use) return i;
    }
    return -1;
}

static int pbkdf2_hash(const char *password, const unsigned char *salt, size_t salt_len, unsigned char *out, size_t out_len) {
    if (!password || !salt || !out) return 0;
    if (salt_len > INT_MAX || out_len > INT_MAX) return 0;
    int passlen = (int)strlen(password);
    if (passlen < 0) return 0;
    int rc = PKCS5_PBKDF2_HMAC(password, passlen, salt, (int)salt_len, PBKDF2_ITERS, EVP_sha256(), (int)out_len, out);
    return rc == 1;
}

static void free_user(int idx) {
    if (idx < 0 || idx >= MAX_USERS) return;
    if (users[idx].in_use) {
        if (users[idx].email) {
            size_t l = strlen(users[idx].email);
            secure_zero(users[idx].email, l);
            free(users[idx].email);
            users[idx].email = NULL;
        }
        secure_zero(users[idx].salt, SALT_LEN);
        secure_zero(users[idx].hash, HASH_LEN);
        users[idx].in_use = 0;
    }
}

int register_user(const char *email, const char *password) {
    if (!is_valid_email(email) || !is_strong_password(password)) return 0;
    if (find_user_by_email(email) >= 0) return 0;
    int slot = allocate_user_slot();
    if (slot < 0) return 0;
    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) return 0;
    unsigned char hash[HASH_LEN];
    if (!pbkdf2_hash(password, salt, SALT_LEN, hash, HASH_LEN)) return 0;

    size_t elen = strlen(email);
    char *e = (char*)malloc(elen + 1);
    if (!e) return 0;
    memcpy(e, email, elen + 1);

    users[slot].email = e;
    memcpy(users[slot].salt, salt, SALT_LEN);
    memcpy(users[slot].hash, hash, HASH_LEN);
    users[slot].in_use = 1;

    secure_zero(salt, SALT_LEN);
    secure_zero(hash, HASH_LEN);
    return 1;
}

int login_user(const char *email, const char *password, char *out_session, size_t out_len) {
    if (!is_valid_email(email) || !password || !out_session || out_len < (2 * TOKEN_LEN + 1)) return 0;
    int idx = find_user_by_email(email);
    if (idx < 0) return 0;
    unsigned char cand[HASH_LEN];
    if (!pbkdf2_hash(password, users[idx].salt, SALT_LEN, cand, HASH_LEN)) return 0;
    int ok = CRYPTO_memcmp(cand, users[idx].hash, HASH_LEN) == 0;
    secure_zero(cand, sizeof(cand));
    if (!ok) return 0;

    int sslot = allocate_session_slot();
    if (sslot < 0) return 0;
    unsigned char tok[TOKEN_LEN];
    if (RAND_bytes(tok, TOKEN_LEN) != 1) return 0;
    if (!hex_encode(tok, TOKEN_LEN, sessions[sslot].token, sizeof(sessions[sslot].token))) {
        secure_zero(tok, TOKEN_LEN);
        return 0;
    }
    sessions[sslot].user_index = idx;
    sessions[sslot].in_use = 1;
    secure_zero(tok, TOKEN_LEN);
    snprintf(out_session, out_len, "%s", sessions[sslot].token);
    return 1;
}

static int find_session(const char *token) {
    if (!token) return -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].in_use && strncmp(sessions[i].token, token, 2 * TOKEN_LEN) == 0) {
            return i;
        }
    }
    return -1;
}

const char* change_email(const char *session_token, const char *old_email, const char *new_email, const char *confirm_password) {
    if (!session_token || !old_email || !new_email || !confirm_password) return "Invalid input";
    int sidx = find_session(session_token);
    if (sidx < 0) return "Not logged in";
    if (!is_valid_email(old_email) || !is_valid_email(new_email)) return "Invalid email";
    if (strcasecmp(old_email, new_email) == 0) return "New email must be different";

    int uidx = find_user_by_email(old_email);
    if (uidx < 0) return "Email not found";
    if (sessions[sidx].user_index != uidx) return "Session does not match user";

    unsigned char cand[HASH_LEN];
    if (!pbkdf2_hash(confirm_password, users[uidx].salt, SALT_LEN, cand, HASH_LEN)) return "Invalid credentials";
    int ok = CRYPTO_memcmp(cand, users[uidx].hash, HASH_LEN) == 0;
    secure_zero(cand, sizeof(cand));
    if (!ok) return "Invalid credentials";

    if (find_user_by_email(new_email) >= 0) return "Email already in use";

    /* Perform change: allocate new string, free old */
    size_t nlen = strlen(new_email);
    char *ne = (char*)malloc(nlen + 1);
    if (!ne) return "Internal error";
    memcpy(ne, new_email, nlen + 1);

    if (users[uidx].email) {
        size_t olen = strlen(users[uidx].email);
        secure_zero(users[uidx].email, olen);
        free(users[uidx].email);
    }
    users[uidx].email = ne;

    return "Email changed";
}

int main(void) {
    /* Test cases */
    const char *pass1 = getenv("USER1_PASS") ? getenv("USER1_PASS") : "Str0ng!Passw0rd";
    const char *pass2 = getenv("USER2_PASS") ? getenv("USER2_PASS") : "An0ther#Strong1";
    const char *pass3 = getenv("USER3_PASS") ? getenv("USER3_PASS") : "Third#Str0ngP4ss";

    printf("Register user1: %d\n", register_user("alice@example.com", pass1));
    printf("Register user2: %d\n", register_user("bob@example.com", pass2));
    printf("Duplicate user1: %d\n", register_user("alice@example.com", pass1));

    char token1[2 * TOKEN_LEN + 1];
    int login_ok = login_user("alice@example.com", pass1, token1, sizeof(token1));
    printf("Login user1 ok: %d\n", login_ok);
    const char *res1 = change_email(login_ok ? token1 : "bad", "alice@example.com", "alice.new@example.com", pass1);
    printf("Change email success: %s\n", res1);

    const char *res2 = change_email("badtoken", "bob@example.com", "bob.new@example.com", pass2);
    printf("Change without login: %s\n", res2);

    char token2[2 * TOKEN_LEN + 1];
    int login2_ok = login_user("bob@example.com", pass2, token2, sizeof(token2));
    const char *res3 = change_email(login2_ok ? token2 : "bad", "alice.new@example.com", "bob.changed@example.com", pass2);
    printf("Wrong old email: %s\n", res3);

    const char *res4 = change_email(login2_ok ? token2 : "bad", "bob@example.com", "robert@example.com", "WrongP@ss1");
    printf("Wrong password: %s\n", res4);

    printf("Register user3: %d\n", register_user("carol@example.com", pass3));
    char token3[2 * TOKEN_LEN + 1];
    printf("Login user3 ok: %d\n", login_user("carol@example.com", pass3, token3, sizeof(token3)));
    const char *res5 = change_email(login2_ok ? token2 : "bad", "bob@example.com", "alice.new@example.com", pass2);
    printf("Email already used: %s\n", res5);

    /* Cleanup allocated user emails */
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].in_use) {
            free_user(i);
        }
    }
    /* Clear sessions */
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].in_use) {
            secure_zero(sessions[i].token, sizeof(sessions[i].token));
            sessions[i].in_use = 0;
        }
    }
    return 0;
}
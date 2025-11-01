#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define MAX_USERS 32
#define MAX_SESSIONS 64
#define USERNAME_MAX 64
#define ROLE_MAX 6
#define SALT_LEN 16
#define KEY_LEN 32
#define ITERATIONS 210000
#define TOKEN_BYTES 32
#define TOKEN_HEX_LEN (TOKEN_BYTES*2)

typedef struct {
    int in_use;
    char username[USERNAME_MAX+1];
    unsigned char salt[SALT_LEN];
    unsigned char hash[KEY_LEN];
    int iterations;
    char role[ROLE_MAX+1];
} user_record_t;

typedef struct {
    int in_use;
    char token[TOKEN_HEX_LEN+1];
    char username[USERNAME_MAX+1];
} session_record_t;

static user_record_t USERS[MAX_USERS];
static session_record_t SESSIONS[MAX_SESSIONS];

static int is_valid_username(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len < 3 || len > USERNAME_MAX) return 0;
    for (size_t i=0;i<len;i++) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c=='_' || c=='-' || c=='.')) return 0;
    }
    return 1;
}

static int is_valid_role(const char* r) {
    if (!r) return 0;
    return (strcmp(r,"user")==0 || strcmp(r,"admin")==0);
}

static int password_policy(const char* pw) {
    if (!pw) return 0;
    size_t len = strlen(pw);
    if (len < 12 || len > 128) return 0;
    int up=0, lo=0, di=0, sy=0;
    for (size_t i=0;i<len;i++) {
        unsigned char c = (unsigned char)pw[i];
        if (isupper(c)) up=1;
        else if (islower(c)) lo=1;
        else if (isdigit(c)) di=1;
        else sy=1;
    }
    return up && lo && di && sy;
}

static int pbkdf2_sha256(const char* password, const unsigned char* salt, int salt_len, int iterations, unsigned char* out, int out_len) {
    if (!password || !salt || !out) return 0;
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                          salt, salt_len,
                          iterations, EVP_sha256(),
                          out_len, out) != 1) {
        return 0;
    }
    return 1;
}

static void hex_encode(const unsigned char* in, size_t len, char* out, size_t out_len) {
    static const char* hex = "0123456789abcdef";
    if (out_len < (len*2 + 1)) return;
    for (size_t i=0;i<len;i++) {
        out[2*i]   = hex[(in[i] >> 4) & 0xF];
        out[2*i+1] = hex[in[i] & 0xF];
    }
    out[len*2] = '\0';
}

static int register_user(const char* username, const char* password, const char* role) {
    if (!is_valid_username(username) || !is_valid_role(role) || !password_policy(password)) {
        return 0;
    }
    int idx = -1;
    for (int i=0;i<MAX_USERS;i++) {
        if (!USERS[i].in_use) { idx = i; break; }
    }
    if (idx < 0) return 0;
    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return 0;
    }
    unsigned char hash[KEY_LEN];
    if (!pbkdf2_sha256(password, salt, SALT_LEN, ITERATIONS, hash, KEY_LEN)) {
        return 0;
    }
    USERS[idx].in_use = 1;
    strncpy(USERS[idx].username, username, USERNAME_MAX);
    USERS[idx].username[USERNAME_MAX] = '\0';
    memcpy(USERS[idx].salt, salt, SALT_LEN);
    memcpy(USERS[idx].hash, hash, KEY_LEN);
    USERS[idx].iterations = ITERATIONS;
    strncpy(USERS[idx].role, role, ROLE_MAX);
    USERS[idx].role[ROLE_MAX] = '\0';
    OPENSSL_cleanse(salt, sizeof(salt));
    OPENSSL_cleanse(hash, sizeof(hash));
    return 1;
}

static int find_user(const char* username) {
    for (int i=0;i<MAX_USERS;i++) {
        if (USERS[i].in_use && strncmp(USERS[i].username, username, USERNAME_MAX)==0) return i;
    }
    return -1;
}

static int add_session(const char* token, const char* username) {
    for (int i=0;i<MAX_SESSIONS;i++) {
        if (!SESSIONS[i].in_use) {
            SESSIONS[i].in_use = 1;
            strncpy(SESSIONS[i].token, token, TOKEN_HEX_LEN);
            SESSIONS[i].token[TOKEN_HEX_LEN] = '\0';
            strncpy(SESSIONS[i].username, username, USERNAME_MAX);
            SESSIONS[i].username[USERNAME_MAX] = '\0';
            return 1;
        }
    }
    return 0;
}

static int find_session(const char* token) {
    for (int i=0;i<MAX_SESSIONS;i++) {
        if (SESSIONS[i].in_use && strncmp(SESSIONS[i].token, token, TOKEN_HEX_LEN)==0) return i;
    }
    return -1;
}

// Endpoints
const char* login(const char* request_id) {
    static char resp[128] = {0};
    if (!request_id || strlen(request_id) > 128) {
        snprintf(resp, sizeof(resp), "400 Bad Request");
        return resp;
    }
    snprintf(resp, sizeof(resp), "Login endpoint (GET). Please POST to do_login.");
    return resp;
}

char* do_login(const char* username, const char* password) {
    if (!is_valid_username(username) || !password || strlen(password) == 0) {
        return NULL;
    }
    int idx = find_user(username);
    if (idx < 0) return NULL;
    unsigned char calc[KEY_LEN];
    if (!pbkdf2_sha256(password, USERS[idx].salt, SALT_LEN, USERS[idx].iterations, calc, KEY_LEN)) {
        return NULL;
    }
    int ok = (CRYPTO_memcmp(calc, USERS[idx].hash, KEY_LEN) == 0);
    OPENSSL_cleanse(calc, sizeof(calc));
    if (!ok) return NULL;

    unsigned char tok_bytes[TOKEN_BYTES];
    if (RAND_bytes(tok_bytes, TOKEN_BYTES) != 1) {
        return NULL;
    }
    char token[TOKEN_HEX_LEN+1];
    hex_encode(tok_bytes, TOKEN_BYTES, token, sizeof(token));
    OPENSSL_cleanse(tok_bytes, sizeof(tok_bytes));

    if (!add_session(token, username)) {
        return NULL;
    }
    char* out = (char*)malloc(TOKEN_HEX_LEN + 1);
    if (!out) return NULL;
    strncpy(out, token, TOKEN_HEX_LEN);
    out[TOKEN_HEX_LEN] = '\0';
    return out;
}

char* index(const char* session_token) {
    const char* unauthorized = "401 Unauthorized";
    if (!session_token) {
        char* out = (char*)malloc(strlen(unauthorized)+1);
        if (!out) return NULL;
        strcpy(out, unauthorized);
        return out;
    }
    int si = find_session(session_token);
    if (si < 0) {
        char* out = (char*)malloc(strlen(unauthorized)+1);
        if (!out) return NULL;
        strcpy(out, unauthorized);
        return out;
    }
    const char* user = SESSIONS[si].username;
    int ui = find_user(user);
    if (ui < 0) {
        char* out = (char*)malloc(strlen(unauthorized)+1);
        if (!out) return NULL;
        strcpy(out, unauthorized);
        return out;
    }
    char buf[256];
    snprintf(buf, sizeof(buf), "Welcome, %s (role=%s)", USERS[ui].username, USERS[ui].role);
    char* out = (char*)malloc(strlen(buf)+1);
    if (!out) return NULL;
    strcpy(out, buf);
    return out;
}

char* user_page(const char* session_token) {
    const char* unauthorized = "401 Unauthorized";
    if (!session_token) {
        char* out = (char*)malloc(strlen(unauthorized)+1);
        if (!out) return NULL;
        strcpy(out, unauthorized);
        return out;
    }
    int si = find_session(session_token);
    if (si < 0) {
        char* out = (char*)malloc(strlen(unauthorized)+1);
        if (!out) return NULL;
        strcpy(out, unauthorized);
        return out;
    }
    const char* user = SESSIONS[si].username;
    char buf[256];
    snprintf(buf, sizeof(buf), "User page for %s", user);
    char* out = (char*)malloc(strlen(buf)+1);
    if (!out) return NULL;
    strcpy(out, buf);
    return out;
}

char* admin_page(const char* session_token) {
    const char* unauthorized = "401 Unauthorized";
    const char* forbidden = "403 Forbidden";
    if (!session_token) {
        char* out = (char*)malloc(strlen(unauthorized)+1);
        if (!out) return NULL;
        strcpy(out, unauthorized);
        return out;
    }
    int si = find_session(session_token);
    if (si < 0) {
        char* out = (char*)malloc(strlen(unauthorized)+1);
        if (!out) return NULL;
        strcpy(out, unauthorized);
        return out;
    }
    const char* user = SESSIONS[si].username;
    int ui = find_user(user);
    if (ui < 0) {
        char* out = (char*)malloc(strlen(unauthorized)+1);
        if (!out) return NULL;
        strcpy(out, unauthorized);
        return out;
    }
    if (strcmp(USERS[ui].role, "admin") != 0) {
        char* out = (char*)malloc(strlen(forbidden)+1);
        if (!out) return NULL;
        strcpy(out, forbidden);
        return out;
    }
    char buf[256];
    snprintf(buf, sizeof(buf), "Admin page for %s", user);
    char* out = (char*)malloc(strlen(buf)+1);
    if (!out) return NULL;
    strcpy(out, buf);
    return out;
}

// Test helpers
static char* gen_strong_password(size_t length) {
    if (length < 12) length = 12;
    const char* U = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char* L = "abcdefghijklmnopqrstuvwxyz";
    const char* D = "0123456789";
    const char* S = "!@#$%^&*()-_=+[]{}:,.?/";
    const char* ALL = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_=+[]{}:,.?/";

    char* pw = (char*)malloc(length + 1);
    if (!pw) return NULL;
    size_t idx = 0;

    unsigned char r;
    RAND_bytes(&r, 1); pw[idx++] = U[r % 26];
    RAND_bytes(&r, 1); pw[idx++] = L[r % 26];
    RAND_bytes(&r, 1); pw[idx++] = D[r % 10];
    RAND_bytes(&r, 1); pw[idx++] = S[r % 24];

    while (idx < length) {
        RAND_bytes(&r, 1);
        pw[idx++] = ALL[r % strlen(ALL)];
    }
    // Fisher-Yates shuffle
    for (size_t i = length - 1; i > 0; --i) {
        unsigned int rv = 0;
        RAND_bytes((unsigned char*)&rv, sizeof(rv));
        size_t j = rv % (i + 1);
        char tmp = pw[i]; pw[i] = pw[j]; pw[j] = tmp;
    }
    pw[length] = '\0';
    return pw;
}

int main(void) {
    printf("== GET /login ==\n");
    printf("%s\n", login("req-1"));

    char* pwUserA = gen_strong_password(16);
    char* pwAdminA = gen_strong_password(18);
    char* pwUserB = gen_strong_password(20);

    // Test 1
    printf("\n== Test 1: userA successful login ==\n");
    register_user("userA", pwUserA, "user");
    char* tok1 = do_login("userA", pwUserA);
    printf("Token: %s\n", tok1 ? tok1 : "(login failed)");
    char* idx1 = index(tok1 ? tok1 : "");
    printf("Index: %s\n", idx1 ? idx1 : "(null)");
    char* up1 = user_page(tok1 ? tok1 : "");
    printf("User page: %s\n", up1 ? up1 : "(null)");
    char* ap1 = admin_page(tok1 ? tok1 : "");
    printf("Admin page: %s\n", ap1 ? ap1 : "(null)");

    // Test 2
    printf("\n== Test 2: wrong password ==\n");
    char* tok2 = do_login("userA", "WrongPass123!");
    printf("Token: %s\n", tok2 ? tok2 : "(login failed)");

    // Test 3
    printf("\n== Test 3: adminA successful admin access ==\n");
    register_user("adminA", pwAdminA, "admin");
    char* tok3 = do_login("adminA", pwAdminA);
    printf("Token: %s\n", tok3 ? tok3 : "(login failed)");
    char* ap3 = admin_page(tok3 ? tok3 : "");
    printf("Admin page: %s\n", ap3 ? ap3 : "(null)");

    // Test 4
    printf("\n== Test 4: invalid token access ==\n");
    char* idx4 = index("deadbeef");
    printf("Index: %s\n", idx4 ? idx4 : "(null)");
    char* ap4 = admin_page("deadbeef");
    printf("Admin page: %s\n", ap4 ? ap4 : "(null)");

    // Test 5
    printf("\n== Test 5: userB normal access, admin denied ==\n");
    register_user("userB", pwUserB, "user");
    char* tok5 = do_login("userB", pwUserB);
    printf("Token: %s\n", tok5 ? tok5 : "(login failed)");
    char* idx5 = index(tok5 ? tok5 : "");
    printf("Index: %s\n", idx5 ? idx5 : "(null)");
    char* ap5 = admin_page(tok5 ? tok5 : "");
    printf("Admin page: %s\n", ap5 ? ap5 : "(null)");

    // Free allocated memory
    free(pwUserA);
    free(pwAdminA);
    free(pwUserB);
    if (tok1) free(tok1);
    if (idx1) free(idx1);
    if (up1) free(up1);
    if (ap1) free(ap1);
    if (tok2) free(tok2);
    if (tok3) free(tok3);
    if (ap3) free(ap3);
    if (idx4) free(idx4);
    if (ap4) free(ap4);
    if (tok5) free(tok5);
    if (idx5) free(idx5);
    if (ap5) free(ap5);

    return 0;
}
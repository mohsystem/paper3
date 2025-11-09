// Secure minimal demo of index, login, and secret pages with in-memory session handling.
// Uses OpenSSL for cryptography (PBKDF2-HMAC-SHA256 and CSPRNG).
// Build example (may vary by system): cc -Wall -Wextra -Werror -O2 demo.c -lcrypto -o demo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define MAX_USERNAME_LEN 32
#define MIN_PASSWORD_LEN 12
#define MAX_PASSWORD_LEN 128

#define SALT_LEN 16
#define HASH_LEN 32
#define PBKDF2_ITERATIONS 210000

#define SESSION_TOKEN_BYTES 32
#define SESSION_TOKEN_HEX_LEN (SESSION_TOKEN_BYTES * 2)

#define MAX_SESSIONS 16

// ----- Utility secure helpers -----
static void secure_zero(void *p, size_t n) {
    if (p && n > 0) {
        OPENSSL_cleanse(p, n);
    }
}

static size_t safe_strlen(const char *s, size_t max) {
    if (!s) return 0;
    size_t i = 0;
    for (; i < max && s[i] != '\0'; ++i) {}
    return i;
}

static int bin_to_hex(const unsigned char *bin, size_t bin_len, char *hex, size_t hex_len) {
    static const char *hexchars = "0123456789abcdef";
    if (!bin || !hex) return -1;
    if (hex_len < (bin_len * 2 + 1)) return -1;
    for (size_t i = 0; i < bin_len; ++i) {
        hex[i * 2]     = hexchars[(bin[i] >> 4) & 0xF];
        hex[i * 2 + 1] = hexchars[bin[i] & 0xF];
    }
    hex[bin_len * 2] = '\0';
    return 0;
}

static int is_hex(const char *s) {
    if (!s) return 0;
    for (const char *p = s; *p; ++p) {
        if (!((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F'))) return 0;
    }
    return 1;
}

static int is_valid_username(const char *u) {
    if (!u) return 0;
    size_t len = safe_strlen(u, MAX_USERNAME_LEN + 1);
    if (len == 0 || len > MAX_USERNAME_LEN) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '.' || c == '-')) return 0;
    }
    return 1;
}

static int password_policy_ok(const char *pw) {
    if (!pw) return 0;
    size_t len = safe_strlen(pw, MAX_PASSWORD_LEN + 1);
    if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) return 0;
    int has_l = 0, has_u = 0, has_d = 0, has_s = 0;
    const char *specials = "!@#$%^&*()-_=+[]{};:,.?/\\|~`";
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)pw[i];
        if (islower(c)) has_l = 1;
        else if (isupper(c)) has_u = 1;
        else if (isdigit(c)) has_d = 1;
        else if (strchr(specials, (char)c) != NULL) has_s = 1;
        else {
            // Reject non-printable or strange control characters
            if (!isprint(c)) return 0;
        }
    }
    return has_l && has_u && has_d && has_s;
}

static int derive_key_pbkdf2(const char *password, const unsigned char *salt, size_t salt_len, unsigned char out_key[HASH_LEN]) {
    if (!password || !salt || !out_key) return -1;
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, (int)salt_len, PBKDF2_ITERATIONS, EVP_sha256(), HASH_LEN, out_key) != 1) {
        return -1;
    }
    return 0;
}

// ----- Auth and session state -----
typedef struct {
    int initialized;
    char admin_user[MAX_USERNAME_LEN + 1];
    unsigned char salt[SALT_LEN];
    unsigned char pw_hash[HASH_LEN];
} auth_state_t;

typedef struct {
    int active;
    char token_hex[SESSION_TOKEN_HEX_LEN + 1];
    char username[MAX_USERNAME_LEN + 1];
} session_t;

static auth_state_t g_auth = {0};
static session_t g_sessions[MAX_SESSIONS] = {0};

// ----- Auth functions -----
int init_auth_from_env(void) {
    const char *user = getenv("SECURE_ADMIN_USER");
    if (!user || !is_valid_username(user)) {
        // Default to "admin" if not provided; validate it
        user = "admin";
    }
    const char *pw = getenv("SECURE_ADMIN_PASSWORD");
    if (!pw) {
        return -1; // Secret must come from environment
    }
    if (!password_policy_ok(pw)) {
        return -1; // Does not meet policy
    }
    // Fill in state
    memset(g_auth.admin_user, 0, sizeof(g_auth.admin_user));
    size_t ulen = safe_strlen(user, MAX_USERNAME_LEN);
    memcpy(g_auth.admin_user, user, ulen);
    g_auth.admin_user[ulen] = '\0';

    if (RAND_bytes(g_auth.salt, SALT_LEN) != 1) {
        return -1;
    }
    if (derive_key_pbkdf2(pw, g_auth.salt, SALT_LEN, g_auth.pw_hash) != 0) {
        return -1;
    }
    g_auth.initialized = 1;
    return 0;
}

void free_auth(void) {
    if (g_auth.initialized) {
        secure_zero(g_auth.pw_hash, sizeof(g_auth.pw_hash));
        secure_zero(g_auth.salt, sizeof(g_auth.salt));
        secure_zero(g_auth.admin_user, sizeof(g_auth.admin_user));
        g_auth.initialized = 0;
    }
    for (size_t i = 0; i < MAX_SESSIONS; ++i) {
        secure_zero(g_sessions[i].token_hex, sizeof(g_sessions[i].token_hex));
        secure_zero(g_sessions[i].username, sizeof(g_sessions[i].username));
        g_sessions[i].active = 0;
    }
}

// ----- Session functions -----
static int create_session(const char *username, char *out_token_hex, size_t out_len) {
    if (!username || !out_token_hex) return -1;
    if (out_len < (SESSION_TOKEN_HEX_LEN + 1)) return -1;
    unsigned char tok[SESSION_TOKEN_BYTES];
    if (RAND_bytes(tok, sizeof(tok)) != 1) return -1;

    char hex[SESSION_TOKEN_HEX_LEN + 1];
    if (bin_to_hex(tok, sizeof(tok), hex, sizeof(hex)) != 0) {
        return -1;
    }

    // Store into sessions
    for (size_t i = 0; i < MAX_SESSIONS; ++i) {
        if (!g_sessions[i].active) {
            g_sessions[i].active = 1;
            memset(g_sessions[i].username, 0, sizeof(g_sessions[i].username));
            size_t ulen = safe_strlen(username, MAX_USERNAME_LEN);
            memcpy(g_sessions[i].username, username, ulen);
            g_sessions[i].username[ulen] = '\0';

            memset(g_sessions[i].token_hex, 0, sizeof(g_sessions[i].token_hex));
            memcpy(g_sessions[i].token_hex, hex, SESSION_TOKEN_HEX_LEN);
            g_sessions[i].token_hex[SESSION_TOKEN_HEX_LEN] = '\0';

            // Provide to caller
            memset(out_token_hex, 0, out_len);
            memcpy(out_token_hex, hex, SESSION_TOKEN_HEX_LEN);
            out_token_hex[SESSION_TOKEN_HEX_LEN] = '\0';

            secure_zero(tok, sizeof(tok));
            return 0;
        }
    }
    secure_zero(tok, sizeof(tok));
    return -1; // No free slots
}

static int session_lookup_username(const char *token_hex, char *out_username, size_t out_len) {
    if (!token_hex || !out_username) return -1;
    if (safe_strlen(token_hex, SESSION_TOKEN_HEX_LEN + 1) != SESSION_TOKEN_HEX_LEN) return -1;
    if (!is_hex(token_hex)) return -1;
    for (size_t i = 0; i < MAX_SESSIONS; ++i) {
        if (g_sessions[i].active) {
            // Constant-time compare to avoid token probing
            if (CRYPTO_memcmp(g_sessions[i].token_hex, token_hex, SESSION_TOKEN_HEX_LEN) == 0) {
                size_t ulen = safe_strlen(g_sessions[i].username, MAX_USERNAME_LEN);
                if (ulen + 1 > out_len) return -1;
                memset(out_username, 0, out_len);
                memcpy(out_username, g_sessions[i].username, ulen);
                out_username[ulen] = '\0';
                return 0;
            }
        }
    }
    return -1;
}

// ----- Page rendering -----
char *index_page(void) {
    const char *tmpl =
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head><meta charset=\"utf-8\"><title>Index</title></head>\n"
        "<body>\n"
        "<h1>Welcome</h1>\n"
        "<p><a href=\"/login\">Login</a></p>\n"
        "</body>\n"
        "</html>\n";
    size_t need = strlen(tmpl) + 1;
    char *buf = (char *)malloc(need);
    if (!buf) return NULL;
    int n = snprintf(buf, need, "%s", tmpl);
    if (n < 0 || (size_t)n >= need) {
        free(buf);
        return NULL;
    }
    return buf;
}

char *login_page(void) {
    const char *tmpl =
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head><meta charset=\"utf-8\"><title>Login</title></head>\n"
        "<body>\n"
        "<h1>Login</h1>\n"
        "<form method=\"POST\" action=\"/do_login\">\n"
        "<label>Username: <input type=\"text\" name=\"username\" maxlength=\"32\" required></label><br>\n"
        "<label>Password: <input type=\"password\" name=\"password\" maxlength=\"128\" required></label><br>\n"
        "<button type=\"submit\">Sign in</button>\n"
        "</form>\n"
        "</body>\n"
        "</html>\n";
    size_t need = strlen(tmpl) + 1;
    char *buf = (char *)malloc(need);
    if (!buf) return NULL;
    int n = snprintf(buf, need, "%s", tmpl);
    if (n < 0 || (size_t)n >= need) {
        free(buf);
        return NULL;
    }
    return buf;
}

// do_login: returns 0 on success, 1 on auth failure, -1 on validation/internal error.
// On success, out_session_token is filled and out_redirect set to "/secret".
// On failure, out_redirect is set to "/login".
int do_login(const char *username, const char *password, char *out_session_token, size_t out_session_token_len, char *out_redirect, size_t out_redirect_len) {
    if (!g_auth.initialized || !username || !password || !out_redirect) return -1;
    if (!is_valid_username(username)) {
        if (out_redirect && out_redirect_len >= 8) {
            snprintf(out_redirect, out_redirect_len, "%s", "/login");
        }
        return -1;
    }
    size_t pwlen = safe_strlen(password, MAX_PASSWORD_LEN + 1);
    if (pwlen == 0 || pwlen > MAX_PASSWORD_LEN) {
        if (out_redirect && out_redirect_len >= 8) {
            snprintf(out_redirect, out_redirect_len, "%s", "/login");
        }
        return -1;
    }

    // Verify username matches configured admin (single-user demo)
    if (strncmp(username, g_auth.admin_user, MAX_USERNAME_LEN) != 0) {
        if (out_redirect && out_redirect_len >= 8) {
            snprintf(out_redirect, out_redirect_len, "%s", "/login");
        }
        return 1;
    }

    unsigned char test_hash[HASH_LEN];
    if (derive_key_pbkdf2(password, g_auth.salt, SALT_LEN, test_hash) != 0) {
        if (out_redirect && out_redirect_len >= 8) {
            snprintf(out_redirect, out_redirect_len, "%s", "/login");
        }
        secure_zero(test_hash, sizeof(test_hash));
        return -1;
    }

    int rc = 1;
    if (CRYPTO_memcmp(test_hash, g_auth.pw_hash, HASH_LEN) == 0) {
        // Auth OK: create a session
        if (out_session_token && out_session_token_len >= (SESSION_TOKEN_HEX_LEN + 1)) {
            if (create_session(username, out_session_token, out_session_token_len) == 0) {
                if (out_redirect && out_redirect_len >= 9) {
                    snprintf(out_redirect, out_redirect_len, "%s", "/secret");
                }
                rc = 0;
            } else {
                // session creation failed
                if (out_redirect && out_redirect_len >= 8) {
                    snprintf(out_redirect, out_redirect_len, "%s", "/login");
                }
                rc = -1;
            }
        } else {
            if (out_redirect && out_redirect_len >= 8) {
                snprintf(out_redirect, out_redirect_len, "%s", "/login");
            }
            rc = -1;
        }
    } else {
        // Wrong password
        if (out_redirect && out_redirect_len >= 8) {
            snprintf(out_redirect, out_redirect_len, "%s", "/login");
        }
        rc = 1;
    }

    secure_zero(test_hash, sizeof(test_hash));
    return rc;
}

char *secret_page(const char *session_token_hex) {
    // Validate session
    char uname[MAX_USERNAME_LEN + 1];
    memset(uname, 0, sizeof(uname));
    if (!session_token_hex || session_lookup_username(session_token_hex, uname, sizeof(uname)) != 0) {
        const char *tmpl =
            "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head><meta charset=\"utf-8\"><title>Redirect</title></head>\n"
            "<body>\n"
            "<p>Redirecting to <a href=\"/login\">/login</a></p>\n"
            "</body>\n"
            "</html>\n";
        size_t need = strlen(tmpl) + 1;
        char *buf = (char *)malloc(need);
        if (!buf) return NULL;
        int n = snprintf(buf, need, "%s", tmpl);
        if (n < 0 || (size_t)n >= need) {
            free(buf);
            return NULL;
        }
        return buf;
    }

    const char *tmpl_head =
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head><meta charset=\"utf-8\"><title>Secret</title></head>\n"
        "<body>\n"
        "<h1>Secret settings</h1>\n"
        "<p>Welcome, ";
    const char *tmpl_tail =
        ".</p>\n"
        "</body>\n"
        "</html>\n";
    size_t need = strlen(tmpl_head) + strlen(tmpl_tail) + strlen(uname) + 1;
    char *buf = (char *)malloc(need);
    if (!buf) return NULL;
    int n = snprintf(buf, need, "%s%s%s", tmpl_head, uname, tmpl_tail);
    if (n < 0 || (size_t)n >= need) {
        free(buf);
        return NULL;
    }
    return buf;
}

// ----- Test helpers -----
static char *generate_strong_password(size_t len) {
    const char *lower = "abcdefghijklmnopqrstuvwxyz";
    const char *upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *digits = "0123456789";
    const char *special = "!@#$%^&*()-_=+[]{};:,.?/\\|~`";
    const char *all = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{};:,.?/\\|~`";

    if (len < MIN_PASSWORD_LEN) len = MIN_PASSWORD_LEN;

    char *pw = (char *)malloc(len + 1);
    if (!pw) return NULL;
    memset(pw, 0, len + 1);

    // Ensure at least one from each category
    unsigned char b[4];
    if (RAND_bytes(b, sizeof(b)) != 1) {
        free(pw);
        return NULL;
    }
    pw[0] = lower[b[0] % strlen(lower)];
    pw[1] = upper[b[1] % strlen(upper)];
    pw[2] = digits[b[2] % strlen(digits)];
    pw[3] = special[b[3] % strlen(special)];

    for (size_t i = 4; i < len; ++i) {
        unsigned char x;
        if (RAND_bytes(&x, 1) != 1) {
            secure_zero(pw, len + 1);
            free(pw);
            return NULL;
        }
        pw[i] = all[x % strlen(all)];
    }

    // Simple Fisher-Yates shuffle to mix the first 4 guaranteed chars
    for (size_t i = len - 1; i > 0; --i) {
        unsigned char x;
        if (RAND_bytes(&x, 1) != 1) {
            secure_zero(pw, len + 1);
            free(pw);
            return NULL;
        }
        size_t j = x % (i + 1);
        char tmp = pw[i];
        pw[i] = pw[j];
        pw[j] = tmp;
    }
    pw[len] = '\0';

    if (!password_policy_ok(pw)) {
        // Extremely unlikely due to construction, regenerate recursively
        char *retry = generate_strong_password(len);
        secure_zero(pw, len + 1);
        free(pw);
        return retry;
    }
    return pw;
}

// ----- Main with 5 test cases -----
int main(void) {
    // Prepare environment with a securely generated password (not hard-coded)
    char *generated_pw = generate_strong_password(20);
    if (!generated_pw) {
        fprintf(stderr, "Failed to generate secure password.\n");
        return 1;
    }
#if defined(_WIN32)
    _putenv_s("SECURE_ADMIN_USER", "admin");
    _putenv_s("SECURE_ADMIN_PASSWORD", generated_pw);
#else
    setenv("SECURE_ADMIN_USER", "admin", 1);
    setenv("SECURE_ADMIN_PASSWORD", generated_pw, 1);
#endif

    if (init_auth_from_env() != 0) {
        fprintf(stderr, "Auth initialization failed. Ensure environment variables are set and valid.\n");
        secure_zero(generated_pw, strlen(generated_pw));
        free(generated_pw);
        return 1;
    }

    // Test case 1: Render index page
    printf("---- Test 1: Index Page ----\n");
    char *idx = index_page();
    if (idx) {
        printf("%s\n", idx);
        free(idx);
    } else {
        printf("Failed to render index page.\n");
    }

    // Test case 2: Render login page
    printf("---- Test 2: Login Page ----\n");
    char *lp = login_page();
    if (lp) {
        printf("%s\n", lp);
        free(lp);
    } else {
        printf("Failed to render login page.\n");
    }

    // Test case 3: Attempt login with wrong password
    printf("---- Test 3: Login with wrong password ----\n");
    char token_buf[SESSION_TOKEN_HEX_LEN + 1];
    char redirect_buf[32];
    memset(token_buf, 0, sizeof(token_buf));
    memset(redirect_buf, 0, sizeof(redirect_buf));
    int rc = do_login("admin", "wrong-password-123!", token_buf, sizeof(token_buf), redirect_buf, sizeof(redirect_buf));
    printf("Login result: %s, Redirect: %s\n", (rc == 0 ? "success" : (rc == 1 ? "auth_failed" : "error")), redirect_buf[0] ? redirect_buf : "(none)");

    // Test case 4: Attempt login with invalid username
    printf("---- Test 4: Login with invalid username ----\n");
    memset(token_buf, 0, sizeof(token_buf));
    memset(redirect_buf, 0, sizeof(redirect_buf));
    rc = do_login("bad user!", generated_pw, token_buf, sizeof(token_buf), redirect_buf, sizeof(redirect_buf));
    printf("Login result: %s, Redirect: %s\n", (rc == 0 ? "success" : (rc == 1 ? "auth_failed" : "error")), redirect_buf[0] ? redirect_buf : "(none)");

    // Test case 5: Successful login and secret page access, then invalid token access
    printf("---- Test 5: Successful login and secret access ----\n");
    memset(token_buf, 0, sizeof(token_buf));
    memset(redirect_buf, 0, sizeof(redirect_buf));
    rc = do_login("admin", generated_pw, token_buf, sizeof(token_buf), redirect_buf, sizeof(redirect_buf));
    if (rc == 0) {
        printf("Login success. Redirect: %s\n", redirect_buf);
        // Access secret page with valid session token (do not print token)
        char *secret_ok = secret_page(token_buf);
        if (secret_ok) {
            printf("%s\n", secret_ok);
            free(secret_ok);
        } else {
            printf("Failed to render secret page (valid token).\n");
        }
    } else {
        printf("Login failed unexpectedly in test 5.\n");
    }
    // Access secret page with invalid token
    char *secret_fail = secret_page("deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef");
    if (secret_fail) {
        printf("%s\n", secret_fail);
        free(secret_fail);
    } else {
        printf("Failed to render secret page (invalid token).\n");
    }

    // Cleanup
    free_auth();
    secure_zero(generated_pw, strlen(generated_pw));
    free(generated_pw);
    return 0;
}
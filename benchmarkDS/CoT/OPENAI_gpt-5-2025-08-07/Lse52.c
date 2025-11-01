#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Simple dynamic session store for demo
typedef struct Session {
    char* session_id;
    char* username;   // NULL if not authenticated
    char* csrf;
} Session;

typedef struct SessionStore {
    Session* arr;
    size_t len;
    size_t cap;
} SessionStore;

static SessionStore store = {NULL, 0, 0};

// Demo credentials
static const char* DEMO_USER = "admin";
static const char* DEMO_PASS = "s3cureP@ss!";

static void ensure_capacity() {
    if (store.len >= store.cap) {
        size_t ncap = store.cap == 0 ? 8 : store.cap * 2;
        Session* narr = (Session*)realloc(store.arr, ncap * sizeof(Session));
        if (!narr) { perror("realloc"); exit(1); }
        store.arr = narr;
        store.cap = ncap;
    }
}

static char* str_dup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* d = (char*)malloc(n + 1);
    if (!d) { perror("malloc"); exit(1); }
    memcpy(d, s, n + 1);
    return d;
}

static int safe_equals(const char* a, const char* b) {
    if (!a || !b) return 0;
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return 0;
    unsigned char diff = 0;
    for (size_t i = 0; i < la; ++i) {
        diff |= ((unsigned char)a[i]) ^ ((unsigned char)b[i]);
    }
    return diff == 0;
}

// Minimal base64url without padding encoder for random bytes
static const char* B64URL = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static char* b64url_encode(const unsigned char* data, size_t len) {
    size_t outlen = (len * 4 + 2) / 3;
    char* out = (char*)malloc(outlen + 1);
    if (!out) { perror("malloc"); exit(1); }
    size_t o = 0;
    int val = 0, valb = -6;
    for (size_t i = 0; i < len; ++i) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            out[o++] = B64URL[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    if (valb > -6) out[o++] = B64URL[((val << 8) >> (valb + 8)) & 0x3F];
    out[o] = '\0';
    return out;
}

static void get_random_bytes(unsigned char* buf, size_t n) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        size_t r = fread(buf, 1, n, f);
        fclose(f);
        if (r == n) return;
    }
    // Fallback (not ideal): use rand seeded once
    static int seeded = 0;
    if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }
    for (size_t i = 0; i < n; ++i) buf[i] = (unsigned char)(rand() & 0xFF);
}

static char* gen_token(size_t nbytes) {
    unsigned char* buf = (unsigned char*)malloc(nbytes);
    if (!buf) { perror("malloc"); exit(1); }
    get_random_bytes(buf, nbytes);
    char* tok = b64url_encode(buf, nbytes);
    free(buf);
    return tok;
}

static Session* find_session(const char* sid) {
    if (!sid) return NULL;
    for (size_t i = 0; i < store.len; ++i) {
        if (safe_equals(store.arr[i].session_id, sid)) return &store.arr[i];
    }
    return NULL;
}

char* index_page() {
    return str_dup("<html><body><h1>Welcome</h1><a href=\"/login\">Login</a></body></html>");
}

char* init_session() {
    ensure_capacity();
    Session s;
    s.session_id = gen_token(24);
    s.username = NULL;
    s.csrf = gen_token(24);
    store.arr[store.len++] = s;
    return str_dup(s.session_id);
}

static char* escape_html(const char* in) {
    // Minimal escape for attribute contexts
    size_t n = strlen(in);
    // Worst case multiply by 6
    char* out = (char*)malloc(n * 6 + 1);
    if (!out) { perror("malloc"); exit(1); }
    size_t o = 0;
    for (size_t i = 0; i < n; ++i) {
        char c = in[i];
        switch (c) {
            case '&': memcpy(out + o, "&amp;", 5); o += 5; break;
            case '<': memcpy(out + o, "&lt;", 4); o += 4; break;
            case '>': memcpy(out + o, "&gt;", 4); o += 4; break;
            case '"': memcpy(out + o, "&quot;", 6); o += 6; break;
            case '\'': memcpy(out + o, "&#x27;", 6); o += 6; break;
            default: out[o++] = c; break;
        }
    }
    out[o] = '\0';
    return out;
}

char* login_page(const char* session_id) {
    Session* s = find_session(session_id);
    if (!s) return str_dup("ERROR: Invalid session");
    char* esc = escape_html(s->csrf);
    const char* p1 = "<html><body><h1>Login</h1><form method=\"POST\" action=\"/do_login\">"
                     "<input type=\"hidden\" name=\"csrf\" value=\"";
    const char* p2 = "\"/>Username: <input type=\"text\" name=\"username\"/><br/>"
                     "Password: <input type=\"password\" name=\"password\"/><br/>"
                     "<button type=\"submit\">Login</button></form></body></html>";
    size_t len = strlen(p1) + strlen(esc) + strlen(p2);
    char* out = (char*)malloc(len + 1);
    if (!out) { perror("malloc"); exit(1); }
    strcpy(out, p1);
    strcat(out, esc);
    strcat(out, p2);
    free(esc);
    return out;
}

char* do_login(const char* session_id, const char* username, const char* password, const char* csrf_submitted) {
    Session* s = find_session(session_id);
    if (!s) return str_dup("ERROR: Invalid session");
    if (!csrf_submitted || !safe_equals(csrf_submitted, s->csrf)) return str_dup("ERROR: CSRF validation failed");
    if (!username || !password) return str_dup("ERROR: Missing credentials");
    if (safe_equals(username, DEMO_USER) && safe_equals(password, DEMO_PASS)) {
        if (s->username) { free(s->username); }
        s->username = str_dup(username);
        if (s->csrf) { free(s->csrf); }
        s->csrf = gen_token(24); // rotate CSRF
        return str_dup("REDIRECT:/secret");
    }
    return str_dup("ERROR: Invalid username or password");
}

char* secret_page(const char* session_id) {
    Session* s = find_session(session_id);
    if (!s) return str_dup("ERROR: Invalid session");
    if (!s->username) return str_dup("REDIRECT:/login");
    return str_dup("<html><body><h1>Secret settings</h1></body></html>");
}

int main(void) {
    // Test 1: Index
    char* t1 = index_page();
    printf("TEST1 Index:\n%s\n", t1);
    free(t1);

    // New session
    char* sid = init_session();

    // Test 2: Login page
    char* t2 = login_page(sid);
    printf("\nTEST2 Login Page:\n%s\n", t2);
    free(t2);
    Session* s = find_session(sid);
    char* csrf = s ? str_dup(s->csrf) : NULL;

    // Test 3: Wrong password
    char* t3 = do_login(sid, "admin", "wrong", csrf);
    printf("\nTEST3 Wrong Password:\n%s\n", t3);
    free(t3);

    // Test 4: Wrong CSRF
    char* t4 = do_login(sid, "admin", "s3cureP@ss!", "badtoken");
    printf("\nTEST4 Wrong CSRF:\n%s\n", t4);
    free(t4);

    // Test 5: Success and secret
    char* t5 = do_login(sid, "admin", "s3cureP@ss!", csrf);
    printf("\nTEST5 Successful Login and Secret:\n%s\n", t5);
    free(t5);
    char* t6 = secret_page(sid);
    printf("%s\n", t6);
    free(t6);

    // Cleanup store
    free(csrf);
    free(sid);
    for (size_t i = 0; i < store.len; ++i) {
        free(store.arr[i].session_id);
        if (store.arr[i].username) free(store.arr[i].username);
        if (store.arr[i].csrf) free(store.arr[i].csrf);
    }
    free(store.arr);
    return 0;
}
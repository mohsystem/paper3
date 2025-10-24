#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define MAX_USERS 128
#define MAX_SESSIONS 256
#define USERNAME_MAX 64
#define SID_LEN_BYTES 32
#define SALT_LEN 16
#define HASH_LEN 32
#define SESSION_TTL 1200
#define PBKDF2_ITERS 210000

typedef struct {
    char username[USERNAME_MAX+1];
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    time_t pwd_set;
    int in_use;
} User;

typedef struct {
    char sid_hex[SID_LEN_BYTES*2+1];
    char username[USERNAME_MAX+1];
    time_t expiry;
    int in_use;
} Session;

typedef struct {
    User users[MAX_USERS];
    Session sessions[MAX_SESSIONS];
} App;

typedef struct {
    char *method;
    char *path;
    struct {
        char *name;
        char *value;
    } headers[64];
    size_t headers_count;
    char *body;
} Request;

static void app_init(App* app) {
    if (!app) return;
    memset(app, 0, sizeof(*app));
}

static void secure_free(void* p, size_t len) {
    if (p && len) {
        OPENSSL_cleanse(p, len);
    }
    free(p);
}

static int secure_rand_bytes(unsigned char* buf, size_t len) {
    if (!buf || len == 0) return 0;
    return RAND_bytes(buf, (int)len) == 1;
}

static void hex_encode(const unsigned char* in, size_t inlen, char* out, size_t outlen) {
    static const char* hex = "0123456789abcdef";
    if (outlen < inlen*2+1) return;
    for (size_t i=0;i<inlen;i++) {
        out[i*2] = hex[(in[i]>>4)&0xF];
        out[i*2+1] = hex[in[i]&0xF];
    }
    out[inlen*2] = '\0';
}

static int constant_time_eq(const char* a, const char* b) {
    if (!a || !b) return 0;
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return 0;
    return CRYPTO_memcmp(a, b, la) == 0;
}

static int pbkdf2_sha256(const char* password, const unsigned char* salt, size_t saltlen, int iterations, unsigned char* out, size_t outlen) {
    if (!password || !salt || !out || outlen == 0) return 0;
    return PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, (int)saltlen, iterations, EVP_sha256(), (int)outlen, out) == 1;
}

static int password_policy_check(const char* pwd) {
    if (!pwd) return 0;
    size_t n = strlen(pwd);
    if (n < 12) return 0;
    int hasU=0, hasL=0, hasD=0, hasS=0;
    for (size_t i=0;i<n;i++) {
        unsigned char c = (unsigned char)pwd[i];
        if (isupper(c)) hasU=1;
        else if (islower(c)) hasL=1;
        else if (isdigit(c)) hasD=1;
        else hasS=1;
    }
    return hasU && hasL && hasD && hasS;
}

static char from_hex_char(char c) {
    if (c >= '0' && c <= '9') return (char)(c - '0');
    if (c >= 'a' && c <= 'f') return (char)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (char)(c - 'A' + 10);
    return 0;
}

static char* url_decode(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i=0;i<n;i++) {
        char c = s[i];
        if (c == '%' && i+2 < n && isxdigit((unsigned char)s[i+1]) && isxdigit((unsigned char)s[i+2])) {
            char hi = from_hex_char(s[i+1]);
            char lo = from_hex_char(s[i+2]);
            out[j++] = (char)((hi<<4) | lo);
            i += 2;
        } else if (c == '+') {
            out[j++] = ' ';
        } else {
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

typedef struct {
    char* keys[64];
    char* vals[64];
    size_t count;
} FormData;

static void formdata_free(FormData* f) {
    if (!f) return;
    for (size_t i=0;i<f->count;i++) {
        free(f->keys[i]);
        free(f->vals[i]);
    }
    f->count = 0;
}

static FormData parse_form(const char* body) {
    FormData f;
    memset(&f, 0, sizeof(f));
    if (!body) return f;
    size_t len = strlen(body);
    char* tmp = (char*)malloc(len + 1);
    if (!tmp) return f;
    memcpy(tmp, body, len+1);
    char* saveptr = NULL;
    char* token = strtok_r(tmp, "&", &saveptr);
    while (token && f.count < 64) {
        char* eq = strchr(token, '=');
        char* k = NULL; char* v = NULL;
        if (eq) {
            *eq = '\0';
            k = url_decode(token);
            v = url_decode(eq + 1);
        } else {
            k = url_decode(token);
            v = (char*)calloc(1,1);
        }
        if (k && v && strlen(k) > 0) {
            f.keys[f.count] = k;
            f.vals[f.count] = v;
            f.count++;
        } else {
            free(k); free(v);
        }
        token = strtok_r(NULL, "&", &saveptr);
    }
    free(tmp);
    return f;
}

static char* str_tolower_dup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    for (size_t i=0;i<n;i++) out[i] = (char)tolower((unsigned char)s[i]);
    out[n] = '\0';
    return out;
}

static void trim(char* s) {
    if (!s) return;
    size_t n = strlen(s);
    size_t i = 0;
    while (i < n && isspace((unsigned char)s[i])) i++;
    size_t j = n;
    while (j > i && isspace((unsigned char)s[j-1])) j--;
    if (i > 0) memmove(s, s+i, j - i);
    s[j - i] = '\0';
}

static int parse_request(const char* raw, Request* req) {
    if (!raw || !req) return 0;
    memset(req, 0, sizeof(*req));
    const char* p = strstr(raw, "\r\n");
    if (!p) return 0;
    size_t rl_len = (size_t)(p - raw);
    char* requestLine = (char*)malloc(rl_len + 1);
    if (!requestLine) return 0;
    memcpy(requestLine, raw, rl_len);
    requestLine[rl_len] = '\0';
    char* save = NULL;
    char* method = strtok_r(requestLine, " ", &save);
    char* path = strtok_r(NULL, " ", &save);
    if (!method || !path) { free(requestLine); return 0; }
    req->method = strdup(method);
    req->path = strdup(path);
    free(requestLine);

    const char* headerStart = p + 2;
    const char* headerEnd = strstr(headerStart, "\r\n\r\n");
    if (!headerEnd) return 0;
    const char* cursor = headerStart;
    while (cursor < headerEnd && req->headers_count < 64) {
        const char* lineEnd = strstr(cursor, "\r\n");
        if (!lineEnd || lineEnd > headerEnd) break;
        size_t lineLen = (size_t)(lineEnd - cursor);
        if (lineLen == 0) break;
        char* line = (char*)malloc(lineLen + 1);
        if (!line) return 0;
        memcpy(line, cursor, lineLen);
        line[lineLen] = '\0';
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char* name = line;
            char* value = colon + 1;
            trim(name); trim(value);
            char* lname = str_tolower_dup(name);
            req->headers[req->headers_count].name = lname ? lname : strdup("");
            req->headers[req->headers_count].value = strdup(value);
            req->headers_count++;
            free(line);
        } else {
            free(line);
        }
        cursor = lineEnd + 2;
    }
    size_t bodyLen = strlen(headerEnd + 4);
    req->body = (char*)malloc(bodyLen + 1);
    if (!req->body) return 0;
    memcpy(req->body, headerEnd + 4, bodyLen + 1);
    return 1;
}

static void free_request(Request* req) {
    if (!req) return;
    free(req->method);
    free(req->path);
    for (size_t i=0;i<req->headers_count;i++) {
        free(req->headers[i].name);
        free(req->headers[i].value);
    }
    free(req->body);
    memset(req, 0, sizeof(*req));
}

static const char* get_header(const Request* req, const char* name) {
    if (!req || !name) return NULL;
    for (size_t i=0;i<req->headers_count;i++) {
        if (req->headers[i].name && strcmp(req->headers[i].name, name) == 0) {
            return req->headers[i].value;
        }
    }
    return NULL;
}

static int app_register_user(App* app, const char* username, const char* password) {
    if (!app || !username || !password) return 0;
    size_t ulen = strlen(username);
    if (ulen == 0 || ulen > USERNAME_MAX) return 0;
    if (!password_policy_check(password)) return 0;
    for (int i=0;i<MAX_USERS;i++) {
        if (app->users[i].in_use && strncmp(app->users[i].username, username, USERNAME_MAX) == 0) {
            return 0;
        }
    }
    int slot = -1;
    for (int i=0;i<MAX_USERS;i++) {
        if (!app->users[i].in_use) { slot = i; break; }
    }
    if (slot < 0) return 0;
    User* u = &app->users[slot];
    memset(u, 0, sizeof(*u));
    strncpy(u->username, username, USERNAME_MAX);
    if (!secure_rand_bytes(u->salt, SALT_LEN)) return 0;
    if (!pbkdf2_sha256(password, u->salt, SALT_LEN, PBKDF2_ITERS, u->hash, HASH_LEN)) return 0;
    u->pwd_set = time(NULL);
    u->in_use = 1;
    return 1;
}

static int app_verify_credentials(const App* app, const char* username, const char* password) {
    if (!app || !username || !password) return 0;
    const User* u = NULL;
    for (int i=0;i<MAX_USERS;i++) {
        if (app->users[i].in_use && strncmp(app->users[i].username, username, USERNAME_MAX) == 0) {
            u = &app->users[i];
            break;
        }
    }
    if (!u) return 0;
    unsigned char calc[HASH_LEN];
    if (!pbkdf2_sha256(password, u->salt, SALT_LEN, PBKDF2_ITERS, calc, HASH_LEN)) return 0;
    int eq = CRYPTO_memcmp(u->hash, calc, HASH_LEN) == 0;
    OPENSSL_cleanse(calc, sizeof(calc));
    return eq;
}

static int app_create_session(App* app, const char* username, char* sid_hex_out, size_t sid_hex_out_len) {
    if (!app || !username || !sid_hex_out) return 0;
    if (sid_hex_out_len < SID_LEN_BYTES*2+1) return 0;
    unsigned char sid[SID_LEN_BYTES];
    if (!secure_rand_bytes(sid, sizeof(sid))) return 0;
    hex_encode(sid, sizeof(sid), sid_hex_out, sid_hex_out_len);
    int slot = -1;
    for (int i=0;i<MAX_SESSIONS;i++) {
        if (!app->sessions[i].in_use) { slot = i; break; }
    }
    if (slot < 0) return 0;
    Session* s = &app->sessions[slot];
    memset(s, 0, sizeof(*s));
    strncpy(s->sid_hex, sid_hex_out, sizeof(s->sid_hex)-1);
    strncpy(s->username, username, USERNAME_MAX);
    s->expiry = time(NULL) + SESSION_TTL;
    s->in_use = 1;
    return 1;
}

static void app_cleanup_expired(App* app) {
    if (!app) return;
    time_t now = time(NULL);
    for (int i=0;i<MAX_SESSIONS;i++) {
        if (app->sessions[i].in_use && app->sessions[i].expiry < now) {
            memset(&app->sessions[i], 0, sizeof(app->sessions[i]));
        }
    }
}

static int app_terminate_session(App* app, const char* sid_hex) {
    if (!app || !sid_hex) return 0;
    for (int i=0;i<MAX_SESSIONS;i++) {
        if (app->sessions[i].in_use && constant_time_eq(app->sessions[i].sid_hex, sid_hex)) {
            memset(&app->sessions[i], 0, sizeof(app->sessions[i]));
            return 1;
        }
    }
    return 0;
}

static const char* app_username_from_session(App* app, const char* sid_hex) {
    if (!app || !sid_hex) return NULL;
    app_cleanup_expired(app);
    for (int i=0;i<MAX_SESSIONS;i++) {
        if (app->sessions[i].in_use && constant_time_eq(app->sessions[i].sid_hex, sid_hex)) {
            time_t now = time(NULL);
            if (app->sessions[i].expiry < now) {
                memset(&app->sessions[i], 0, sizeof(app->sessions[i]));
                return NULL;
            }
            return app->sessions[i].username;
        }
    }
    return NULL;
}

typedef struct {
    int status;
    struct {
        char *name;
        char *value;
    } headers[16];
    size_t headers_count;
    char* body;
} Response;

static void response_init(Response* r) {
    memset(r, 0, sizeof(*r));
    r->status = 500;
}

static void response_add_header(Response* r, const char* name, const char* value) {
    if (r->headers_count >= 16) return;
    r->headers[r->headers_count].name = strdup(name);
    r->headers[r->headers_count].value = strdup(value);
    r->headers_count++;
}

static void response_set_body(Response* r, const char* body) {
    if (!body) return;
    size_t n = strlen(body);
    r->body = (char*)malloc(n + 1);
    if (!r->body) return;
    memcpy(r->body, body, n + 1);
}

static void response_free(Response* r) {
    if (!r) return;
    for (size_t i=0;i<r->headers_count;i++) {
        free(r->headers[i].name);
        free(r->headers[i].value);
    }
    free(r->body);
    memset(r, 0, sizeof(*r));
}

static const char* http_status_text(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default: return "OK";
    }
}

static char* build_response(const Response* r) {
    if (!r) return NULL;
    size_t bodyLen = r->body ? strlen(r->body) : 0;
    char contentLen[64];
    snprintf(contentLen, sizeof(contentLen), "%zu", bodyLen);
    // Estimate size
    size_t cap = 512 + bodyLen + r->headers_count * 64;
    char* buf = (char*)malloc(cap);
    if (!buf) return NULL;
    size_t off = 0;
    off += snprintf(buf + off, cap - off, "HTTP/1.1 %d %s\r\n", r->status, http_status_text(r->status));
    off += snprintf(buf + off, cap - off, "Content-Security-Policy: default-src 'none'\r\n");
    off += snprintf(buf + off, cap - off, "X-Content-Type-Options: nosniff\r\n");
    off += snprintf(buf + off, cap - off, "X-Frame-Options: DENY\r\n");
    off += snprintf(buf + off, cap - off, "Referrer-Policy: no-referrer\r\n");
    off += snprintf(buf + off, cap - off, "Cache-Control: no-store\r\n");
    for (size_t i=0;i<r->headers_count;i++) {
        off += snprintf(buf + off, cap - off, "%s: %s\r\n", r->headers[i].name, r->headers[i].value);
    }
    off += snprintf(buf + off, cap - off, "Content-Length: %s\r\n", contentLen);
    off += snprintf(buf + off, cap - off, "\r\n");
    if (bodyLen > 0) {
        memcpy(buf + off, r->body, bodyLen);
        off += bodyLen;
    }
    // Null-terminate for printing convenience
    char* out = (char*)malloc(off + 1);
    if (!out) { free(buf); return NULL; }
    memcpy(out, buf, off);
    out[off] = '\0';
    free(buf);
    return out;
}

static char* find_header_value(const char* response, const char* name) {
    if (!response || !name) return NULL;
    size_t nameLen = strlen(name);
    const char* p = response;
    while (1) {
        const char* lineEnd = strstr(p, "\r\n");
        if (!lineEnd) return NULL;
        if (lineEnd == p) return NULL; // end of headers
        const char* colon = strchr(p, ':');
        if (colon && colon < lineEnd) {
            size_t klen = (size_t)(colon - p);
            if (klen == nameLen && strncasecmp(p, name, nameLen) == 0) {
                const char* valStart = colon + 1;
                while (valStart < lineEnd && isspace((unsigned char)*valStart)) valStart++;
                size_t vlen = (size_t)(lineEnd - valStart);
                char* val = (char*)malloc(vlen + 1);
                if (!val) return NULL;
                memcpy(val, valStart, vlen);
                val[vlen] = '\0';
                return val;
            }
        }
        p = lineEnd + 2;
    }
}

static char* parse_cookie_sid(const char* cookieHeader) {
    if (!cookieHeader) return NULL;
    char* dup = strdup(cookieHeader);
    if (!dup) return NULL;
    char* save = NULL;
    char* token = strtok_r(dup, ";", &save);
    while (token) {
        while (*token && isspace((unsigned char)*token)) token++;
        char* eq = strchr(token, '=');
        if (eq) {
            *eq = '\0';
            char* name = token;
            char* value = eq + 1;
            if (strcmp(name, "SID") == 0) {
                char* out = strdup(value);
                free(dup);
                return out;
            }
        }
        token = strtok_r(NULL, ";", &save);
    }
    free(dup);
    return NULL;
}

static char* app_process_http_request(App* app, const char* raw) {
    if (!app || !raw) return NULL;
    Request req;
    if (!parse_request(raw, &req)) {
        Response r; response_init(&r);
        r.status = 400; response_set_body(&r, "Malformed request");
        response_add_header(&r, "Content-Type", "text/plain; charset=utf-8");
        char* out = build_response(&r);
        response_free(&r);
        return out;
    }
    Response r; response_init(&r);
    response_add_header(&r, "Content-Type", "text/plain; charset=utf-8");

    if (strcmp(req.path, "/register") == 0 && strcmp(req.method, "POST") == 0) {
        const char* ctype = get_header(&req, "content-type");
        if (!ctype || strncasecmp(ctype, "application/x-www-form-urlencoded", 33) != 0) {
            r.status = 400; response_set_body(&r, "Unsupported Content-Type");
        } else {
            FormData f = parse_form(req.body);
            const char* user = NULL; const char* pass = NULL;
            for (size_t i=0;i<f.count;i++) {
                if (strcmp(f.keys[i], "username") == 0) user = f.vals[i];
                else if (strcmp(f.keys[i], "password") == 0) pass = f.vals[i];
            }
            if (user && pass && app_register_user(app, user, pass)) {
                r.status = 201; response_set_body(&r, "Registered");
            } else {
                r.status = 400; response_set_body(&r, "Registration failed");
            }
            formdata_free(&f);
        }
    } else if (strcmp(req.path, "/login") == 0 && strcmp(req.method, "POST") == 0) {
        const char* ctype = get_header(&req, "content-type");
        if (!ctype || strncasecmp(ctype, "application/x-www-form-urlencoded", 33) != 0) {
            r.status = 400; response_set_body(&r, "Unsupported Content-Type");
        } else {
            FormData f = parse_form(req.body);
            const char* user = NULL; const char* pass = NULL;
            for (size_t i=0;i<f.count;i++) {
                if (strcmp(f.keys[i], "username") == 0) user = f.vals[i];
                else if (strcmp(f.keys[i], "password") == 0) pass = f.vals[i];
            }
            if (user && pass && app_verify_credentials(app, user, pass)) {
                char sid_hex[SID_LEN_BYTES*2+1];
                if (app_create_session(app, user, sid_hex, sizeof(sid_hex))) {
                    r.status = 200; response_set_body(&r, "Logged in");
                    char cookie[256];
                    snprintf(cookie, sizeof(cookie), "SID=%s; HttpOnly; Secure; SameSite=Strict; Path=/", sid_hex);
                    response_add_header(&r, "Set-Cookie", cookie);
                } else {
                    r.status = 500; response_set_body(&r, "Session error");
                }
            } else {
                r.status = 401; response_set_body(&r, "Unauthorized");
            }
            formdata_free(&f);
        }
    } else if (strcmp(req.path, "/me") == 0 && strcmp(req.method, "GET") == 0) {
        const char* cookie = get_header(&req, "cookie");
        char* sid = parse_cookie_sid(cookie);
        if (!sid) {
            r.status = 401; response_set_body(&r, "Unauthorized");
        } else {
            const char* uname = app_username_from_session(app, sid);
            if (!uname) {
                r.status = 401; response_set_body(&r, "Unauthorized");
            } else {
                char body[256];
                snprintf(body, sizeof(body), "Hello, %s", uname);
                r.status = 200; response_set_body(&r, body);
            }
            free(sid);
        }
    } else if (strcmp(req.path, "/logout") == 0 && strcmp(req.method, "POST") == 0) {
        const char* cookie = get_header(&req, "cookie");
        char* sid = parse_cookie_sid(cookie);
        if (!sid || !app_terminate_session(app, sid)) {
            r.status = 401; response_set_body(&r, "Unauthorized");
        } else {
            r.status = 200; response_set_body(&r, "Logged out");
            response_add_header(&r, "Set-Cookie", "SID=; HttpOnly; Secure; SameSite=Strict; Path=/; Max-Age=0");
        }
        free(sid);
    } else {
        r.status = 404; response_set_body(&r, "Not Found");
    }

    char* out = build_response(&r);
    response_free(&r);
    free_request(&req);
    return out;
}

static char* make_request(const char* method, const char* path, const char* extraHeaders, const char* body) {
    if (!method || !path) return NULL;
    if (!body) body = "";
    size_t bodyLen = strlen(body);
    char contentLen[64];
    snprintf(contentLen, sizeof(contentLen), "%zu", bodyLen);
    size_t cap = 512 + strlen(path) + strlen(method) + (extraHeaders ? strlen(extraHeaders) : 0) + bodyLen;
    char* buf = (char*)malloc(cap);
    if (!buf) return NULL;
    size_t off = 0;
    off += snprintf(buf + off, cap - off, "%s %s HTTP/1.1\r\n", method, path);
    if (extraHeaders) off += snprintf(buf + off, cap - off, "%s", extraHeaders);
    off += snprintf(buf + off, cap - off, "Content-Length: %s\r\n", contentLen);
    off += snprintf(buf + off, cap - off, "\r\n");
    if (bodyLen) {
        memcpy(buf + off, body, bodyLen);
        off += bodyLen;
    }
    char* out = (char*)malloc(off + 1);
    if (!out) { free(buf); return NULL; }
    memcpy(out, buf, off);
    out[off] = '\0';
    free(buf);
    return out;
}

int main(void) {
    App app;
    app_init(&app);

    // Test 1: Register user
    {
        const char* headers = "Host: localhost\r\nContent-Type: application/x-www-form-urlencoded\r\n";
        char* req = make_request("POST", "/register", headers, "username=bob&password=G00d!Password#1");
        char* resp = app_process_http_request(&app, req);
        if (resp) printf("Test1 Status: %.3s\n", resp + 9);
        free(req);
        free(resp);
    }

    // Test 2: Login wrong password
    {
        const char* headers = "Host: localhost\r\nContent-Type: application/x-www-form-urlencoded\r\n";
        char* req = make_request("POST", "/login", headers, "username=bob&password=WrongPass!123");
        char* resp = app_process_http_request(&app, req);
        if (resp) printf("Test2 Status: %.3s\n", resp + 9);
        free(req);
        free(resp);
    }

    // Test 3: Login correct and access /me
    char* sid_cookie = NULL;
    {
        const char* headers = "Host: localhost\r\nContent-Type: application/x-www-form-urlencoded\r\n";
        char* req = make_request("POST", "/login", headers, "username=bob&password=G00d!Password#1");
        char* resp = app_process_http_request(&app, req);
        if (resp) printf("Test3 Login Status: %.3s\n", resp + 9);
        char* set_cookie = find_header_value(resp, "Set-Cookie");
        if (set_cookie) {
            sid_cookie = set_cookie; // store
        }
        free(req);
        // /me
        char cookieHeader[512] = {0};
        if (sid_cookie) {
            snprintf(cookieHeader, sizeof(cookieHeader), "Host: localhost\r\nCookie: %s\r\n", sid_cookie);
        } else {
            snprintf(cookieHeader, sizeof(cookieHeader), "Host: localhost\r\n");
        }
        char* req2 = make_request("GET", "/me", cookieHeader, "");
        char* resp2 = app_process_http_request(&app, req2);
        if (resp2) printf("Test3 /me Status: %.3s\n", resp2 + 9);
        free(req2);
        free(resp);
    }

    // Test 4: Logout
    {
        char headers[512] = {0};
        if (sid_cookie) {
            snprintf(headers, sizeof(headers), "Host: localhost\r\nCookie: %s\r\n", sid_cookie);
        } else {
            snprintf(headers, sizeof(headers), "Host: localhost\r\n");
        }
        char* req = make_request("POST", "/logout", headers, "");
        char* resp = app_process_http_request(&app, req);
        if (resp) printf("Test4 Logout Status: %.3s\n", resp + 9);
        free(req);
        free(resp);
    }

    // Test 5: /me after logout
    {
        char headers[512] = {0};
        if (sid_cookie) {
            snprintf(headers, sizeof(headers), "Host: localhost\r\nCookie: %s\r\n", sid_cookie);
        } else {
            snprintf(headers, sizeof(headers), "Host: localhost\r\n");
        }
        char* req = make_request("GET", "/me", headers, "");
        char* resp = app_process_http_request(&app, req);
        if (resp) printf("Test5 /me after logout Status: %.3s\n", resp + 9);
        free(req);
        free(resp);
    }

    free(sid_cookie);
    return 0;
}
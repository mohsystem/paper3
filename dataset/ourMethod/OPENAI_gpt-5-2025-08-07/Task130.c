#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_HEADERS 32
#define NAME_LEN 64
#define VALUE_LEN 512
#define BODY_LEN 512

typedef struct {
    char name[NAME_LEN];
    char value[VALUE_LEN];
} Header;

typedef struct {
    Header items[MAX_HEADERS];
    int count;
} HeaderList;

typedef struct {
    int status;
    HeaderList headers;
    char body[BODY_LEN];
} Response;

static const char* ALLOWED_ORIGINS[] = {
    "https://example.com",
    "https://app.example.com"
};
static const int ALLOWED_ORIGINS_COUNT = 2;

static const char* ALLOWED_METHODS[] = {"GET", "POST", "OPTIONS"};
static const int ALLOWED_METHODS_COUNT = 3;

static const char* ALLOWED_HEADERS[] = {"content-type", "x-requested-with"};
static const int ALLOWED_HEADERS_COUNT = 2;

static const int MAX_AGE_SECONDS = 600;
static const int ALLOW_CREDENTIALS = 0;

static void init_headers(HeaderList* h) {
    if (h) {
        h->count = 0;
        for (int i = 0; i < MAX_HEADERS; i++) {
            h->items[i].name[0] = '\0';
            h->items[i].value[0] = '\0';
        }
    }
}

static int add_header(HeaderList* h, const char* name, const char* value) {
    if (!h || !name || !value) return 0;
    if (h->count >= MAX_HEADERS) return 0;
    size_t nl = strnlen(name, NAME_LEN);
    size_t vl = strnlen(value, VALUE_LEN);
    if (nl == 0 || nl >= NAME_LEN || vl >= VALUE_LEN) return 0;
    snprintf(h->items[h->count].name, NAME_LEN, "%s", name);
    snprintf(h->items[h->count].value, VALUE_LEN, "%s", value);
    h->count++;
    return 1;
}

static int ascii_ci_equal(const char* a, const char* b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

static const char* get_header(const HeaderList* h, const char* name) {
    if (!h || !name) return NULL;
    for (int i = 0; i < h->count; i++) {
        if (ascii_ci_equal(h->items[i].name, name)) {
            return h->items[i].value;
        }
    }
    return NULL;
}

static int contains_str_ci(const char* const* arr, int count, const char* val) {
    for (int i = 0; i < count; i++) {
        if (ascii_ci_equal(arr[i], val)) return 1;
    }
    return 0;
}

static int is_valid_token(const char* s, size_t maxLen) {
    if (!s) return 0;
    size_t len = strnlen(s, maxLen + 1);
    if (len == 0 || len > maxLen) return 0;
    return 1;
}

static void set_cors_headers(HeaderList* out, const char* origin) {
    add_header(out, "Vary", "Origin");
    add_header(out, "Access-Control-Allow-Origin", origin ? origin : "");
    if (ALLOW_CREDENTIALS) {
        add_header(out, "Access-Control-Allow-Credentials", "true");
    }
    add_header(out, "Access-Control-Expose-Headers", "Content-Type");
}

static void join_sorted_methods(char* out, size_t outLen) {
    // Already small list, sort manually
    const char* buf[ALLOWED_METHODS_COUNT];
    for (int i = 0; i < ALLOWED_METHODS_COUNT; i++) buf[i] = ALLOWED_METHODS[i];
    // simple bubble sort case-insensitive
    for (int i = 0; i < ALLOWED_METHODS_COUNT; i++) {
        for (int j = i + 1; j < ALLOWED_METHODS_COUNT; j++) {
            if (strncasecmp(buf[i], buf[j], 64) > 0) {
                const char* tmp = buf[i]; buf[i] = buf[j]; buf[j] = tmp;
            }
        }
    }
    out[0] = '\0';
    for (int i = 0; i < ALLOWED_METHODS_COUNT; i++) {
        if (i) strncat(out, ", ", outLen - strlen(out) - 1);
        strncat(out, buf[i], outLen - strlen(out) - 1);
    }
}

static void join_sorted_headers(char* out, size_t outLen) {
    const char* buf[ALLOWED_HEADERS_COUNT];
    for (int i = 0; i < ALLOWED_HEADERS_COUNT; i++) buf[i] = ALLOWED_HEADERS[i];
    for (int i = 0; i < ALLOWED_HEADERS_COUNT; i++) {
        for (int j = i + 1; j < ALLOWED_HEADERS_COUNT; j++) {
            if (strncasecmp(buf[i], buf[j], 64) > 0) {
                const char* tmp = buf[i]; buf[i] = buf[j]; buf[j] = tmp;
            }
        }
    }
    out[0] = '\0';
    for (int i = 0; i < ALLOWED_HEADERS_COUNT; i++) {
        if (i) strncat(out, ", ", outLen - strlen(out) - 1);
        strncat(out, buf[i], outLen - strlen(out) - 1);
    }
}

Response process_request(const char* methodIn, const char* pathIn, const HeaderList* reqHeaders, const char* body) {
    Response resp;
    resp.status = 500;
    init_headers(&resp.headers);
    resp.body[0] = '\0';

    char method[17]; method[16] = '\0';
    char path[129]; path[128] = '\0';

    if (!methodIn || !pathIn) {
        resp.status = 400;
        snprintf(resp.body, BODY_LEN, "%s", "Bad Request");
        return resp;
    }

    // Uppercase method safely
    size_t mlen = strnlen(methodIn, 16);
    for (size_t i = 0; i < mlen; i++) method[i] = (char)toupper((unsigned char)methodIn[i]);
    method[mlen] = '\0';

    // Copy path
    size_t plen = strnlen(pathIn, 128);
    memcpy(path, pathIn, plen);
    path[plen] = '\0';

    if (!is_valid_token(method, 16) || !is_valid_token(path, 128) || path[0] != '/') {
        resp.status = 400;
        snprintf(resp.body, BODY_LEN, "%s", "Bad Request");
        return resp;
    }

    const char* origin = get_header(reqHeaders, "Origin");
    if (origin && strnlen(origin, 204) > 200) {
        resp.status = 400;
        snprintf(resp.body, BODY_LEN, "%s", "Invalid Origin");
        return resp;
    }

    int isPreflight = (ascii_ci_equal(method, "OPTIONS") &&
                       origin != NULL &&
                       get_header(reqHeaders, "Access-Control-Request-Method") != NULL);

    if (isPreflight) {
        if (!contains_str_ci(ALLOWED_ORIGINS, ALLOWED_ORIGINS_COUNT, origin)) {
            resp.status = 403;
            snprintf(resp.body, BODY_LEN, "%s", "CORS preflight failed: origin not allowed");
            return resp;
        }
        const char* reqMethod = get_header(reqHeaders, "Access-Control-Request-Method");
        if (!reqMethod) {
            resp.status = 400;
            snprintf(resp.body, BODY_LEN, "%s", "CORS preflight failed: missing ACRM");
            return resp;
        }
        char reqMethodUp[17]; reqMethodUp[16] = '\0';
        size_t rmlen = strnlen(reqMethod, 16);
        for (size_t i = 0; i < rmlen; i++) reqMethodUp[i] = (char)toupper((unsigned char)reqMethod[i]);
        reqMethodUp[rmlen] = '\0';
        if (!contains_str_ci(ALLOWED_METHODS, ALLOWED_METHODS_COUNT, reqMethodUp)) {
            resp.status = 403;
            snprintf(resp.body, BODY_LEN, "%s", "CORS preflight failed: method not allowed");
            return resp;
        }
        const char* acrh = get_header(reqHeaders, "Access-Control-Request-Headers");
        if (acrh) {
            char tmp[256];
            snprintf(tmp, sizeof(tmp), "%s", acrh);
            char* saveptr = NULL;
            char* token = strtok_r(tmp, ",", &saveptr);
            while (token) {
                // trim spaces
                while (*token && isspace((unsigned char)*token)) token++;
                char* end = token + strlen(token);
                while (end > token && isspace((unsigned char)*(end - 1))) { *(--end) = '\0'; }
                // lower-case
                for (char* p = token; *p; ++p) *p = (char)tolower((unsigned char)*p);

                if (*token && !contains_str_ci(ALLOWED_HEADERS, ALLOWED_HEADERS_COUNT, token)) {
                    resp.status = 403;
                    snprintf(resp.body, BODY_LEN, "CORS preflight failed: header not allowed -> %s", token);
                    return resp;
                }
                token = strtok_r(NULL, ",", &saveptr);
            }
        }

        set_cors_headers(&resp.headers, origin);
        char methodsBuf[128]; methodsBuf[0] = '\0';
        char headersBuf[128]; headersBuf[0] = '\0';
        join_sorted_methods(methodsBuf, sizeof(methodsBuf));
        join_sorted_headers(headersBuf, sizeof(headersBuf));
        add_header(&resp.headers, "Access-Control-Allow-Methods", methodsBuf);
        add_header(&resp.headers, "Access-Control-Allow-Headers", headersBuf);
        char age[16];
        snprintf(age, sizeof(age), "%d", MAX_AGE_SECONDS);
        add_header(&resp.headers, "Access-Control-Max-Age", age);
        resp.status = 204;
        resp.body[0] = '\0';
        return resp;
    } else {
        if (origin) {
            if (!contains_str_ci(ALLOWED_ORIGINS, ALLOWED_ORIGINS_COUNT, origin)) {
                resp.status = 403;
                snprintf(resp.body, BODY_LEN, "%s", "CORS error: origin not allowed");
                return resp;
            }
            set_cors_headers(&resp.headers, origin);
        }

        if (ascii_ci_equal(method, "GET") && strcmp(path, "/data") == 0) {
            add_header(&resp.headers, "Content-Type", "application/json; charset=UTF-8");
            resp.status = 200;
            snprintf(resp.body, BODY_LEN, "%s", "{\"message\":\"ok\"}");
            return resp;
        } else if (strcmp(path, "/data") != 0) {
            resp.status = 404;
            snprintf(resp.body, BODY_LEN, "%s", "Not Found");
            return resp;
        } else {
            resp.status = 405;
            snprintf(resp.body, BODY_LEN, "%s", "Method Not Allowed");
            return resp;
        }
    }
}

static void print_response(const char* label, const Response* r) {
    printf("%s\n", label);
    printf("Status: %d\n", r->status);
    printf("Headers:\n");
    for (int i = 0; i < r->headers.count; i++) {
        printf("  %s: %s\n", r->headers.items[i].name, r->headers.items[i].value);
    }
    printf("Body: %s\n\n", r->body);
}

static void build_headers(HeaderList* h, int pairCount, const char* pairs[][2]) {
    init_headers(h);
    for (int i = 0; i < pairCount; i++) {
        const char* k = pairs[i][0];
        const char* v = pairs[i][1];
        if (!k) k = "";
        if (!v) v = "";
        if ((int)strnlen(k, NAME_LEN) < NAME_LEN && (int)strnlen(v, VALUE_LEN) < VALUE_LEN) {
            add_header(h, k, v);
        }
    }
}

int main(void) {
    // Test 1: Allowed origin GET
    HeaderList h1; 
    const char* p1[][2] = { {"Origin", "https://example.com"} };
    build_headers(&h1, 1, p1);
    Response r1 = process_request("GET", "/data", &h1, NULL);
    print_response("Test 1:", &r1);

    // Test 2: Disallowed origin
    HeaderList h2;
    const char* p2[][2] = { {"Origin", "https://evil.com"} };
    build_headers(&h2, 1, p2);
    Response r2 = process_request("GET", "/data", &h2, NULL);
    print_response("Test 2:", &r2);

    // Test 3: Preflight allowed
    HeaderList h3;
    const char* p3[][2] = {
        {"Origin", "https://app.example.com"},
        {"Access-Control-Request-Method", "GET"},
        {"Access-Control-Request-Headers", "Content-Type, X-Requested-With"}
    };
    build_headers(&h3, 3, p3);
    Response r3 = process_request("OPTIONS", "/data", &h3, NULL);
    print_response("Test 3:", &r3);

    // Test 4: Preflight with disallowed header
    HeaderList h4;
    const char* p4[][2] = {
        {"Origin", "https://example.com"},
        {"Access-Control-Request-Method", "GET"},
        {"Access-Control-Request-Headers", "X-Secret"}
    };
    build_headers(&h4, 3, p4);
    Response r4 = process_request("OPTIONS", "/data", &h4, NULL);
    print_response("Test 4:", &r4);

    // Test 5: No Origin header
    HeaderList h5;
    const char* p5[][2] = { {"User-Agent", "TestClient/1.0"} };
    build_headers(&h5, 1, p5);
    Response r5 = process_request("GET", "/data", &h5, NULL);
    print_response("Test 5:", &r5);

    return 0;
}
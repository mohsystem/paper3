#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char* key;
    char* value;
} Header;

typedef struct {
    int status;
    Header* headers;
    int header_count;
    char* body;
} Response;

static const char* ALLOWED_ORIGINS[] = {"https://example.com", "https://app.local"};
static const int ALLOWED_ORIGINS_COUNT = 2;

static const char* ALLOWED_METHODS[] = {"GET", "POST"};
static const int ALLOWED_METHODS_COUNT = 2;

static const char* ALLOWED_HEADERS_DEFAULT = "Content-Type, Authorization, X-Requested-With";

static int str_case_eq(const char* a, const char* b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

static char* str_dup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* d = (char*)malloc(n + 1);
    if (d) memcpy(d, s, n + 1);
    return d;
}

static void add_header(Response* resp, const char* key, const char* value) {
    resp->headers = (Header*)realloc(resp->headers, sizeof(Header) * (resp->header_count + 1));
    resp->headers[resp->header_count].key = str_dup(key);
    resp->headers[resp->header_count].value = str_dup(value);
    resp->header_count += 1;
}

static const char* get_header(Header* headers, int count, const char* key) {
    for (int i = 0; i < count; ++i) {
        if (str_case_eq(headers[i].key, key)) {
            return headers[i].value;
        }
    }
    return NULL;
}

static int origin_allowed(const char* origin) {
    if (!origin) return 0;
    for (int i = 0; i < ALLOWED_ORIGINS_COUNT; ++i) {
        if (strcmp(origin, ALLOWED_ORIGINS[i]) == 0) return 1;
    }
    return 0;
}

static int method_allowed(const char* method) {
    if (!method) return 0;
    for (int i = 0; i < ALLOWED_METHODS_COUNT; ++i) {
        if (strcmp(method, ALLOWED_METHODS[i]) == 0) return 1;
    }
    return 0;
}

static char* to_upper_dup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* d = (char*)malloc(n + 1);
    for (size_t i = 0; i < n; ++i) d[i] = (char)toupper((unsigned char)s[i]);
    d[n] = '\0';
    return d;
}

Response handle_request(const char* method_in, const char* origin, const char* path, Header* request_headers, int request_header_count) {
    Response resp;
    resp.status = 200;
    resp.headers = NULL;
    resp.header_count = 0;
    resp.body = NULL;

    char* method = to_upper_dup(method_in ? method_in : "");

    if (strcmp(method, "OPTIONS") == 0) {
        if (!origin_allowed(origin)) {
            resp.status = 403;
            resp.body = str_dup("Forbidden: Origin not allowed");
            free(method);
            return resp;
        }
        const char* req_method = get_header(request_headers, request_header_count, "Access-Control-Request-Method");
        if (!req_method) {
            resp.status = 400;
            resp.body = str_dup("Bad Request: Missing Access-Control-Request-Method");
            free(method);
            return resp;
        }
        char* req_method_upper = to_upper_dup(req_method);
        if (!method_allowed(req_method_upper)) {
            resp.status = 405;
            resp.body = str_dup("Method Not Allowed for CORS preflight");
            free(req_method_upper);
            free(method);
            return resp;
        }
        const char* req_headers = get_header(request_headers, request_header_count, "Access-Control-Request-Headers");
        add_header(&resp, "Access-Control-Allow-Origin", origin ? origin : "");
        add_header(&resp, "Vary", "Origin");
        // Build allowed methods string
        char methods_buf[64] = {0};
        for (int i = 0; i < ALLOWED_METHODS_COUNT; ++i) {
            strcat(methods_buf, ALLOWED_METHODS[i]);
            if (i + 1 < ALLOWED_METHODS_COUNT) strcat(methods_buf, ", ");
        }
        add_header(&resp, "Access-Control-Allow-Methods", methods_buf);
        add_header(&resp, "Access-Control-Allow-Headers", req_headers && strlen(req_headers) > 0 ? req_headers : ALLOWED_HEADERS_DEFAULT);
        add_header(&resp, "Access-Control-Max-Age", "600");
        resp.status = 204;
        resp.body = str_dup("");
        free(req_method_upper);
        free(method);
        return resp;
    }

    if (!path || strcmp(path, "/hello") != 0) {
        resp.status = 404;
        resp.body = str_dup("Not Found");
        free(method);
        return resp;
    }

    if (!method_allowed(method)) {
        resp.status = 405;
        resp.body = str_dup("Method Not Allowed");
        free(method);
        return resp;
    }

    if (origin) {
        if (!origin_allowed(origin)) {
            resp.status = 403;
            resp.body = str_dup("Forbidden: Origin not allowed");
            free(method);
            return resp;
        }
        add_header(&resp, "Access-Control-Allow-Origin", origin);
        add_header(&resp, "Vary", "Origin");
        add_header(&resp, "Access-Control-Expose-Headers", "X-RateLimit-Remaining");
    }
    add_header(&resp, "Content-Type", "application/json");
    add_header(&resp, "X-RateLimit-Remaining", "42");

    const char* prefix = "{\"message\":\"Hello from CORS-enabled API\",\"method\":\"";
    const char* suffix = "\"}";
    size_t body_len = strlen(prefix) + strlen(method) + strlen(suffix);
    resp.body = (char*)malloc(body_len + 1);
    strcpy(resp.body, prefix);
    strcat(resp.body, method);
    strcat(resp.body, suffix);
    resp.status = 200;

    free(method);
    return resp;
}

static void free_response(Response* r) {
    if (!r) return;
    for (int i = 0; i < r->header_count; ++i) {
        free(r->headers[i].key);
        free(r->headers[i].value);
    }
    free(r->headers);
    free(r->body);
}

static void print_response(const char* label, Response* r) {
    printf("%s\n", label);
    printf("Status: %d\n", r->status);
    for (int i = 0; i < r->header_count; ++i) {
        printf("%s: %s\n", r->headers[i].key, r->headers[i].value);
    }
    printf("Body: %s\n", r->body ? r->body : "");
}

int main() {
    // Test 1: GET /hello from allowed origin
    Header h1[] = {};
    Response r1 = handle_request("GET", "https://example.com", "/hello", h1, 0);
    print_response("=== C Test 1 ===", &r1);
    free_response(&r1);

    // Test 2: OPTIONS preflight with allowed origin and headers
    Header h2[] = {
        {"Access-Control-Request-Method", "POST"},
        {"Access-Control-Request-Headers", "Content-Type, X-Test"}
    };
    Response r2 = handle_request("OPTIONS", "https://example.com", "/hello", h2, 2);
    print_response("=== C Test 2 ===", &r2);
    free_response(&r2);

    // Test 3: GET from disallowed origin
    Header h3[] = {};
    Response r3 = handle_request("GET", "https://evil.com", "/hello", h3, 0);
    print_response("=== C Test 3 ===", &r3);
    free_response(&r3);

    // Test 4: POST to /hello from allowed origin
    Header h4[] = {};
    Response r4 = handle_request("POST", "https://app.local", "/hello", h4, 0);
    print_response("=== C Test 4 ===", &r4);
    free_response(&r4);

    // Test 5: OPTIONS with disallowed requested method
    Header h5[] = {
        {"Access-Control-Request-Method", "PUT"}
    };
    Response r5 = handle_request("OPTIONS", "https://example.com", "/hello", h5, 1);
    print_response("=== C Test 5 ===", &r5);
    free_response(&r5);

    return 0;
}
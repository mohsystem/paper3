#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_HEADERS 32
#define MAX_NAME 64
#define MAX_VALUE 256
#define MAX_BODY 512

typedef struct {
    int status;
    char header_name[MAX_HEADERS][MAX_NAME];
    char header_value[MAX_HEADERS][MAX_VALUE];
    int header_count;
    char body[MAX_BODY];
} Response;

// Security: allowlists
static const char* ALLOWED_ORIGINS[] = {
    "https://trustedorigin.com",
    "http://localhost:3000"
};
static const int ALLOWED_ORIGINS_COUNT = 2;

static const char* ALLOWED_METHODS[] = { "GET", "POST" };
static const int ALLOWED_METHODS_COUNT = 2;

static const char* ALLOWED_HEADERS[] = {
    "content-type", "authorization", "x-requested-with"
};
static const int ALLOWED_HEADERS_COUNT = 3;

static void str_trim(char* s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    if (start > 0 || end < len) {
        memmove(s, s + start, end - start);
        s[end - start] = '\0';
    }
}

static void str_to_upper(char* s) {
    for (; *s; ++s) *s = (char)toupper((unsigned char)*s);
}
static void str_to_lower(char* s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static int is_origin_allowed(const char* origin) {
    if (!origin) return 0;
    char buf[256];
    strncpy(buf, origin, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';
    str_trim(buf);
    for (int i = 0; i < ALLOWED_ORIGINS_COUNT; ++i) {
        if (strcmp(buf, ALLOWED_ORIGINS[i]) == 0) return 1;
    }
    return 0;
}

static int is_method_allowed(const char* method) {
    if (!method) return 0;
    for (int i = 0; i < ALLOWED_METHODS_COUNT; ++i) {
        if (strcmp(method, ALLOWED_METHODS[i]) == 0) return 1;
    }
    return 0;
}

static void headers_add(Response* r, const char* name, const char* value) {
    if (r->header_count >= MAX_HEADERS) return;
    strncpy(r->header_name[r->header_count], name, MAX_NAME - 1);
    r->header_name[r->header_count][MAX_NAME - 1] = '\0';
    strncpy(r->header_value[r->header_count], value, MAX_VALUE - 1);
    r->header_value[r->header_count][MAX_VALUE - 1] = '\0';
    r->header_count++;
}

static void join_methods(char* out, size_t outsz) {
    out[0] = '\0';
    for (int i = 0; i < ALLOWED_METHODS_COUNT; ++i) {
        if (i) strncat(out, ", ", outsz - strlen(out) - 1);
        strncat(out, ALLOWED_METHODS[i], outsz - strlen(out) - 1);
    }
}

static void join_allowed_headers(char* out, size_t outsz, const char** list, int count) {
    out[0] = '\0';
    for (int i = 0; i < count; ++i) {
        if (i) strncat(out, ", ", outsz - strlen(out) - 1);
        strncat(out, list[i], outsz - strlen(out) - 1);
    }
}

static int header_in_allowed(const char* h) {
    for (int i = 0; i < ALLOWED_HEADERS_COUNT; ++i) {
        if (strcmp(h, ALLOWED_HEADERS[i]) == 0) return 1;
    }
    return 0;
}

static int parse_and_filter_headers(const char* header_list, const char** out, int max_out) {
    if (!header_list || !*header_list) {
        // No requested, return full allowlist (limited by max_out)
        int n = ALLOWED_HEADERS_COUNT < max_out ? ALLOWED_HEADERS_COUNT : max_out;
        for (int i = 0; i < n; ++i) out[i] = ALLOWED_HEADERS[i];
        return n;
    }
    char buf[512];
    strncpy(buf, header_list, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';
    int count = 0;
    char* saveptr = NULL;
    char* tok = strtok_r(buf, ",", &saveptr);
    while (tok && count < max_out) {
        char t[128];
        strncpy(t, tok, sizeof(t)-1);
        t[sizeof(t)-1] = '\0';
        str_trim(t);
        str_to_lower(t);
        if (t[0] != '\0' && header_in_allowed(t)) {
            // Store pointer to static allowlist to avoid lifetime issues
            for (int i = 0; i < ALLOWED_HEADERS_COUNT; ++i) {
                if (strcmp(t, ALLOWED_HEADERS[i]) == 0) {
                    out[count++] = ALLOWED_HEADERS[i];
                    break;
                }
            }
        }
        tok = strtok_r(NULL, ",", &saveptr);
    }
    return count;
}

Response handle_request(const char* method,
                        const char* path,
                        const char* origin,
                        const char* access_control_request_method,
                        const char* access_control_request_headers) {
    Response r;
    r.status = 200;
    r.header_count = 0;
    r.body[0] = '\0';

    headers_add(&r, "X-Content-Type-Options", "nosniff");
    headers_add(&r, "X-Frame-Options", "DENY");
    headers_add(&r, "Referrer-Policy", "no-referrer");
    headers_add(&r, "Cache-Control", "no-store");

    char m[16] = {0};
    if (method) {
        strncpy(m, method, sizeof(m)-1);
        for (char* p = m; *p; ++p) *p = (char)toupper((unsigned char)*p);
    }

    char pth[128] = {0};
    if (path) {
        strncpy(pth, path, sizeof(pth)-1);
        str_trim(pth);
    }

    int originAllowed = is_origin_allowed(origin);

    if (strcmp(m, "OPTIONS") == 0) {
        if (!originAllowed) {
            r.status = 403;
            return r;
        }
        char reqm[16] = {0};
        if (access_control_request_method) {
            strncpy(reqm, access_control_request_method, sizeof(reqm)-1);
            str_trim(reqm);
            str_to_upper(reqm);
        }
        if (!is_method_allowed(reqm)) {
            r.status = 405;
            return r;
        }

        const char* filtered[16];
        int filtered_count = parse_and_filter_headers(access_control_request_headers, filtered, 16);

        headers_add(&r, "Access-Control-Allow-Origin", origin ? origin : "");
        headers_add(&r, "Vary", "Origin");

        char methods_csv[64];
        join_methods(methods_csv, sizeof(methods_csv));
        headers_add(&r, "Access-Control-Allow-Methods", methods_csv);

        if (filtered_count > 0) {
            char hdr_csv[256];
            join_allowed_headers(hdr_csv, sizeof(hdr_csv), filtered, filtered_count);
            headers_add(&r, "Access-Control-Allow-Headers", hdr_csv);
        }
        headers_add(&r, "Access-Control-Max-Age", "600");
        r.status = 204;
        r.body[0] = '\0';
        return r;
    }

    if (strcmp(pth, "/api/data") != 0) {
        r.status = 404;
        snprintf(r.body, sizeof(r.body), "%s", "{\"error\":\"not found\"}");
        return r;
    }

    if (!is_method_allowed(m)) {
        r.status = 405;
        snprintf(r.body, sizeof(r.body), "%s", "{\"error\":\"method not allowed\"}");
        return r;
    }

    if (originAllowed) {
        headers_add(&r, "Access-Control-Allow-Origin", origin ? origin : "");
        headers_add(&r, "Vary", "Origin");
        headers_add(&r, "Access-Control-Expose-Headers", "Content-Type, Content-Length");
    }
    headers_add(&r, "Content-Type", "application/json; charset=utf-8");
    r.status = 200;
    snprintf(r.body, sizeof(r.body), "{\"message\":\"Hello from CORS protected API\",\"path\":\"%s\"}", pth);
    return r;
}

static void print_response(const char* title, const Response* r) {
    printf("== %s ==\n", title);
    printf("Status: %d\n", r->status);
    for (int i = 0; i < r->header_count; ++i) {
        printf("%s: %s\n", r->header_name[i], r->header_value[i]);
    }
    printf("Body: %s\n\n", r->body);
}

int main(void) {
    Response t1 = handle_request("OPTIONS", "/api/data", "https://trustedorigin.com", "GET", "Content-Type, X-Requested-With");
    Response t2 = handle_request("OPTIONS", "/api/data", "https://evil.com", "GET", "Content-Type");
    Response t3 = handle_request("GET", "/api/data", "http://localhost:3000", NULL, NULL);
    Response t4 = handle_request("OPTIONS", "/api/data", "http://localhost:3000", "DELETE", "Content-Type");
    Response t5 = handle_request("GET", "/api/data", "https://unknown.com", NULL, NULL);

    print_response("Test1 Allowed Origin Preflight", &t1);
    print_response("Test2 Disallowed Origin Preflight", &t2);
    print_response("Test3 Allowed Origin Actual GET", &t3);
    print_response("Test4 Not Allowed Method Preflight", &t4);
    print_response("Test5 Disallowed Origin Actual GET", &t5);

    return 0;
}
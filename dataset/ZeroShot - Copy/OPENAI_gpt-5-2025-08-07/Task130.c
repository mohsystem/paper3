#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_HEADERS 20
#define MAX_KEY 64
#define MAX_VAL 256
#define MAX_BODY 5120
#define MAX_HEADER_LIST_LEN 1024
#define MAX_TOKENS 50

typedef struct {
    char key[MAX_KEY];
    char value[MAX_VAL];
} Header;

typedef struct {
    int status;
    Header headers[MAX_HEADERS];
    int header_count;
    char body[MAX_BODY];
} Response;

typedef struct {
    const char* items[8];
    int count;
} OriginSet;

typedef struct {
    const char* items[8];
    int count;
} MethodSet;

typedef struct {
    const char* items[8];
    int count;
} HeaderSet;

static const OriginSet ALLOWED_ORIGINS = {{"https://example.com","https://app.example.com"},2};
static const MethodSet ALLOWED_METHODS = {{"GET","POST","OPTIONS"},3};
static const HeaderSet ALLOWED_HEADERS = {{"content-type","authorization","x-requested-with"},3};
static const char* EXPOSE_HEADERS = "X-Response-Time";
static const int MAX_AGE = 600;

static void init_response(Response* r) {
    r->status = 200;
    r->header_count = 0;
    r->body[0] = '\0';
}

static void set_header(Response* r, const char* key, const char* value) {
    if (r->header_count >= MAX_HEADERS) return;
    strncpy(r->headers[r->header_count].key, key, MAX_KEY-1);
    r->headers[r->header_count].key[MAX_KEY-1] = '\0';
    strncpy(r->headers[r->header_count].value, value, MAX_VAL-1);
    r->headers[r->header_count].value[MAX_VAL-1] = '\0';
    r->header_count++;
}

static bool in_origin_set(const char* origin) {
    for (int i=0;i<ALLOWED_ORIGINS.count;i++) {
        if (strcmp(origin, ALLOWED_ORIGINS.items[i]) == 0) return true;
    }
    return false;
}

static bool in_method_set(const char* method) {
    for (int i=0;i<ALLOWED_METHODS.count;i++) {
        if (strcmp(method, ALLOWED_METHODS.items[i]) == 0) return true;
    }
    return false;
}

static bool in_header_set(const char* header_lower) {
    for (int i=0;i<ALLOWED_HEADERS.count;i++) {
        if (strcmp(header_lower, ALLOWED_HEADERS.items[i]) == 0) return true;
    }
    return false;
}

static void strtoupper(char* s) {
    for (; *s; ++s) *s = (char)toupper((unsigned char)*s);
}

static void strtolower(char* s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static void trim(char* s) {
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end-1])) end--;
    if (start > 0 || end < len) {
        memmove(s, s + start, end - start);
        s[end - start] = '\0';
    }
}

static bool is_token(const char* s) {
    size_t len = strlen(s);
    if (len == 0 || len > 100) return false;
    for (size_t i=0;i<len;i++) {
        if (!(isalnum((unsigned char)s[i]) || s[i] == '-')) return false;
    }
    return true;
}

static int parse_header_list(const char* list, char tokens[][101], int max_tokens) {
    if (list == NULL) return 0;
    size_t L = strlen(list);
    if (L > MAX_HEADER_LIST_LEN) return -1;
    int count = 0;
    char buf[MAX_HEADER_LIST_LEN+1];
    strncpy(buf, list, MAX_HEADER_LIST_LEN);
    buf[MAX_HEADER_LIST_LEN] = '\0';
    char* p = buf;
    while (*p && count < max_tokens) {
        char* comma = strchr(p, ',');
        char part[128];
        if (comma) {
            size_t n = (size_t)(comma - p);
            if (n >= sizeof(part)) n = sizeof(part)-1;
            strncpy(part, p, n);
            part[n] = '\0';
            p = comma + 1;
        } else {
            strncpy(part, p, sizeof(part)-1);
            part[sizeof(part)-1] = '\0';
            p += strlen(p);
        }
        trim(part);
        if (part[0] == '\0') continue;
        if (!is_token(part)) return -1;
        strncpy(tokens[count], part, 100);
        tokens[count][100] = '\0';
        count++;
    }
    return count;
}

static void json_escape(const char* in, char* out, size_t out_size) {
    size_t o = 0;
    if (out_size == 0) return;
    if (o + 1 < out_size) out[o++] = '"';
    for (size_t i=0; in[i] && o + 6 < out_size; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == '\\')      { out[o++]='\\'; out[o++]='\\'; }
        else if (c == '"')  { out[o++]='\\'; out[o++]='"'; }
        else if (c == '\b') { out[o++]='\\'; out[o++]='b'; }
        else if (c == '\f') { out[o++]='\\'; out[o++]='f'; }
        else if (c == '\n') { out[o++]='\\'; out[o++]='n'; }
        else if (c == '\r') { out[o++]='\\'; out[o++]='r'; }
        else if (c == '\t') { out[o++]='\\'; out[o++]='t'; }
        else if (c < 0x20) {
            int n = snprintf(out + o, out_size - o, "\\u%04x", c);
            if (n < 0) break;
            o += (size_t)n;
        } else {
            out[o++] = (char)c;
        }
    }
    if (o + 2 <= out_size) { out[o++]='"'; out[o]='\0'; }
    else out[out_size-1] = '\0';
}

Response handleRequest(const char* method,
                       const char* origin,
                       const char* accessControlRequestMethod,
                       const char* accessControlRequestHeaders,
                       const char* path,
                       const char* requestBody) {
    Response r;
    init_response(&r);

    char m[16];
    snprintf(m, sizeof(m), "%s", method ? method : "");
    strtoupper(m);

    char o[256];
    if (origin) {
        snprintf(o, sizeof(o), "%s", origin);
        trim(o);
    } else {
        o[0] = '\0';
    }
    bool hasOrigin = o[0] != '\0';

    if (strcmp(m, "OPTIONS") == 0) {
        set_header(&r, "Vary", "Origin, Access-Control-Request-Method, Access-Control-Request-Headers");
    } else {
        set_header(&r, "Vary", "Origin");
    }

    bool originAllowed = hasOrigin && in_origin_set(o);

    if (strcmp(m, "OPTIONS") == 0) {
        if (!originAllowed) {
            r.status = 403;
            return r;
        }
        char reqMethod[16];
        snprintf(reqMethod, sizeof(reqMethod), "%s", accessControlRequestMethod ? accessControlRequestMethod : "");
        strtoupper(reqMethod);
        if (!in_method_set(reqMethod) || strcmp(reqMethod, "OPTIONS") == 0) {
            r.status = 403;
            return r;
        }
        char tokens[MAX_TOKENS][101];
        int tcount = parse_header_list(accessControlRequestHeaders, tokens, MAX_TOKENS);
        if (tcount < 0) {
            r.status = 400;
            return r;
        }
        for (int i=0;i<tcount;i++) {
            char lower[101];
            snprintf(lower, sizeof(lower), "%s", tokens[i]);
            strtolower(lower);
            if (!in_header_set(lower)) {
                r.status = 403;
                return r;
            }
        }
        set_header(&r, "Access-Control-Allow-Origin", o);
        set_header(&r, "Access-Control-Allow-Credentials", "true");
        set_header(&r, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        char echo[MAX_VAL]="";
        for (int i=0;i<tcount;i++) {
            if (i>0) strncat(echo, ", ", sizeof(echo)-strlen(echo)-1);
            strncat(echo, tokens[i], sizeof(echo)-strlen(echo)-1);
        }
        set_header(&r, "Access-Control-Allow-Headers", echo);
        char maxage[16];
        snprintf(maxage, sizeof(maxage), "%d", MAX_AGE);
        set_header(&r, "Access-Control-Max-Age", maxage);
        r.status = 204;
        return r;
    }

    if (hasOrigin && !originAllowed) {
        r.status = 403;
        snprintf(r.body, sizeof(r.body), "%s", "{\"error\":\"CORS origin denied\"}");
        return r;
    }

    if (strcmp(path, "/api/data") == 0) {
        if (strcmp(m, "GET") == 0) {
            set_header(&r, "Content-Type", "application/json; charset=utf-8");
            set_header(&r, "X-Response-Time", "1");
            set_header(&r, "Cache-Control", "no-store");
            if (originAllowed) {
                set_header(&r, "Access-Control-Allow-Origin", o);
                set_header(&r, "Access-Control-Allow-Credentials", "true");
                set_header(&r, "Access-Control-Expose-Headers", EXPOSE_HEADERS);
            }
            // No portable millisecond timestamp; use placeholder
            snprintf(r.body, sizeof(r.body), "%s", "{\"message\":\"Hello\",\"ts\":0}");
            r.status = 200;
            return r;
        } else if (strcmp(m, "POST") == 0) {
            const char* rb = requestBody ? requestBody : "";
            char truncated[1025];
            strncpy(truncated, rb, 1024);
            truncated[1024] = '\0';
            char esc[2048];
            json_escape(truncated, esc, sizeof(esc));
            set_header(&r, "Content-Type", "application/json; charset=utf-8");
            set_header(&r, "X-Response-Time", "1");
            set_header(&r, "Cache-Control", "no-store");
            if (originAllowed) {
                set_header(&r, "Access-Control-Allow-Origin", o);
                set_header(&r, "Access-Control-Allow-Credentials", "true");
                set_header(&r, "Access-Control-Expose-Headers", EXPOSE_HEADERS);
            }
            snprintf(r.body, sizeof(r.body), "{\"received\":%s}", esc);
            r.status = 200;
            return r;
        } else {
            r.status = 405;
            return r;
        }
    } else {
        r.status = 404;
        return r;
    }
}

static void print_response(const char* label, const Response* r) {
    printf("=== %s ===\n", label);
    printf("Status: %d\n", r->status);
    for (int i=0;i<r->header_count;i++) {
        printf("%s: %s\n", r->headers[i].key, r->headers[i].value);
    }
    if (r->body[0] != '\0') {
        printf("Body: %s\n", r->body);
    }
}

int main(void) {
    Response r1 = handleRequest("OPTIONS", "https://example.com", "GET", "Content-Type, X-Requested-With", "/api/data", NULL);
    print_response("Test1 Preflight OK", &r1);

    Response r2 = handleRequest("GET", "https://app.example.com", NULL, NULL, "/api/data", NULL);
    print_response("Test2 GET with CORS", &r2);

    Response r3 = handleRequest("POST", "https://evil.com", NULL, NULL, "/api/data", "{\"a\":1}");
    print_response("Test3 POST disallowed origin", &r3);

    Response r4 = handleRequest("GET", NULL, NULL, NULL, "/api/data", NULL);
    print_response("Test4 GET no origin", &r4);

    Response r5 = handleRequest("OPTIONS", "https://example.com", "DELETE", "Authorization", "/api/data", NULL);
    print_response("Test5 Preflight bad method", &r5);

    return 0;
}
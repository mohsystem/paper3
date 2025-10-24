/*
Chain-of-Through process in comments:
1) Problem: Implement simple API-like handler that accepts user JSON and stores in a DB (JSONL file).
2) Security: Limit payload, validate/sanitize inputs, strict content-type, append-only I/O.
3) Implementation: Minimal JSON extraction for name/email/age; validators; JSON escaping; file append.
4) Review: Fixed DB path, bounds checks, no dynamic code execution.
5) Output: Final secure C code with 5 test cases.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#define MAX_BODY_SIZE 4096
#define DB_PATH "users_db.jsonl"

typedef struct {
    int statusCode;
    char body[256];
} Response;

static void sanitize(char* s) {
    if (!s) return;
    // Remove CR/LF and trim
    size_t len = strlen(s), j = 0;
    for (size_t i = 0; i < len; i++) {
        if (s[i] != '\r' && s[i] != '\n') s[j++] = s[i];
    }
    s[j] = '\0';
    // trim
    size_t start = 0;
    while (isspace((unsigned char)s[start])) start++;
    size_t end = strlen(s);
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    if (start > 0 || end < strlen(s)) {
        memmove(s, s + start, end - start);
        s[end - start] = '\0';
    }
}

static void json_escape(const char* in, char* out, size_t outcap) {
    size_t j = 0;
    for (size_t i = 0; in[i] && j + 6 < outcap; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == '\\') { out[j++]='\\'; out[j++]='\\'; }
        else if (c == '"') { out[j++]='\\'; out[j++]='"'; }
        else if (c == '\b') { out[j++]='\\'; out[j++]='b'; }
        else if (c == '\f') { out[j++]='\\'; out[j++]='f'; }
        else if (c == '\n') { out[j++]='\\'; out[j++]='n'; }
        else if (c == '\r') { out[j++]='\\'; out[j++]='r'; }
        else if (c == '\t') { out[j++]='\\'; out[j++]='t'; }
        else if (c < 0x20) {
            int n = snprintf(out + j, outcap - j, "\\u%04x", c);
            if (n < 0) break;
            j += (size_t)n;
        } else {
            out[j++] = c;
        }
    }
    out[j] = '\0';
}

static int validate_name(const char* name) {
    if (!name) return 0;
    size_t len = strlen(name);
    if (len < 1 || len > 100) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        if (!(isalnum(c) || c == ' ' || c == '-' || c == '\'' || c == '_' || c == '.')) return 0;
    }
    return 1;
}

static int validate_email(const char* email) {
    if (!email) return 0;
    size_t len = strlen(email);
    if (len < 3 || len > 254) return 0;
    const char* at = strchr(email, '@');
    if (!at || at == email || strchr(at + 1, '@')) return 0;
    const char* domain = at + 1;
    if (strlen(domain) < 3) return 0;
    if (!strchr(domain, '.')) return 0;
    // local allowed
    for (const char* p = email; p < at; ++p) {
        char c = *p;
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-')) return 0;
    }
    // domain allowed
    for (const char* p = domain; *p; ++p) {
        char c = *p;
        if (!(isalnum((unsigned char)c) || c == '.' || c == '-')) return 0;
    }
    if (domain[0] == '-' || domain[strlen(domain)-1] == '-') return 0;
    if (strstr(domain, "..")) return 0;
    const char* lastdot = strrchr(domain, '.');
    if (!lastdot || strlen(lastdot+1) < 2 || strlen(lastdot+1) > 24) return 0;
    for (const char* p = lastdot + 1; *p; ++p) if (!isalpha((unsigned char)*p)) return 0;
    return 1;
}

static int validate_age(int age) {
    return age >= 0 && age <= 150;
}

static int skip_ws(const char* s, int i) {
    while (s[i] && (s[i]==' '||s[i]=='\n'||s[i]=='\r'||s[i]=='\t')) i++;
    return i;
}

static int find_key(const char* json, const char* key) {
    // find "key"
    size_t klen = strlen(key);
    size_t len = strlen(json);
    int inStr = 0, esc = 0;
    for (size_t i = 0; i + klen + 2 <= len; i++) {
        char c = json[i];
        if (inStr) {
            if (esc) esc = 0;
            else if (c == '\\') esc = 1;
            else if (c == '"') inStr = 0;
        } else {
            if (c == '"') {
                if (strncmp(json + i + 1, key, klen) == 0 && json[i + 1 + klen] == '"') {
                    return (int)(i + 1 + (int)klen + 1);
                } else {
                    inStr = 1;
                }
            }
        }
    }
    return -1;
}

static int extract_json_string(const char* json, const char* key, char* out, size_t outcap) {
    int idx = find_key(json, key);
    if (idx < 0) return 0;
    int colon = skip_ws(json, idx);
    if (json[colon] != ':') return 0;
    int i = skip_ws(json, colon + 1);
    if (json[i] != '"') return 0;
    i++;
    size_t j = 0;
    int esc = 0;
    while (json[i]) {
        char c = json[i++];
        if (esc) {
            switch (c) {
                case '"': if (j+1<outcap) out[j++]='"'; break;
                case '\\': if (j+1<outcap) out[j++]='\\'; break;
                case '/': if (j+1<outcap) out[j++]='/'; break;
                case 'b': if (j+1<outcap) out[j++]='\b'; break;
                case 'f': if (j+1<outcap) out[j++]='\f'; break;
                case 'n': if (j+1<outcap) out[j++]='\n'; break;
                case 'r': if (j+1<outcap) out[j++]='\r'; break;
                case 't': if (j+1<outcap) out[j++]='\t'; break;
                case 'u': {
                    if (!json[i]||!json[i+1]||!json[i+2]||!json[i+3]) return 0;
                    char hex[5] = {json[i],json[i+1],json[i+2],json[i+3],0};
                    int val = 0;
                    if (sscanf(hex, "%x", &val) != 1) return 0;
                    if (j+1<outcap) out[j++] = (char)val;
                    i += 4;
                    break;
                }
                default: return 0;
            }
            esc = 0;
        } else {
            if (c == '\\') esc = 1;
            else if (c == '"') break;
            else {
                if (j+1<outcap) out[j++] = c;
            }
        }
    }
    out[j] = '\0';
    return 1;
}

static int extract_json_int(const char* json, const char* key, int* out) {
    int idx = find_key(json, key);
    if (idx < 0) return 0;
    int colon = skip_ws(json, idx);
    if (json[colon] != ':') return 0;
    int i = skip_ws(json, colon + 1);
    int neg = 0;
    if (json[i] == '-') { neg = 1; i++; }
    if (!isdigit((unsigned char)json[i])) return 0;
    long long val = 0;
    while (isdigit((unsigned char)json[i])) {
        val = val * 10 + (json[i] - '0');
        i++;
        if (val > 1000000) break;
    }
    if (neg) val = -val;
    if (val < -2147483648LL || val > 2147483647LL) return 0;
    *out = (int)val;
    return 1;
}

static void gen_uuid(char* out, size_t outcap) {
    // Simple pseudo UUID
    unsigned int r[4];
    for (int i = 0; i < 4; i++) r[i] = (unsigned int)rand();
    snprintf(out, outcap, "%08x-%04x-%04x-%04x-%08x%04x",
             r[0], (r[1] & 0xFFFF), ((r[1] >> 16) & 0x0FFF) | 0x4000,
             ((r[2]) & 0x3FFF) | 0x8000, r[3], (unsigned)(r[2] & 0xFFFF));
}

static int db_append_user(const char* id, const char* name, const char* email, int age, long created) {
    FILE* f = fopen(DB_PATH, "a");
    if (!f) return 0;
    char nameEsc[256], emailEsc[256], idEsc[64];
    json_escape(name, nameEsc, sizeof(nameEsc));
    json_escape(email, emailEsc, sizeof(emailEsc));
    json_escape(id, idEsc, sizeof(idEsc));
    int ok = fprintf(f, "{\"id\":\"%s\",\"name\":\"%s\",\"email\":\"%s\",\"age\":%d,\"createdAt\":%ld}\n",
            idEsc, nameEsc, emailEsc, age, created) > 0;
    fclose(f);
    return ok;
}

Response handle_request(const char* method, const char* path, const char* content_type, const char* body) {
    Response res;
    res.statusCode = 500;
    snprintf(res.body, sizeof(res.body), "{\"error\":\"Internal Server Error\"}");

    if (!method || !path) {
        res.statusCode = 400;
        snprintf(res.body, sizeof(res.body), "{\"error\":\"Bad Request\"}");
        return res;
    }
    char m[8]; memset(m, 0, sizeof(m));
    snprintf(m, sizeof(m), "%s", method);
    for (int i = 0; m[i]; ++i) m[i] = (char)toupper((unsigned char)m[i]);
    if (strcmp(m, "POST") != 0) {
        res.statusCode = 405;
        snprintf(res.body, sizeof(res.body), "{\"error\":\"Method Not Allowed\"}");
        return res;
    }
    if (strcmp(path, "/users") != 0) {
        res.statusCode = 404;
        snprintf(res.body, sizeof(res.body), "{\"error\":\"Not Found\"}");
        return res;
    }
    if (!body) body = "";
    if (strlen(body) > MAX_BODY_SIZE) {
        res.statusCode = 413;
        snprintf(res.body, sizeof(res.body), "{\"error\":\"Payload Too Large\"}");
        return res;
    }
    char ctbuf[64] = {0};
    if (content_type) {
        snprintf(ctbuf, sizeof(ctbuf), "%s", content_type);
        for (int i = 0; ctbuf[i]; ++i) ctbuf[i] = (char)tolower((unsigned char)ctbuf[i]);
    }
    if (strncmp(ctbuf, "application/json", 16) != 0) {
        res.statusCode = 415;
        snprintf(res.body, sizeof(res.body), "{\"error\":\"Unsupported Media Type\"}");
        return res;
    }

    // parse
    char name[128] = {0}, email[256] = {0};
    int age = -1;
    if (!extract_json_string(body, "name", name, sizeof(name))) {
        res.statusCode = 400; snprintf(res.body, sizeof(res.body), "{\"error\":\"Missing key: name\"}"); return res;
    }
    if (!extract_json_string(body, "email", email, sizeof(email))) {
        res.statusCode = 400; snprintf(res.body, sizeof(res.body), "{\"error\":\"Missing key: email\"}"); return res;
    }
    if (!extract_json_int(body, "age", &age)) {
        res.statusCode = 400; snprintf(res.body, sizeof(res.body), "{\"error\":\"Invalid age\"}"); return res;
    }
    sanitize(name);
    sanitize(email);

    if (!validate_name(name)) {
        res.statusCode = 400; snprintf(res.body, sizeof(res.body), "{\"error\":\"Invalid name\"}"); return res;
    }
    if (!validate_email(email)) {
        res.statusCode = 400; snprintf(res.body, sizeof(res.body), "{\"error\":\"Invalid email\"}"); return res;
    }
    if (!validate_age(age)) {
        res.statusCode = 400; snprintf(res.body, sizeof(res.body), "{\"error\":\"Invalid age\"}"); return res;
    }

    char id[64]; gen_uuid(id, sizeof(id));
    long created = time(NULL);
    if (!db_append_user(id, name, email, age, created)) {
        res.statusCode = 500; snprintf(res.body, sizeof(res.body), "{\"error\":\"Internal Server Error\"}"); return res;
    }
    char idEsc[96]; json_escape(id, idEsc, sizeof(idEsc));
    res.statusCode = 201;
    snprintf(res.body, sizeof(res.body), "{\"message\":\"User created\",\"id\":\"%s\"}", idEsc);
    return res;
}

int main(void) {
    srand((unsigned int)time(NULL));
    const char* ct = "application/json";
    const char* tests[5] = {
        "{\"name\":\"Alice Smith\",\"email\":\"alice@example.com\",\"age\":30}",
        "{\"name\":\"Bob\",\"email\":\"bob[at]example.com\",\"age\":25}",
        "{\"email\":\"charlie@example.com\",\"age\":40}",
        "{\"name\":\"Diana\",\"email\":\"diana@example.com\",\"age\":-5}",
        "{\"name\":\"Edward\",\"email\":\"edward@example.com\",\"age\":28}"
    };

    Response r1 = handle_request("POST", "/users", ct, tests[0]);
    printf("%d %s\n", r1.statusCode, r1.body);

    Response r2 = handle_request("POST", "/users", ct, tests[1]);
    printf("%d %s\n", r2.statusCode, r2.body);

    Response r3 = handle_request("POST", "/users", ct, tests[2]);
    printf("%d %s\n", r3.statusCode, r3.body);

    Response r4 = handle_request("POST", "/users", ct, tests[3]);
    printf("%d %s\n", r4.statusCode, r4.body);

    Response r5 = handle_request("GET", "/users", ct, tests[4]);
    printf("%d %s\n", r5.statusCode, r5.body);

    return 0;
}
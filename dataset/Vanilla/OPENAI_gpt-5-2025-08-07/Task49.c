#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Simple in-memory "database"
typedef struct {
    int id;
    char* name;
    char* email;
    int age;
} User;

typedef struct {
    User* data;
    size_t size;
    size_t cap;
    int next_id;
} DB;

static DB DB_USERS = {NULL, 0, 0, 1};

static void db_ensure_cap(DB* db) {
    if (db->size >= db->cap) {
        size_t ncap = db->cap == 0 ? 8 : db->cap * 2;
        User* ndata = (User*)realloc(db->data, ncap * sizeof(User));
        if (!ndata) exit(1);
        db->data = ndata;
        db->cap = ncap;
    }
}

static char* cstrdup(const char* s) {
    size_t n = strlen(s);
    char* r = (char*)malloc(n + 1);
    if (!r) exit(1);
    memcpy(r, s, n + 1);
    return r;
}

// Public API functions
int addUserC(const char* name, const char* email, int age) {
    db_ensure_cap(&DB_USERS);
    User u;
    u.id = DB_USERS.next_id++;
    u.name = cstrdup(name);
    u.email = cstrdup(email);
    u.age = age;
    DB_USERS.data[DB_USERS.size++] = u;
    return u.id;
}

static void json_escape(const char* s, char** out, size_t* len, size_t* cap) {
    for (const char* p = s; *p; ++p) {
        const char* add = NULL;
        char buf[3] = {0};
        if (*p == '\\') add = "\\\\";
        else if (*p == '\"') add = "\\\"";
        else if (*p == '\n') add = "\\n";
        else if (*p == '\r') add = "\\r";
        else {
            buf[0] = *p; buf[1] = '\0';
            add = buf;
        }
        size_t addlen = strlen(add);
        if (*len + addlen + 1 > *cap) {
            *cap = (*cap == 0 ? 64 : (*cap * 2 + addlen));
            *out = (char*)realloc(*out, *cap);
            if (!*out) exit(1);
        }
        memcpy(*out + *len, add, addlen);
        *len += addlen;
    }
}

char* getAllUsersJsonC() {
    size_t cap = 128, len = 0;
    char* out = (char*)malloc(cap);
    if (!out) exit(1);
    out[len++] = '[';
    for (size_t i = 0; i < DB_USERS.size; ++i) {
        if (i > 0) out[len++] = ',';
        if (len + 32 > cap) { cap *= 2; out = (char*)realloc(out, cap); if (!out) exit(1); }
        len += snprintf(out + len, cap - len, "{\"id\":%d,\"name\":\"", DB_USERS.data[i].id);
        char* esc = NULL; size_t esclen = 0, esccap = 0;
        json_escape(DB_USERS.data[i].name, &esc, &esclen, &esccap);
        if (len + esclen + 1 > cap) { cap = len + esclen + 128; out = (char*)realloc(out, cap); if (!out) exit(1); }
        memcpy(out + len, esc, esclen); len += esclen; free(esc);
        if (len + 20 > cap) { cap *= 2; out = (char*)realloc(out, cap); if (!out) exit(1); }
        memcpy(out + len, "\",\"email\":\"", 11); len += 11;

        esc = NULL; esclen = 0; esccap = 0;
        json_escape(DB_USERS.data[i].email, &esc, &esclen, &esccap);
        if (len + esclen + 1 > cap) { cap = len + esclen + 128; out = (char*)realloc(out, cap); if (!out) exit(1); }
        memcpy(out + len, esc, esclen); len += esclen; free(esc);
        if (len + 20 > cap) { cap *= 2; out = (char*)realloc(out, cap); if (!out) exit(1); }
        len += snprintf(out + len, cap - len, "\",\"age\":%d}", DB_USERS.data[i].age);
    }
    if (len + 2 > cap) { cap += 2; out = (char*)realloc(out, cap); if (!out) exit(1); }
    out[len++] = ']';
    out[len] = '\0';
    return out;
}

// Very naive JSON parsing for keys "name","email","age"
static int extractStringC(const char* json, const char* key, char** outStr) {
    size_t klen = strlen(key);
    const char* p = strstr(json, key);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p != '\"') return 0;
    p++;
    const char* start = p;
    while (*p && *p != '\"') p++;
    if (*p != '\"') return 0;
    size_t len = (size_t)(p - start);
    char* s = (char*)malloc(len + 1);
    if (!s) exit(1);
    memcpy(s, start, len);
    s[len] = '\0';
    *outStr = s;
    return 1;
}

static int extractIntC(const char* json, const char* key, int* outVal) {
    const char* p = strstr(json, key);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    while (*p && isspace((unsigned char)*p)) p++;
    int neg = 0;
    if (*p == '-') { neg = 1; p++; }
    if (!isdigit((unsigned char)*p)) return 0;
    long v = 0;
    while (*p && isdigit((unsigned char)*p)) { v = v * 10 + (*p - '0'); p++; }
    *outVal = (int)(neg ? -v : v);
    return 1;
}

// Simulated API endpoint handler
char* handleRequestC(const char* method, const char* path, const char* body) {
    if (strcmp(method, "POST") == 0 && strcmp(path, "/users") == 0) {
        char* name = NULL; char* email = NULL; int age;
        int ok = extractStringC(body, "\"name\"", &name) &&
                 extractStringC(body, "\"email\"", &email) &&
                 extractIntC(body, "\"age\"", &age);
        if (!ok) {
            const char* err = "{\"error\":\"Invalid input\"}";
            char* r = (char*)malloc(strlen(err) + 1);
            strcpy(r, err);
            if (name) free(name);
            if (email) free(email);
            return r;
        }
        int id = addUserC(name, email, age);
        free(name); free(email);
        char buf[64];
        snprintf(buf, sizeof(buf), "{\"status\":\"ok\",\"id\":%d}", id);
        char* r = (char*)malloc(strlen(buf) + 1);
        strcpy(r, buf);
        return r;
    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/users") == 0) {
        return getAllUsersJsonC();
    }
    const char* nf = "{\"error\":\"Not found\"}";
    char* r = (char*)malloc(strlen(nf) + 1);
    strcpy(r, nf);
    return r;
}

int main() {
    const char* payloads[5] = {
        "{\"name\":\"Alice\",\"email\":\"alice@example.com\",\"age\":30}",
        "{\"name\":\"Bob\",\"email\":\"bob@example.com\",\"age\":25}",
        "{\"name\":\"Carol\",\"email\":\"carol@example.com\",\"age\":28}",
        "{\"name\":\"Dave\",\"email\":\"dave@example.com\",\"age\":40}",
        "{\"name\":\"Eve\",\"email\":\"eve@example.com\",\"age\":22}"
    };
    for (int i = 0; i < 5; ++i) {
        char* resp = handleRequestC("POST", "/users", payloads[i]);
        printf("%s\n", resp);
        free(resp);
    }
    char* all = handleRequestC("GET", "/users", "");
    printf("%s\n", all);
    free(all);

    // free DB memory
    for (size_t i = 0; i < DB_USERS.size; ++i) {
        free(DB_USERS.data[i].name);
        free(DB_USERS.data[i].email);
    }
    free(DB_USERS.data);
    return 0;
}
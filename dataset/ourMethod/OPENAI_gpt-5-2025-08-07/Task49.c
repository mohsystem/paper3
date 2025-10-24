#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <sqlite3.h>

#define MAX_REQUEST_SIZE 8192
#define MAX_BODY_SIZE 4096
#define MAX_FIELD_LEN 256

static const char* DB_PATH = "users_c.db";

static char* str_dup_safe(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n);
    out[n] = '\0';
    return out;
}

static char* build_response(int status, const char* message, const char* extraJson) {
    const char* statusText = "Error";
    switch (status) {
        case 201: statusText = "Created"; break;
        case 200: statusText = "OK"; break;
        case 400: statusText = "Bad Request"; break;
        case 404: statusText = "Not Found"; break;
        case 405: statusText = "Method Not Allowed"; break;
        case 411: statusText = "Length Required"; break;
        case 413: statusText = "Payload Too Large"; break;
        case 415: statusText = "Unsupported Media Type"; break;
        case 422: statusText = "Unprocessable Entity"; break;
        case 409: statusText = "Conflict"; break;
        case 500: statusText = "Internal Server Error"; break;
    }

    // Escape message
    size_t msg_len = strlen(message);
    size_t esc_len = 0;
    for (size_t i = 0; i < msg_len; i++) {
        char c = message[i];
        if (c == '"' || c == '\\') esc_len += 2;
        else if ((unsigned char)c < 0x20) esc_len += 1; // replace with space
        else esc_len += 1;
    }
    char* esc_msg = (char*)malloc(esc_len + 1);
    if (!esc_msg) return str_dup_safe("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
    size_t p = 0;
    for (size_t i = 0; i < msg_len; i++) {
        char c = message[i];
        if (c == '"' || c == '\\') {
            esc_msg[p++] = '\\';
            esc_msg[p++] = c;
        } else if ((unsigned char)c < 0x20) {
            esc_msg[p++] = ' ';
        } else {
            esc_msg[p++] = c;
        }
    }
    esc_msg[p] = '\0';

    // Build JSON body
    size_t extra_len = (extraJson ? strlen(extraJson) : 0);
    size_t body_cap = 20 + esc_len + (extra_len ? 1 + extra_len : 0) + 4;
    char* body = (char*)malloc(body_cap + 1);
    if (!body) {
        free(esc_msg);
        return str_dup_safe("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
    }
    if (extra_len) {
        snprintf(body, body_cap + 1, "{\"message\":\"%s\",%s}", esc_msg, extraJson);
    } else {
        snprintf(body, body_cap + 1, "{\"message\":\"%s\"}", esc_msg);
    }
    free(esc_msg);

    // Headers
    char header[256];
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %d %s\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                     status, statusText, strlen(body));

    size_t total = (size_t)n + strlen(body);
    char* resp = (char*)malloc(total + 1);
    if (!resp) {
        free(body);
        return str_dup_safe("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
    }
    memcpy(resp, header, (size_t)n);
    memcpy(resp + n, body, strlen(body));
    resp[total] = '\0';
    free(body);
    return resp;
}

static void to_lower_inplace(char* s) {
    if (!s) return;
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static char* trim_dup(const char* s) {
    if (!s) return NULL;
    const char* b = s;
    while (*b && isspace((unsigned char)*b)) b++;
    const char* e = s + strlen(s);
    while (e > b && isspace((unsigned char)*(e - 1))) e--;
    size_t n = (size_t)(e - b);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, b, n);
    out[n] = '\0';
    return out;
}

static int validate_username(const char* u) {
    size_t n = strlen(u);
    if (n < 3 || n > 32) return 0;
    for (size_t i = 0; i < n; i++) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '_')) return 0;
    }
    return 1;
}

static int validate_email(const char* email) {
    size_t n = strlen(email);
    if (n < 3 || n > 254) return 0;
    for (size_t i = 0; i < n; i++) if (email[i] == ' ') return 0;
    char* at = strchr(email, '@');
    if (!at) return 0;
    if (strchr(at + 1, '@')) return 0;
    size_t local_len = (size_t)(at - email);
    const char* domain = at + 1;
    if (local_len == 0 || *domain == '\0') return 0;
    if (domain[0] == '.' || domain[strlen(domain) - 1] == '.') return 0;
    if (!strchr(domain, '.')) return 0;
    auto int_allowed = [](char c) { return isalnum((unsigned char)c) || c == '.' || c == '-' || c == '_' || c == '+'; };
    for (size_t i = 0; i < local_len; i++) if (!int_allowed(email[i])) return 0;
    for (const char* p = domain; *p; ++p) if (!int_allowed(*p)) return 0;
    return 1;
}

static int validate_age(int age) {
    return age >= 13 && age <= 120;
}

// JSON extract "key":"value"
static int json_extract_string(const char* body, const char* key, char* out, size_t out_sz) {
    if (!body || !key || !out || out_sz == 0) return 0;
    char pattern[64];
    int pn = snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    if (pn <= 0 || (size_t)pn >= sizeof(pattern)) return 0;
    const char* kpos = strstr(body, pattern);
    if (!kpos) return 0;
    const char* colon = strchr(kpos + pn, ':');
    if (!colon) return 0;
    const char* p = colon + 1;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p != '\"') return 0;
    p++;
    size_t idx = 0;
    while (*p && *p != '\"') {
        if (*p == '\\') {
            p++;
            if (*p == '\"' || *p == '\\') {
                if (idx + 1 >= out_sz) return 0;
                out[idx++] = *p++;
                continue;
            } else {
                return 0;
            }
        }
        if (idx + 1 >= out_sz) return 0;
        out[idx++] = *p++;
        if (idx >= MAX_FIELD_LEN) return 0;
    }
    if (*p != '\"') return 0;
    out[idx] = '\0';
    return 1;
}

// JSON extract "key":number
static int json_extract_int(const char* body, const char* key, int* out) {
    if (!body || !key || !out) return 0;
    char pattern[64];
    int pn = snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    if (pn <= 0 || (size_t)pn >= sizeof(pattern)) return 0;
    const char* kpos = strstr(body, pattern);
    if (!kpos) return 0;
    const char* colon = strchr(kpos + pn, ':');
    if (!colon) return 0;
    const char* p = colon + 1;
    while (*p && isspace((unsigned char)*p)) p++;
    int neg = 0;
    if (*p == '-') { neg = 1; p++; }
    if (!isdigit((unsigned char)*p)) return 0;
    long long val = 0;
    while (isdigit((unsigned char)*p)) {
        val = val * 10 + (*p - '0');
        if (val > INT_MAX) return 0;
        p++;
    }
    if (neg) val = -val;
    if (val < INT_MIN || val > INT_MAX) return 0;
    *out = (int)val;
    return 1;
}

static int init_db(void) {
    sqlite3* db = NULL;
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 0;
    }
    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "email TEXT NOT NULL,"
        "age INTEGER NOT NULL CHECK(age >= 0)"
        ");";
    char* err = NULL;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

static int insert_user(const char* username, const char* email, int age, int* conflict) {
    if (conflict) *conflict = 0;
    sqlite3* db = NULL;
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 0;
    }
    const char* sql = "INSERT INTO users(username,email,age) VALUES(?,?,?);";
    sqlite3_stmt* stmt = NULL;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }
    sqlite3_bind_text(stmt, 1, username, (int)strlen(username), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email, (int)strlen(email), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, age);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        if (rc == SQLITE_CONSTRAINT || rc == SQLITE_CONSTRAINT_UNIQUE) {
            if (conflict) *conflict = 1;
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 1;
}

char* handle_request(const char* requestText) {
    if (!requestText) return build_response(400, "Empty request", NULL);
    size_t req_len = strlen(requestText);
    if (req_len > MAX_REQUEST_SIZE) return build_response(413, "Request too large", NULL);

    const char* header_end = strstr(requestText, "\r\n\r\n");
    if (!header_end) return build_response(400, "Malformed HTTP request", NULL);
    size_t header_len = (size_t)(header_end - requestText);
    size_t body_len = req_len - header_len - 4;
    const char* header = requestText;
    const char* body = header_end + 4;

    // Request line
    const char* line_end = strstr(header, "\r\n");
    size_t line_len = line_end ? (size_t)(line_end - header) : header_len;
    char* req_line = (char*)malloc(line_len + 1);
    if (!req_line) return build_response(500, "Memory error", NULL);
    memcpy(req_line, header, line_len);
    req_line[line_len] = '\0';

    char method[8] = {0}, path[64] = {0}, version[16] = {0};
    if (sscanf(req_line, "%7s %63s %15s", method, path, version) != 3) {
        free(req_line);
        return build_response(400, "Invalid request line", NULL);
    }
    free(req_line);
    if (strcmp(method, "POST") != 0) return build_response(405, "Only POST allowed", NULL);
    if (strcmp(path, "/users") != 0) return build_response(404, "Not Found", NULL);
    if (strncmp(version, "HTTP/", 5) != 0) return build_response(400, "Invalid HTTP version", NULL);

    // Parse headers
    size_t pos = line_len + 2;
    char content_type[128] = {0};
    char content_length_str[32] = {0};
    int have_ct = 0, have_cl = 0;

    while (pos < header_len) {
        const char* lstart = header + pos;
        const char* lend = strstr(lstart, "\r\n");
        size_t llen = lend ? (size_t)(lend - lstart) : (header_len - pos);
        if (llen == 0) break;
        const char* colon = memchr(lstart, ':', llen);
        if (colon) {
            size_t name_len = (size_t)(colon - lstart);
            size_t value_len = llen - name_len - 1;
            char* name = (char*)malloc(name_len + 1);
            char* value = (char*)malloc(value_len + 1);
            if (!name || !value) {
                if (name) free(name);
                if (value) free(value);
                return build_response(500, "Memory error", NULL);
            }
            memcpy(name, lstart, name_len); name[name_len] = '\0';
            memcpy(value, colon + 1, value_len); value[value_len] = '\0';
            char* name_t = trim_dup(name);
            char* value_t = trim_dup(value);
            free(name);
            free(value);
            if (!name_t || !value_t) {
                if (name_t) free(name_t);
                if (value_t) free(value_t);
                return build_response(500, "Memory error", NULL);
            }
            to_lower_inplace(name_t);
            if (strcmp(name_t, "content-type") == 0) {
                strncpy(content_type, value_t, sizeof(content_type) - 1);
                have_ct = 1;
            } else if (strcmp(name_t, "content-length") == 0) {
                strncpy(content_length_str, value_t, sizeof(content_length_str) - 1);
                have_cl = 1;
            }
            free(name_t);
            free(value_t);
        }
        pos += llen + 2;
    }

    if (!have_cl) return build_response(411, "Content-Length required", NULL);
    for (size_t i = 0; content_length_str[i]; i++) {
        if (!isdigit((unsigned char)content_length_str[i])) return build_response(400, "Invalid Content-Length", NULL);
        if (i > 9) return build_response(400, "Invalid Content-Length", NULL);
    }
    unsigned long cl = strtoul(content_length_str, NULL, 10);
    if (cl > MAX_BODY_SIZE) return build_response(413, "Payload too large", NULL);
    if (cl != body_len) return build_response(400, "Content-Length mismatch", NULL);

    if (!have_ct) return build_response(415, "Content-Type required", NULL);
    char ct_lower[128];
    strncpy(ct_lower, content_type, sizeof(ct_lower) - 1);
    ct_lower[sizeof(ct_lower) - 1] = '\0';
    to_lower_inplace(ct_lower);
    if (strncmp(ct_lower, "application/json", 16) != 0) return build_response(415, "Unsupported Media Type", NULL);

    // Parse JSON fields
    char username[MAX_FIELD_LEN + 1] = {0};
    char email[MAX_FIELD_LEN + 1] = {0};
    int age = 0;
    if (!json_extract_string(body, "username", username, sizeof(username))) {
        return build_response(422, "Missing or invalid 'username'", NULL);
    }
    if (!json_extract_string(body, "email", email, sizeof(email))) {
        return build_response(422, "Missing or invalid 'email'", NULL);
    }
    if (!json_extract_int(body, "age", &age)) {
        return build_response(422, "Missing or invalid 'age'", NULL);
    }

    // Validate
    if (!validate_username(username)) {
        return build_response(422, "Invalid username: 3-32 chars, letters/digits/underscore", NULL);
    }
    if (!validate_email(email)) {
        return build_response(422, "Invalid email format", NULL);
    }
    if (!validate_age(age)) {
        return build_response(422, "Invalid age: must be 13..120", NULL);
    }

    if (!init_db()) {
        return build_response(500, "Database initialization error", NULL);
    }
    int conflict = 0;
    if (!insert_user(username, email, age, &conflict)) {
        if (conflict) {
            return build_response(409, "Username already exists", NULL);
        }
        return build_response(500, "Database insert error", NULL);
    }

    // Build extra JSON
    char safe_user[MAX_FIELD_LEN * 2 + 1];
    char safe_email[MAX_FIELD_LEN * 2 + 1];
    size_t su = 0, se = 0;
    for (size_t i = 0; username[i] && su + 2 < sizeof(safe_user); i++) {
        char c = username[i];
        if (c == '"' || c == '\\') { safe_user[su++] = '\\'; safe_user[su++] = c; }
        else safe_user[su++] = c;
    }
    safe_user[su] = '\0';
    for (size_t i = 0; email[i] && se + 2 < sizeof(safe_email); i++) {
        char c = email[i];
        if (c == '"' || c == '\\') { safe_email[se++] = '\\'; safe_email[se++] = c; }
        else safe_email[se++] = c;
    }
    safe_email[se] = '\0';

    char extra[512];
    snprintf(extra, sizeof(extra), "\"user\":{\"username\":\"%s\",\"email\":\"%s\",\"age\":%d}", safe_user, safe_email, age);
    return build_response(201, "User created", extra);
}

// 5 test cases
int main(void) {
    {
        const char* body = "{\"username\":\"alice_01\",\"email\":\"alice@example.com\",\"age\":30}";
        char req[1024];
        snprintf(req, sizeof(req),
                 "POST /users HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s",
                 strlen(body), body);
        char* resp = handle_request(req);
        puts("Test 1: Valid create");
        puts(resp);
        free(resp);
        puts("");
    }
    {
        const char* body = "{\"username\":\"alice_01\",\"email\":\"alice2@example.com\",\"age\":31}";
        char req[1024];
        snprintf(req, sizeof(req),
                 "POST /users HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %zu\r\n\r\n%s",
                 strlen(body), body);
        char* resp = handle_request(req);
        puts("Test 2: Duplicate username");
        puts(resp);
        free(resp);
        puts("");
    }
    {
        const char* body = "{\"username\":\"bob\",\"email\":\"bob_at_example.com\",\"age\":25}";
        char req[1024];
        snprintf(req, sizeof(req),
                 "POST /users HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s",
                 strlen(body), body);
        char* resp = handle_request(req);
        puts("Test 3: Invalid email");
        puts(resp);
        free(resp);
        puts("");
    }
    {
        const char* body = "";
        char req[512];
        snprintf(req, sizeof(req),
                 "GET /users HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s",
                 strlen(body), body);
        char* resp = handle_request(req);
        puts("Test 4: Wrong method");
        puts(resp);
        free(resp);
        puts("");
    }
    {
        const char* body = "{\"username\":\"carol\",\"email\":\"carol@example.com\",\"age\":22}";
        char req[1024];
        snprintf(req, sizeof(req),
                 "POST /unknown HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s",
                 strlen(body), body);
        char* resp = handle_request(req);
        puts("Test 5: Wrong path");
        puts(resp);
        free(resp);
        puts("");
    }
    return 0;
}
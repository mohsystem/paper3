#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SESSIONS 32

typedef struct {
    char sid[65];
    char csrf[65];
    char username[64];
    char email[128];
    int in_use;
} Session;

static Session sessions[MAX_SESSIONS];

static void init_sessions() {
    static int inited = 0;
    if (!inited) {
        memset(sessions, 0, sizeof(sessions));
        srand((unsigned int)(time(NULL) ^ (uintptr_t)&inited));
        inited = 1;
    }
}

static void rand_hex(char* out, size_t nbytes) {
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < nbytes; ++i) {
        unsigned int b = (unsigned int)(rand() & 0xFF);
        out[i*2] = hex[(b >> 4) & 0xF];
        out[i*2 + 1] = hex[b & 0xF];
    }
    out[nbytes*2] = '\0';
}

static Session* find_session_by_sid(const char* sid) {
    if (!sid || !*sid) return NULL;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sessions[i].in_use && strcmp(sessions[i].sid, sid) == 0) return &sessions[i];
    }
    return NULL;
}

static Session* create_session() {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!sessions[i].in_use) {
            sessions[i].in_use = 1;
            rand_hex(sessions[i].sid, 16);
            sessions[i].csrf[0] = '\0';
            strcpy(sessions[i].username, "default_user");
            strcpy(sessions[i].email, "user@example.com");
            return &sessions[i];
        }
    }
    return NULL;
}

static char* parse_sid_from_cookie(const char* cookie) {
    if (!cookie) return NULL;
    const char* p = cookie;
    while (*p) {
        while (*p == ' ' || *p == ';') p++;
        if (strncmp(p, "SID=", 4) == 0) {
            p += 4;
            const char* start = p;
            while (*p && *p != ';' && *p != '\n' && *p != '\r') p++;
            size_t len = (size_t)(p - start);
            char* sid = (char*)malloc(len + 1);
            if (!sid) return NULL;
            memcpy(sid, start, len);
            sid[len] = '\0';
            return sid;
        }
        while (*p && *p != ';') p++;
    }
    return NULL;
}

// Returns a newly allocated string; caller can free.
char* process_request(const char* method, const char* path, const char* cookieHeader,
                      const char* form_csrf, const char* form_username, const char* form_email) {
    init_sessions();
    char* response = (char*)malloc(8192);
    if (!response) return NULL;
    response[0] = '\0';

    if (strcmp(path, "/settings") != 0) {
        snprintf(response, 8192, "HTTP/1.1 404 Not Found\nContent-Type: text/plain; charset=utf-8\n\nNot Found");
        return response;
    }

    char* sid_from_cookie = parse_sid_from_cookie(cookieHeader);
    Session* session = NULL;
    if (sid_from_cookie) {
        session = find_session_by_sid(sid_from_cookie);
        free(sid_from_cookie);
    }
    if (!session) {
        session = create_session();
        if (!session) {
            snprintf(response, 8192, "HTTP/1.1 500 Internal Server Error\nContent-Type: text/plain; charset=utf-8\n\nNo session slots");
            return response;
        }
    }

    if (strcasecmp(method, "GET") == 0) {
        rand_hex(session->csrf, 16);
        char body[4096];
        snprintf(body, sizeof(body),
                 "<!doctype html><html><head><title>Settings</title></head><body>"
                 "<h1>User Settings</h1>"
                 "<form method=\"POST\" action=\"/settings\">"
                 "<input type=\"hidden\" name=\"csrf_token\" value=\"%s\"/>"
                 "<label>Username: <input type=\"text\" name=\"username\" value=\"%s\"/></label><br/>"
                 "<label>Email: <input type=\"email\" name=\"email\" value=\"%s\"/></label><br/>"
                 "<button type=\"submit\">Update</button>"
                 "</form>"
                 "</body></html>",
                 session->csrf, session->username, session->email);

        snprintf(response, 8192,
                 "HTTP/1.1 200 OK\n"
                 "Set-Cookie: SID=%s; HttpOnly; SameSite=Strict\n"
                 "Content-Type: text/html; charset=utf-8\n\n"
                 "%s",
                 session->sid, body);
        return response;
    } else if (strcasecmp(method, "POST") == 0) {
        if (form_csrf && session->csrf[0] != '\0' && strcmp(form_csrf, session->csrf) == 0) {
            if (form_username && *form_username) {
                strncpy(session->username, form_username, sizeof(session->username) - 1);
                session->username[sizeof(session->username) - 1] = '\0';
            }
            if (form_email && *form_email) {
                strncpy(session->email, form_email, sizeof(session->email) - 1);
                session->email[sizeof(session->email) - 1] = '\0';
            }
            // Rotate token
            rand_hex(session->csrf, 16);
            char body[256];
            snprintf(body, sizeof(body), "Settings updated successfully for %s (%s)", session->username, session->email);
            snprintf(response, 8192,
                     "HTTP/1.1 200 OK\n"
                     "Set-Cookie: SID=%s; HttpOnly; SameSite=Strict\n"
                     "Content-Type: text/plain; charset=utf-8\n\n"
                     "%s",
                     session->sid, body);
            return response;
        } else {
            snprintf(response, 8192,
                     "HTTP/1.1 403 Forbidden\n"
                     "Set-Cookie: SID=%s; HttpOnly; SameSite=Strict\n"
                     "Content-Type: text/plain; charset=utf-8\n\n"
                     "Forbidden: invalid CSRF token",
                     session->sid);
            return response;
        }
    } else {
        snprintf(response, 8192,
                 "HTTP/1.1 405 Method Not Allowed\n"
                 "Allow: GET, POST\n"
                 "Content-Type: text/plain; charset=utf-8\n\n"
                 "Method Not Allowed");
        return response;
    }
}

static char* extract_set_cookie_sid(const char* response) {
    const char* marker = "Set-Cookie: SID=";
    const char* p = strstr(response, marker);
    if (!p) return NULL;
    p += strlen(marker);
    const char* end = strchr(p, '\n');
    size_t len = end ? (size_t)(end - p) : strlen(p);
    // trim to semicolon
    const char* semi = memchr(p, ';', len);
    if (semi) len = (size_t)(semi - p);
    char* sid = (char*)malloc(len + 1);
    if (!sid) return NULL;
    memcpy(sid, p, len);
    sid[len] = '\0';
    return sid;
}

static char* extract_csrf_token(const char* response) {
    const char* name = "name=\"csrf_token\"";
    const char* n = strstr(response, name);
    if (!n) return NULL;
    const char* vm = "value=\"";
    const char* v = strstr(n, vm);
    if (!v) return NULL;
    v += strlen(vm);
    const char* end = strchr(v, '"');
    if (!end) return NULL;
    size_t len = (size_t)(end - v);
    char* token = (char*)malloc(len + 1);
    if (!token) return NULL;
    memcpy(token, v, len);
    token[len] = '\0';
    return token;
}

int main() {
    // Test 1: GET (no cookie)
    char* r1 = process_request("GET", "/settings", "", "", "", "");
    printf("=== Test 1: GET /settings (no cookie) ===\n%s\n", r1);

    char* sid = extract_set_cookie_sid(r1);
    char cookie[128] = {0};
    if (sid) {
        snprintf(cookie, sizeof(cookie), "SID=%s", sid);
    }

    char* csrf1 = extract_csrf_token(r1);

    // Test 2: POST with valid CSRF
    char* r2 = process_request("POST", "/settings", cookie, csrf1 ? csrf1 : "", "alice", "alice@example.com");
    printf("=== Test 2: POST /settings (valid CSRF) ===\n%s\n", r2);

    // Test 3: POST missing CSRF
    char* r3 = process_request("POST", "/settings", cookie, "", "mallory", "mallory@example.com");
    printf("=== Test 3: POST /settings (missing CSRF) ===\n%s\n", r3);

    // Test 4: POST invalid CSRF
    char* r4 = process_request("POST", "/settings", cookie, "badtoken", "bob", "bob@example.com");
    printf("=== Test 4: POST /settings (invalid CSRF) ===\n%s\n", r4);

    // Test 5: GET new token then POST with valid token
    char* r5a = process_request("GET", "/settings", cookie, "", "", "");
    char* csrf_new = extract_csrf_token(r5a);
    char* r5 = process_request("POST", "/settings", cookie, csrf_new ? csrf_new : "", "charlie", "charlie@example.com");
    printf("=== Test 5: POST /settings after fresh GET (valid CSRF) ===\n%s\n", r5);

    // Free allocated memory
    free(r1);
    free(r2);
    free(r3);
    free(r4);
    free(r5a);
    free(r5);
    free(sid);
    free(csrf1);
    free(csrf_new);

    return 0;
}
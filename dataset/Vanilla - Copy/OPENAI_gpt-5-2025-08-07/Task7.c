#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define USER_DB "users.db"

typedef struct {
    char username[128];
    int authenticated;
} SessionEntry;

static SessionEntry sessions[128];
static int session_count = 0;

// Utility: check if session exists
int is_authenticated(const char* username) {
    for (int i = 0; i < session_count; ++i) {
        if (strcmp(sessions[i].username, username) == 0 && sessions[i].authenticated) {
            return 1;
        }
    }
    return 0;
}

// Utility: set session authenticated
void set_authenticated(const char* username) {
    for (int i = 0; i < session_count; ++i) {
        if (strcmp(sessions[i].username, username) == 0) {
            sessions[i].authenticated = 1;
            return;
        }
    }
    if (session_count < 128) {
        strncpy(sessions[session_count].username, username, sizeof(sessions[session_count].username)-1);
        sessions[session_count].username[sizeof(sessions[session_count].username)-1] = '\0';
        sessions[session_count].authenticated = 1;
        session_count++;
    }
}

// Client: Build login request
char* client_build_login_request(const char* username, const char* password) {
    size_t len = strlen("LOGIN||") + strlen(username) + strlen(password) + 1;
    char* req = (char*)malloc(len);
    if (!req) return NULL;
    snprintf(req, len, "LOGIN|%s|%s", username, password);
    return req;
}

// Server: Ensure user DB
void ensure_user_db(const char* users[][2], int count) {
    FILE* f = fopen(USER_DB, "w");
    if (!f) return;
    for (int i = 0; i < count; ++i) {
        fprintf(f, "%s:%s\n", users[i][0], users[i][1]);
    }
    fclose(f);
}

// Server: Authenticate
int authenticate(const char* username, const char* password) {
    FILE* f = fopen(USER_DB, "r");
    if (!f) return 0;
    char line[512];
    int ok = 0;
    while (fgets(line, sizeof(line), f)) {
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        char* colon = strchr(line, ':');
        if (!colon) continue;
        *colon = '\0';
        const char* u = line;
        const char* p = colon + 1;
        if (strcmp(u, username) == 0 && strcmp(p, password) == 0) {
            ok = 1;
            break;
        }
    }
    fclose(f);
    return ok;
}

// Server: Process request
char* server_process(const char* request) {
    if (!request || !*request) {
        char* resp = (char*)malloc(24);
        strcpy(resp, "ERR|INVALID|EmptyRequest");
        return resp;
    }
    // Tokenize by '|'
    char* req_copy = strdup(request);
    if (!req_copy) {
        char* resp = (char*)malloc(22);
        strcpy(resp, "ERR|INVALID|Malformed");
        return resp;
    }
    char* saveptr = NULL;
    char* parts[64];
    int count = 0;
    char* tok = strtok_r(req_copy, "|", &saveptr);
    while (tok && count < 64) {
        parts[count++] = tok;
        tok = strtok_r(NULL, "|", &saveptr);
    }
    if (count == 0) {
        free(req_copy);
        char* resp = (char*)malloc(22);
        strcpy(resp, "ERR|INVALID|Malformed");
        return resp;
    }
    // Uppercase action
    for (char* p = parts[0]; *p; ++p) *p = (char)toupper((unsigned char)*p);

    if (strcmp(parts[0], "LOGIN") == 0) {
        if (count != 3) {
            free(req_copy);
            char* resp = (char*)malloc(20);
            strcpy(resp, "ERR|LOGIN|InvalidArgs");
            return resp;
        }
        const char* username = parts[1];
        const char* password = parts[2];
        if (authenticate(username, password)) {
            set_authenticated(username);
            size_t len = strlen("OK|LOGIN|Welcome ") + strlen(username) + 1;
            char* resp = (char*)malloc(len);
            snprintf(resp, len, "OK|LOGIN|Welcome %s", username);
            free(req_copy);
            return resp;
        } else {
            free(req_copy);
            char* resp = (char*)malloc(20);
            strcpy(resp, "ERR|LOGIN|Unauthorized");
            return resp;
        }
    } else if (strcmp(parts[0], "SEND") == 0) {
        if (count < 4) {
            free(req_copy);
            char* resp = (char*)malloc(19);
            strcpy(resp, "ERR|SEND|InvalidArgs");
            return resp;
        }
        const char* from = parts[1];
        const char* to = parts[2];
        // Rebuild message (remaining after the 3rd token)
        const char* orig = request;
        // Find the start of message by skipping "SEND|from|to|"
        // We'll find the third '|' position
        int pipe_count = 0;
        const char* msg_start = NULL;
        for (const char* c = request; *c; ++c) {
            if (*c == '|') {
                pipe_count++;
                if (pipe_count == 3) {
                    msg_start = c + 1;
                    break;
                }
            }
        }
        if (!is_authenticated(from)) {
            free(req_copy);
            char* resp = (char*)malloc(22);
            strcpy(resp, "ERR|SEND|NotAuthenticated");
            return resp;
        }
        FILE* log = fopen("chat.log", "a");
        if (!log) {
            free(req_copy);
            char* resp = (char*)malloc(19);
            strcpy(resp, "ERR|SEND|StorageError");
            return resp;
        }
        fprintf(log, "FROM:%s TO:%s MSG:%s\n", from, to, msg_start ? msg_start : "");
        fclose(log);
        size_t len = strlen("OK|SEND|DeliveredTo|") + strlen(to) + 1;
        char* resp = (char*)malloc(len);
        snprintf(resp, len, "OK|SEND|DeliveredTo|%s", to);
        free(req_copy);
        return resp;
    } else {
        free(req_copy);
        char* resp = (char*)malloc(24);
        strcpy(resp, "ERR|INVALID|UnknownAction");
        return resp;
    }
}

// Client: Send login
char* client_send_login(const char* username, const char* password) {
    char* req = client_build_login_request(username, password);
    if (!req) return NULL;
    char* resp = server_process(req);
    free(req);
    return resp;
}

int main() {
    const char* users[][2] = { {"alice","password123"},{"bob","letmein"} };
    ensure_user_db(users, 2);

    char* r1 = client_send_login("alice", "password123");
    printf("Test1: %s\n", r1 ? r1 : "(null)");
    free(r1);

    char* r2 = client_send_login("alice", "wrong");
    printf("Test2: %s\n", r2 ? r2 : "(null)");
    free(r2);

    char* r3 = client_send_login("charlie", "nopass");
    printf("Test3: %s\n", r3 ? r3 : "(null)");
    free(r3);

    char* r4 = client_send_login("bob", "letmein");
    printf("Test4: %s\n", r4 ? r4 : "(null)");
    free(r4);

    char* r5 = server_process("SEND|alice|bob|Hello Bob!");
    printf("Test5: %s\n", r5 ? r5 : "(null)");
    free(r5);

    return 0;
}
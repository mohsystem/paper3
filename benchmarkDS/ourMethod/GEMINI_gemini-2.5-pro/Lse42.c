// This code requires the Mongoose library and a POSIX-compliant system (for /dev/urandom).
// 1. Download mongoose.h and mongoose.c from https://github.com/cesanta/mongoose
// 2. Place them in the same directory as this file.
// 3. Compile: gcc -o server_c Lse42.c mongoose.c

#include "mongoose.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_SESSIONS 100
#define SESSION_ID_HEX_LEN 64 // 32 random bytes -> 64 hex chars
#define SESSION_ID_BYTES 32

typedef struct {
    char username[64];
    char email[128];
    char ssn[16];
} UserData;

typedef struct {
    char session_id[SESSION_ID_HEX_LEN + 1];
    UserData data;
    bool active;
} Session;

// A simple in-memory session store. Not thread-safe.
static Session s_sessions[MAX_SESSIONS];

void init_sessions() {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        s_sessions[i].active = false;
        // Securely zero out sensitive fields
        memset(s_sessions[i].session_id, 0, sizeof(s_sessions[i].session_id));
        memset(&s_sessions[i].data, 0, sizeof(UserData));
    }
}

// Generates a secure random session ID using /dev/urandom (POSIX-specific).
// Returns 0 on success, -1 on failure.
int generate_session_id(char *hex_buffer, size_t hex_len) {
    if (hex_len < (SESSION_ID_BYTES * 2 + 1)) return -1;

    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open /dev/urandom");
        return -1;
    }
    unsigned char random_data[SESSION_ID_BYTES];
    ssize_t result = read(fd, random_data, sizeof(random_data));
    close(fd);

    if (result != sizeof(random_data)) {
        fprintf(stderr, "Failed to read from /dev/urandom\n");
        return -1;
    }

    for (size_t i = 0; i < sizeof(random_data); i++) {
        snprintf(hex_buffer + i * 2, 3, "%02x", random_data[i]);
    }
    hex_buffer[hex_len-1] = '\0';
    return 0;
}

static void event_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev != MG_EV_HTTP_MSG) return;

    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    const char *path = hm->uri.ptr;
    
    if (mg_http_match_uri(hm, "/login_test/*")) {
        const char *user_start = path + 12; // Skip "/login_test/"
        size_t user_len = hm->uri.len - 12;

        int session_idx = -1;
        for (int i = 0; i < MAX_SESSIONS; i++) {
            if (!s_sessions[i].active) {
                session_idx = i;
                break;
            }
        }
        if (session_idx == -1) {
            mg_http_reply(c, 503, "", "Server busy, no sessions available\n");
            return;
        }

        Session *s = &s_sessions[session_idx];
        if (generate_session_id(s->session_id, sizeof(s->session_id)) != 0) {
             mg_http_reply(c, 500, "", "Failed to generate session ID\n");
             return;
        }

        // Use safe string functions to populate user data
        strncpy(s->data.username, user_start, sizeof(s->data.username) - 1);
        s->data.username[sizeof(s->data.username) - 1] = '\0';
        snprintf(s->data.email, sizeof(s->data.email), "%.*s@example.com", (int)user_len, user_start);
        strncpy(s->data.ssn, "000-00-0000", sizeof(s->data.ssn) - 1);
        s->data.ssn[sizeof(s->data.ssn) - 1] = '\0';
        s->active = true;

        mg_http_reply(c, 200, "Content-Type: text/plain\r\nSet-Cookie: session_id=%s; Path=/; HttpOnly; SameSite=Lax\r\n", s->session_id, "Session created\n");
    } else if (mg_http_match_uri(hm, "/info")) {
        char session_id[SESSION_ID_HEX_LEN + 1] = {0};
        mg_http_get_var(&hm->headers, "Cookie", "session_id", session_id, sizeof(session_id));
        
        Session *s = NULL;
        if (session_id[0] != '\0') {
             for(int i = 0; i < MAX_SESSIONS; i++) {
                if (s_sessions[i].active && strncmp(s_sessions[i].session_id, session_id, SESSION_ID_HEX_LEN) == 0) {
                    s = &s_sessions[i];
                    break;
                }
            }
        }
        
        if (s) {
            // SECURITY WARNING: Exposing PII is extremely dangerous.
            mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"username\":\"%s\",\"email\":\"%s\",\"ssn\":\"%s\"}\n", s->data.username, s->data.email, s->data.ssn);
        } else {
            mg_http_reply(c, 401, "Content-Type: application/json\r\n", "{\"error\":\"Unauthorized\"}\n");
        }
    } else if (mg_http_match_uri(hm, "/logout_test")) {
        char session_id[SESSION_ID_HEX_LEN + 1] = {0};
        mg_http_get_var(&hm->headers, "Cookie", "session_id", session_id, sizeof(session_id));
        if (session_id[0] != '\0') {
            for(int i = 0; i < MAX_SESSIONS; i++) {
               if (s_sessions[i].active && strncmp(s_sessions[i].session_id, session_id, SESSION_ID_HEX_LEN) == 0) {
                  s_sessions[i].active = false;
                  memset(s_sessions[i].session_id, 0, sizeof(s_sessions[i].session_id)); // Clear sensitive data
                  break;
               }
           }
        }
        mg_http_reply(c, 200, "Content-Type: text/plain\r\nSet-Cookie: session_id=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT\r\n", "Session cleared.\n");
    } else {
        mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Not Found\n");
    }
    (void) fn_data;
}

int main(void) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    init_sessions();
    
    const char *addr = "http://localhost:8001";
    if (mg_http_listen(&mgr, addr, event_handler, NULL) == NULL) {
        fprintf(stderr, "Failed to start server on %s\n", addr);
        return 1;
    }
    
    printf("Starting C web server on %s\n", addr);
    printf("WARNING: This is a basic, non-production server implementation.\n");
    
    printf("\n--- Test Cases (run in a separate terminal) ---\n");
    printf("1. Access /info without session (should fail): curl http://localhost:8001/info\n");
    printf("2. Log in as 'dave' to create session: curl -c cookie.txt http://localhost:8001/login_test/dave\n");
    printf("3. Access /info with session (should succeed): curl -b cookie.txt http://localhost:8001/info\n");
    printf("4. Log out to destroy session: curl -b cookie.txt http://localhost:8001/logout_test\n");
    printf("5. Access /info after logout (should fail): curl -b cookie.txt http://localhost:8001/info\n");

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
    return 0;
}
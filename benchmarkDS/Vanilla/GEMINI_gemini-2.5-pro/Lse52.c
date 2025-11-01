// To compile and run this code:
// 1. Download "mongoose.h" and "mongoose.c" from https://github.com/cesanta/mongoose
// 2. Place them in the same directory as this file.
// 3. Compile:
//    - On Linux/macOS: gcc Lse52.c mongoose.c -o server -lpthread
//    - On Windows (MinGW): gcc Lse52.c mongoose.c -o server.exe -lws2_32
// 4. Run: ./server (or server.exe on Windows)

#include "mongoose.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define MAX_SESSIONS 100

// Simple session structure
struct session {
  char session_id[64];
  char username[64];
  time_t expires;
};

// Global session store and user credentials
static struct session s_sessions[MAX_SESSIONS];
static const char *s_valid_username = "admin";
static const char *s_valid_password = "password";

// Function to find or create a session
static struct session *get_session(const struct mg_http_message *hm) {
  char session_id[64] = "";
  mg_http_get_cookie(hm, "session-id", session_id, sizeof(session_id));
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (s_sessions[i].session_id[0] != '\0' && strcmp(s_sessions[i].session_id, session_id) == 0) {
      if (s_sessions[i].expires > mg_time()) {
        s_sessions[i].expires = mg_time() + 3600; // Extend session
        return &s_sessions[i];
      } else {
        s_sessions[i].session_id[0] = '\0'; // Expire session
      }
    }
  }
  return NULL;
}

static void handle_index(struct mg_connection *c) {
    mg_http_reply(c, 200, "Content-Type: text/html\r\n", "<h1>Welcome!</h1><a href=\"/login\">Login</a>");
}

static void handle_login_get(struct mg_connection *c) {
    char *html = "<h1>Login</h1>"
                 "<form action=\"/do_login\" method=\"post\">"
                 "  <label for=\"username\">Username:</label><br>"
                 "  <input type=\"text\" id=\"username\" name=\"username\"><br>"
                 "  <label for=\"password\">Password:</label><br>"
                 "  <input type=\"password\" id=\"password\" name=\"password\"><br><br>"
                 "  <input type=\"submit\" value=\"Submit\">"
                 "</form>";
    mg_http_reply(c, 200, "Content-Type: text/html\r\n", "%s", html);
}

static void do_login(struct mg_connection *c, const struct mg_http_message *hm) {
    char username[64], password[64];
    mg_http_get_var(&hm->body, "username", username, sizeof(username));
    mg_http_get_var(&hm->body, "password", password, sizeof(password));

    if (strcmp(username, s_valid_username) == 0 && strcmp(password, s_valid_password) == 0) {
        // Find an empty session slot
        for (int i = 0; i < MAX_SESSIONS; i++) {
            if (s_sessions[i].session_id[0] == '\0') {
                uint64_t r = mg_random();
                snprintf(s_sessions[i].session_id, sizeof(s_sessions[i].session_id), "%llx", r);
                strncpy(s_sessions[i].username, username, sizeof(s_sessions[i].username) - 1);
                s_sessions[i].expires = mg_time() + 3600; // Expires in 1 hour
                mg_http_reply(c, 302, "Location: /secret\r\n"
                                     "Set-Cookie: session-id=%s; Path=/\r\n",
                                     "", s_sessions[i].session_id);
                return;
            }
        }
        mg_http_reply(c, 500, "", "Session store is full\n");
    } else {
        mg_http_reply(c, 302, "Location: /login\r\n", "");
    }
}

static void handle_secret(struct mg_connection *c, const struct mg_http_message *hm) {
    struct session *s = get_session(hm);
    if (s != NULL) {
        mg_http_reply(c, 200, "Content-Type: text/html\r\n",
                      "<h1>Secret settings</h1><p>Welcome, %s!</p>", s->username);
    } else {
        mg_http_reply(c, 302, "Location: /login\r\n", "");
    }
}

// Event handler function
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_http_match_uri(hm, "/")) {
        handle_index(c);
    } else if (mg_http_match_uri(hm, "/login")) {
        handle_login_get(c);
    } else if (mg_http_match_uri(hm, "/do_login")) {
        do_login(c, hm);
    } else if (mg_http_match_uri(hm, "/secret")) {
        handle_secret(c, hm);
    } else {
        mg_http_reply(c, 404, "", "Not Found\n");
    }
  }
  (void) fn_data;
}

static void print_test_instructions(const char* url) {
    printf("\n--- 5 TEST CASES ---\n");
    printf("To test, please perform the following actions in your web browser:\n");
    printf("1. Visit Index Page: Go to %s/\n", url);
    printf("   Expected: See 'Welcome!' and a login link.\n");
    printf("\n2. Access Secret Page (Not Logged In): Go to %s/secret\n", url);
    printf("   Expected: Redirected to the login page.\n");
    printf("\n3. Login with Incorrect Credentials: Go to %s/login and enter 'user'/'wrongpass'.\n", url);
    printf("   Expected: Redirected back to the login page.\n");
    printf("\n4. Login with Correct Credentials: On the login page, enter 'admin'/'password'.\n");
    printf("   Expected: Redirected to the secret page, see 'Secret settings'.\n");
    printf("\n5. Access Secret Page (Logged In): After logging in, refresh or revisit %s/secret\n", url);
    printf("   Expected: Still see 'Secret settings' page.\n");
}

int main(void) {
  struct mg_mgr mgr;
  const char *port = "8000";
  char url[100];
  snprintf(url, sizeof(url), "http://localhost:%s", port);

  mg_mgr_init(&mgr);
  printf("Starting web server on %s\n", url);
  mg_http_listen(&mgr, url, fn, NULL);

  print_test_instructions(url);

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  
  mg_mgr_free(&mgr);
  return 0;
}
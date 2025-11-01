/*
 * IMPORTANT: This is a single-file example using the Mongoose library.
 * To compile:
 * 1. Ensure you have OpenSSL development libraries installed (e.g., libssl-dev on Debian/Ubuntu).
 * 2. Compile with:
 *    gcc -std=c11 -o Lse52 Lse52.c -W -Wall -Wextra -lssl -lcrypto
 *
 * This example implements security features like password hashing, session management,
 * and HTML escaping from scratch for demonstration purposes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

// For SHA256 hashing - requires linking -lssl -lcrypto
#include <openssl/sha.h>
#include <openssl/rand.h>

// --- Start of embedded mongoose.h ---
#ifndef MONGOOSE_H
#define MONGOOSE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mg_connection;
struct mg_mgr;
union mg_addr;

typedef void (*mg_event_handler_t)(struct mg_connection *, int, void *, void *);

struct mg_mgr {
  void *user_data;
};

void mg_mgr_init(struct mg_mgr *);
void mg_mgr_free(struct mg_mgr *);
bool mg_mgr_poll(struct mg_mgr *, int);
struct mg_connection *mg_listen(struct mg_mgr *, const char *, mg_event_handler_t,
                                void *);
struct mg_connection *mg_connect(struct mg_mgr *, const char *,
                                 mg_event_handler_t, void *);
void mg_printf(struct mg_connection *, const char *fmt, ...);
void mg_http_reply(struct mg_connection *, int, const char *, const char *fmt,
                   ...);
void mg_http_serve_dir(struct mg_connection *, struct mg_http_message *,
                       const char *);

struct mg_str {
  const char *ptr;
  size_t len;
};

struct mg_http_message {
  struct mg_str method, uri, query, proto;
  struct mg_str body;
  struct mg_str head, chunk;
  struct mg_str *headers;
  char *c_headers[32];
};

struct mg_http_header {
  struct mg_str name;
  struct mg_str value;
};

int mg_http_parse(const char *, size_t, struct mg_http_message *);
struct mg_str *mg_http_get_header(struct mg_http_message *, const char *);

#define MG_EV_HTTP_MSG 100

#ifdef __cplusplus
}
#endif
#endif
// --- End of embedded mongoose.h ---

// --- Start of embedded mongoose.c ---
// NOTE: For brevity in this example, the full mongoose.c is not pasted.
// It is a very large file. The user must download it.
// This is a placeholder for the actual mongoose.c implementation.
// Please download from https://github.com/cesanta/mongoose and include it.
// For this code to compile, you must either paste the contents of mongoose.c
// here, or compile mongoose.c into an object file and link it.
// The provided compile command assumes a system-wide installation or linking.
// The code below WILL NOT COMPILE without the full mongoose implementation.
// Let's stub the necessary functions for the example to be self-contained.

#define MG_ENABLE_IPV6 1
#define MG_ENABLE_HTTP_WEBSOCKET 1
#define MG_ENABLE_HTTP_STREAMING_MULTIPART 1
#define MG_DEFAULT_HTTP_PORT "8000"
#include <mongoose.c>

// --- End of embedded mongoose.c ---

// --- App-specific Code ---
#define MAX_SESSIONS 1024
#define MAX_USERS 128
#define SALT_LEN 16
#define SESSION_ID_LEN 32

pthread_mutex_t mutex;

typedef struct {
    char username[64];
    unsigned char salt[SALT_LEN];
    unsigned char hashed_password[SHA256_DIGEST_LENGTH];
} User;

typedef struct {
    char session_id[SESSION_ID_LEN * 2 + 1]; // Hex string representation
    char username[64];
    time_t expires;
} Session;

// Global in-memory storage. Use a database in a real app.
User user_database[MAX_USERS];
int user_count = 0;
Session session_store[MAX_SESSIONS];
int session_count = 0;

// --- Security Helper Functions ---

void generate_random(unsigned char *buf, size_t len) {
    if (RAND_bytes(buf, len) != 1) {
        fprintf(stderr, "Fatal: Failed to generate random bytes.\n");
        exit(EXIT_FAILURE);
    }
}

void hash_password(const char *password, const unsigned char *salt, unsigned char *hash_out) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, SALT_LEN);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash_out, &sha256);
}

// Constant-time memory comparison to prevent timing attacks
int constant_time_compare(const void *a, const void *b, size_t size) {
    const unsigned char *p1 = (const unsigned char*)a;
    const unsigned char *p2 = (const unsigned char*)b;
    unsigned char result = 0;
    for (size_t i = 0; i < size; i++) {
        result |= p1[i] ^ p2[i];
    }
    return result == 0;
}

void bytes_to_hex(const unsigned char* bytes, size_t len, char* hex_str) {
    for (size_t i = 0; i < len; i++) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
}

// Securely checks credentials
int do_login_logic(const char *username, const char *password) {
    if (!username || !password || username[0] == '\0' || password[0] == '\0') {
        return 0;
    }

    pthread_mutex_lock(&mutex);
    User *user = NULL;
    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_database[i].username, username) == 0) {
            user = &user_database[i];
            break;
        }
    }

    if (user == NULL) {
        pthread_mutex_unlock(&mutex);
        // Prevent username enumeration by performing a dummy hash
        unsigned char dummy_salt[SALT_LEN];
        unsigned char dummy_hash[SHA256_DIGEST_LENGTH];
        generate_random(dummy_salt, SALT_LEN);
        hash_password(password, dummy_salt, dummy_hash);
        return 0;
    }

    unsigned char provided_hash[SHA256_DIGEST_LENGTH];
    hash_password(password, user->salt, provided_hash);
    int result = constant_time_compare(user->hashed_password, provided_hash, SHA256_DIGEST_LENGTH);
    pthread_mutex_unlock(&mutex);
    return result;
}

// Simple HTML escaping to prevent XSS
char* html_escape(const char* data) {
    size_t len = strlen(data);
    char* buffer = (char*)malloc(len * 6 + 1); // Worst case: all chars are '>' -> &gt;
    if (!buffer) return NULL;
    
    char* p = buffer;
    for(size_t i = 0; i < len; ++i) {
        switch(data[i]) {
            case '&':  p += sprintf(p, "&amp;");   break;
            case '\"': p += sprintf(p, "&quot;");  break;
            case '\'': p += sprintf(p, "&#39;");   break;
            case '<':  p += sprintf(p, "&lt;");    break;
            case '>':  p += sprintf(p, "&gt;");    break;
            default:   *p++ = data[i];             break;
        }
    }
    *p = '\0';
    return buffer;
}


// --- Mongoose Event Handler ---
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    if (mg_http_match_uri(hm, "/")) {
      mg_http_reply(c, 200, "Content-Type: text/html\r\n", 
                    "<html><body><h1>Welcome</h1><p><a href='/login'>Login</a></p></body></html>");
    } else if (mg_http_match_uri(hm, "/login")) {
      mg_http_reply(c, 200, "Content-Type: text/html\r\n", 
                    "<html><body><h2>Login</h2>"
                    "<form method='POST' action='/do_login'>"
                    "Username: <input type='text' name='username'><br>"
                    "Password: <input type='password' name='password'><br>"
                    "<input type='submit' value='Login'>"
                    "</form></body></html>");
    } else if (mg_http_match_uri(hm, "/do_login") && hm->method.len == 4 && memcmp(hm->method.ptr, "POST", 4) == 0) {
      char user[64], pass[64];
      mg_http_get_var(&hm->body, "username", user, sizeof(user));
      mg_http_get_var(&hm->body, "password", pass, sizeof(pass));

      if (do_login_logic(user, pass)) {
        pthread_mutex_lock(&mutex);
        if (session_count < MAX_SESSIONS) {
            unsigned char session_bytes[SESSION_ID_LEN];
            generate_random(session_bytes, SESSION_ID_LEN);
            Session* s = &session_store[session_count++];
            bytes_to_hex(session_bytes, SESSION_ID_LEN, s->session_id);
            strncpy(s->username, user, sizeof(s->username) - 1);
            s->expires = time(NULL) + 3600; // 1 hour expiration
            pthread_mutex_unlock(&mutex);

            mg_http_reply(c, 302, "Location: /secret\r\n"
                          "Set-Cookie: session_id=%s; Path=/; HttpOnly\r\n", s->session_id);
        } else {
             pthread_mutex_unlock(&mutex);
             mg_http_reply(c, 500, "", "Server busy, too many sessions.");
        }
      } else {
        mg_http_reply(c, 401, "Content-Type: text/html\r\n", 
                      "Invalid credentials. <a href='/login'>Try Again</a>.");
      }
    } else if (mg_http_match_uri(hm, "/secret")) {
        char session_id[SESSION_ID_LEN * 2 + 1] = {0};
        struct mg_str *cookie_hdr = mg_http_get_header(hm, "Cookie");
        if (cookie_hdr) {
            mg_http_get_var(cookie_hdr, "session_id", session_id, sizeof(session_id));
        }

        char *username = NULL;
        pthread_mutex_lock(&mutex);
        for(int i = 0; i < session_count; i++) {
            if (strcmp(session_store[i].session_id, session_id) == 0 && session_store[i].expires > time(NULL)) {
                username = session_store[i].username;
                break;
            }
        }
        pthread_mutex_unlock(&mutex);

        if (username) {
            char* escaped_user = html_escape(username);
            mg_http_reply(c, 200, "Content-Type: text/html\r\n", 
                          "<html><body><h1>Secret settings</h1><p>Welcome, %s!</p></body></html>", escaped_user);
            free(escaped_user);
        } else {
            mg_http_reply(c, 302, "Location: /login\r\n", "");
        }
    } else {
      mg_http_reply(c, 404, "", "Not Found");
    }
  }
  (void) fn_data;
}

void setup_user(const char* username, const char* password) {
    if (user_count >= MAX_USERS) return;
    pthread_mutex_lock(&mutex);
    User* u = &user_database[user_count];
    strncpy(u->username, username, sizeof(u->username) - 1);
    generate_random(u->salt, SALT_LEN);
    hash_password(password, u->salt, u->hashed_password);
    user_count++;
    pthread_mutex_unlock(&mutex);
}

int main(void) {
  pthread_mutex_init(&mutex, NULL);
  setup_user("admin", "password123");

  // --- 5 Test Cases for Login Logic ---
  printf("--- Running 5 Login Logic Test Cases ---\n");
  printf("1. Correct credentials ('admin', 'password123'): %s\n", do_login_logic("admin", "password123") ? "true" : "false");
  printf("2. Wrong password ('admin', 'wrongpass'): %s\n", do_login_logic("admin", "wrongpass") ? "true" : "false");
  printf("3. Wrong username ('user', 'password123'): %s\n", do_login_logic("user", "password123") ? "true" : "false");
  printf("4. Null username (NULL, 'password123'): %s\n", do_login_logic(NULL, "password123") ? "true" : "false");
  printf("5. Empty password ('admin', ''): %s\n", do_login_logic("admin", "") ? "true" : "false");
  printf("----------------------------------------\n\n");

  // --- Start Web Server ---
  struct mg_mgr mgr;
  const char *s_http_addr = "http://0.0.0.0:8000";
  mg_mgr_init(&mgr);
  mg_http_listen(&mgr, s_http_addr, fn, NULL);
  printf("Server starting on %s\n", s_http_addr);

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  
  mg_mgr_free(&mgr);
  pthread_mutex_destroy(&mutex);
  return 0;
}
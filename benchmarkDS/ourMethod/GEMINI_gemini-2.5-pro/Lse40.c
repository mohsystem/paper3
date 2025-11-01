/*
 * This code requires the libmicrohttpd library.
 *
 * Debian/Ubuntu: sudo apt-get install libmicrohttpd-dev
 * macOS (Homebrew): brew install libmicrohttpd
 *
 * Compilation command:
 * gcc -o server Lse40.c -lmicrohttpd
 *
 * To run:
 * ./server
 *
 * === TESTING ===
 * This example does not include an automated client. Use curl for testing.
 *
 * Test Case 1: Access /info without a session
 * $ curl -i http://localhost:8080/info
 * Expected: HTTP/1.1 401 Unauthorized, "Not Logged In"
 *
 * Test Case 2: Access /login to create a session and save the cookie
 * $ curl -i -c cookie.txt http://localhost:8080/login
 * Expected: HTTP/1.1 302 Found, Location: /info
 *
 * Test Case 3: Access /info using the saved cookie
 * $ curl -i -b cookie.txt http://localhost:8080/info
 * Expected: HTTP/1.1 200 OK, HTML with user info
 *
 * Test Case 4: Verify SSN is masked in the output of the previous command
 * Check the body for "****-**-7890" and NOT "123-456-7890"
 *
 * Test Case 5: Access /info after logging out
 * $ curl -i -b cookie.txt http://localhost:8080/logout
 * Expected: HTTP/1.1 200 OK, "Logged out"
 * $ curl -i -b cookie.txt http://localhost:8080/info
 * Expected: HTTP/1.1 401 Unauthorized (since cookie is now invalid)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <microhttpd.h>
#include <pthread.h>

#define PORT 8080
#define MAX_SESSIONS 100
#define SESSION_ID_LEN 32

typedef struct {
    char username[64];
    char email[128];
    char ssn[16];
} UserData;

typedef struct {
    char id[SESSION_ID_LEN + 1];
    UserData data;
} Session;

// In-memory session store (for demonstration)
static Session session_store[MAX_SESSIONS];
static int session_count = 0;
static pthread_mutex_t session_mutex = PTHREAD_MUTEX_INITIALIZER;

// Generates a cryptographically secure random session ID
void generate_session_id(char *buffer, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open /dev/urandom");
        exit(EXIT_FAILURE);
    }
    char random_data[len];
    if (read(fd, random_data, len) == -1) {
        perror("Failed to read from /dev/urandom");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
    for (size_t i = 0; i < len; ++i) {
        sprintf(&buffer[i * 2], "%02x", (unsigned char)random_data[i]);
    }
}

// Simple HTML escaping to prevent XSS. In a real app, use a robust library.
char* html_escape(const char* data) {
    if (!data) return NULL;
    size_t new_len = strlen(data) * 6 + 1; // Worst case: all chars are special
    char* buffer = (char*)malloc(new_len);
    if (!buffer) return NULL;
    char* p = buffer;
    for (size_t i = 0; data[i] != '\0'; ++i) {
        switch(data[i]) {
            case '&': p += sprintf(p, "&amp;"); break;
            case '\"': p += sprintf(p, "&quot;"); break;
            case '\'': p += sprintf(p, "&apos;"); break;
            case '<': p += sprintf(p, "&lt;"); break;
            case '>': p += sprintf(p, "&gt;"); break;
            default: *p++ = data[i]; break;
        }
    }
    *p = '\0';
    return buffer;
}


int request_handler(void *cls, struct MHD_Connection *connection,
                    const char *url, const char *method,
                    const char *version, const char *upload_data,
                    size_t *upload_data_size, void **con_cls) {

    struct MHD_Response *response;
    int ret;

    if (0 == strcmp(method, "GET")) {
        if (0 == strcmp(url, "/login")) {
            char session_id[SESSION_ID_LEN * 2 + 1];
            generate_session_id(session_id, SESSION_ID_LEN);

            pthread_mutex_lock(&session_mutex);
            if (session_count < MAX_SESSIONS) {
                strncpy(session_store[session_count].id, session_id, sizeof(session_store[session_count].id) -1);
                session_store[session_count].id[sizeof(session_store[session_count].id) - 1] = '\0';
                strcpy(session_store[session_count].data.username, "testuser");
                strcpy(session_store[session_count].data.email, "test@example.com");
                strcpy(session_store[session_count].data.ssn, "123-456-7890");
                session_count++;
            }
            pthread_mutex_unlock(&session_mutex);
            
            response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
            char cookie_buf[256];
            snprintf(cookie_buf, sizeof(cookie_buf), "session_id=%s; Path=/; HttpOnly", session_id);
            MHD_add_response_header(response, "Set-Cookie", cookie_buf);
            MHD_add_response_header(response, "Location", "/info");
            ret = MHD_queue_response(connection, 302, response);
            MHD_destroy_response(response);
            return ret;
        }
        else if (0 == strcmp(url, "/info")) {
            const char* cookie_str = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "session_id");
            Session* user_session = NULL;

            if (cookie_str) {
                pthread_mutex_lock(&session_mutex);
                for (int i = 0; i < session_count; ++i) {
                    if (strcmp(session_store[i].id, cookie_str) == 0) {
                        user_session = &session_store[i];
                        break;
                    }
                }
                pthread_mutex_unlock(&session_mutex);
            }

            if (user_session) {
                char ssn[16];
                strncpy(ssn, user_session->data.ssn, sizeof(ssn)-1);
                ssn[sizeof(ssn)-1] = '\0';
                
                char* last4 = strlen(ssn) >= 4 ? &ssn[strlen(ssn) - 4] : ssn;
                char *esc_user = html_escape(user_session->data.username);
                char *esc_email = html_escape(user_session->data.email);
                
                char body[1024];
                snprintf(body, sizeof(body),
                         "<html><body><h1>User Information</h1><p>Username: %s</p><p>Email: %s</p><p>SSN (last 4 digits): ****-**-%s</p></body></html>",
                         esc_user, esc_email, last4);
                
                free(esc_user);
                free(esc_email);

                response = MHD_create_response_from_buffer(strlen(body), (void *)body, MHD_RESPMEM_MUST_COPY);
                MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
                ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                MHD_destroy_response(response);
            } else {
                const char *body = "<html><body><h1>Not Logged In</h1><p>Please <a href=\"/login\">login</a> first.</p></body></html>";
                response = MHD_create_response_from_buffer(strlen(body), (void *)body, MHD_RESPMEM_PERSISTENT);
                MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
                ret = MHD_queue_response(connection, MHD_HTTP_UNAUTHORIZED, response);
                MHD_destroy_response(response);
            }
            return ret;
        }
        else if (0 == strcmp(url, "/logout")) {
             const char* cookie_str = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "session_id");
             if (cookie_str) {
                pthread_mutex_lock(&session_mutex);
                for (int i = 0; i < session_count; i++) {
                    if (strcmp(session_store[i].id, cookie_str) == 0) {
                        // Invalidate by shifting the last element over
                        session_store[i] = session_store[session_count - 1];
                        session_count--;
                        break;
                    }
                }
                pthread_mutex_unlock(&session_mutex);
             }
             const char* body = "Logged out.";
             response = MHD_create_response_from_buffer(strlen(body), (void*)body, MHD_RESPMEM_PERSISTENT);
             ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
             MHD_destroy_response(response);
             return ret;
        }
    }
    
    // Default 404 Not Found
    const char *page_404 = "<html><body>404 Not Found</body></html>";
    response = MHD_create_response_from_buffer(strlen(page_404), (void*)page_404, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

int main() {
    struct MHD_Daemon *daemon;
    
    // Using MHD_USE_THREAD_PER_CONNECTION for simplicity
    daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, PORT, NULL, NULL,
                              &request_handler, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Server running on port %d. Press Enter to exit.\n", PORT);
    (void)getchar();

    MHD_stop_daemon(daemon);
    return 0;
}
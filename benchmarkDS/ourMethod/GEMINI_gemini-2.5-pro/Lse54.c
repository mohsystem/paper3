#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// On Debian/Ubuntu: sudo apt-get install libmicrohttpd-dev
// On macOS (Homebrew): brew install libmicrohttpd
// Compile with: gcc -o server server.c -lmicrohttpd
#include <microhttpd.h>

#define PORT 8080

// In a real application, never hardcode credentials.
// CWE-798: Use of Hard-coded Credentials
// Use a secure identity provider and retrieve secrets from a vault or environment variables.
// This example uses plaintext comparison for simplicity, but production systems
// MUST use a strong, salted hashing algorithm like Argon2, scrypt, or PBKDF2.
// CWE-759: Use of a One-Way Hash without a Salt
const char *ADMIN_USERNAME = "admin";
const char *ADMIN_PASSWORD = "password";

// Represents the state of a POST request being processed
struct ConnectionInfo {
    char *username;
    char *password;
    struct MHD_PostProcessor *post_processor;
};

const char *login_page = "<html><body><h1>Login</h1><form action='/login' method='post'>"
                         "Username: <input type='text' name='username'><br>"
                         "Password: <input type='password' name='password'><br>"
                         "<input type='submit' value='Login'></form></body></html>";

const char *login_fail_page = "<html><body><h1>Login Failed</h1><p>Invalid username or password.</p>"
                              "<a href='/login'>Try again</a></body></html>";

const char *admin_page = "<html><body><h1>Welcome, Admin!</h1></body></html>";

// Frees the memory associated with a connection
static void request_completed(void *cls, struct MHD_Connection *connection,
                              void **con_cls, enum MHD_RequestTerminationCode toe) {
    struct ConnectionInfo *con_info = *con_cls;

    if (con_info == NULL) {
        return;
    }

    if (con_info->post_processor) {
        MHD_destroy_post_processor(con_info->post_processor);
        con_info->post_processor = NULL;
    }
    
    // Rule #10: Clear sensitive data from memory when no longer needed.
    // Use a volatile pointer and a loop to prevent compiler optimization.
    if (con_info->username) {
       volatile char *p = con_info->username;
       size_t len = strlen(p);
       for (size_t i = 0; i < len; i++) p[i] = '\0';
       free(con_info->username);
    }
    if (con_info->password) {
       volatile char *p = con_info->password;
       size_t len = strlen(p);
       for (size_t i = 0; i < len; i++) p[i] = '\0';
       free(con_info->password);
    }
    
    free(con_info);
    *con_cls = NULL;
}

// Iterator for post data
static int iterate_post(void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                        const char *filename, const char *content_type,
                        const char *transfer_encoding, const char *data, uint64_t off,
                        size_t size) {
    struct ConnectionInfo *con_info = coninfo_cls;

    if (size > 0 && size < 1024) { // Basic size validation
        if (strcmp(key, "username") == 0) {
            if (con_info->username) free(con_info->username);
            con_info->username = strndup(data, size);
            if (con_info->username == NULL) return MHD_NO;
        } else if (strcmp(key, "password") == 0) {
            if (con_info->password) free(con_info->password);
            con_info->password = strndup(data, size);
            if (con_info->password == NULL) return MHD_NO;
        }
    }
    return MHD_YES;
}

// Main handler for all incoming requests
static int answer_to_connection(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls) {
    
    if (strcmp(url, "/login") == 0 && strcmp(method, "GET") == 0) {
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(login_page), (void *)login_page, MHD_RESPMEM_PERSISTENT);
        if (!response) return MHD_NO;
        MHD_add_response_header(response, "Content-Type", "text/html; charset=UTF-8");
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    if (strcmp(url, "/admin") == 0 && strcmp(method, "GET") == 0) {
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(admin_page), (void *)admin_page, MHD_RESPMEM_PERSISTENT);
        if (!response) return MHD_NO;
        MHD_add_response_header(response, "Content-Type", "text/html; charset=UTF-8");
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    if (strcmp(url, "/login") == 0 && strcmp(method, "POST") == 0) {
        struct ConnectionInfo *con_info = *con_cls;

        if (con_info == NULL) {
            con_info = calloc(1, sizeof(struct ConnectionInfo));
            if (con_info == NULL) return MHD_NO;
            con_info->post_processor = MHD_create_post_processor(connection, 1024, iterate_post, (void *)con_info);
            if (con_info->post_processor == NULL) {
                free(con_info);
                return MHD_NO;
            }
            *con_cls = (void *)con_info;
            return MHD_YES;
        }

        if (*upload_data_size != 0) {
            MHD_post_process(con_info->post_processor, upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        } else {
            // Rule #1: Validate input
            if (con_info->username && con_info->password) {
                // NOTE: In a real app, use a constant-time comparison for security.
                if (strcmp(con_info->username, ADMIN_USERNAME) == 0 &&
                    strcmp(con_info->password, ADMIN_PASSWORD) == 0) {
                    struct MHD_Response *response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
                    if (!response) return MHD_NO;
                    MHD_add_response_header(response, "Location", "/admin");
                    int ret = MHD_queue_response(connection, MHD_HTTP_FOUND, response);
                    MHD_destroy_response(response);
                    return ret;
                }
            }
            struct MHD_Response *response = MHD_create_response_from_buffer(
                strlen(login_fail_page), (void *)login_fail_page, MHD_RESPMEM_PERSISTENT);
            if (!response) return MHD_NO;
            MHD_add_response_header(response, "Content-Type", "text/html; charset=UTF-8");
            int ret = MHD_queue_response(connection, MHD_HTTP_UNAUTHORIZED, response);
            MHD_destroy_response(response);
            return ret;
        }
    }
    
    const char *not_found_page = "<html><body>404 Not Found</body></html>";
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(not_found_page), (void *)not_found_page, MHD_RESPMEM_PERSISTENT);
    if (!response) return MHD_NO;
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

int main(void) {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL,
                              MHD_OPTION_NOTIFY_COMPLETED, &request_completed, NULL,
                              MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }
    
    printf("Server started on http://localhost:%d\n", PORT);
    printf("--- Test Cases ---\n");
    printf("1. View login form: curl http://localhost:%d/login\n", PORT);
    printf("2. Successful login (will get a 302 redirect): curl -i -L -d \"username=admin&password=password\" http://localhost:%d/login\n", PORT);
    printf("3. Failed login (wrong password): curl -i -d \"username=admin&password=wrong\" http://localhost:%d/login\n", PORT);
    printf("4. Failed login (wrong username): curl -i -d \"username=user&password=password\" http://localhost:%d/login\n", PORT);
    printf("5. Access admin page after successful login: Follow the redirect from test case #2.\n");
    
    printf("Press Enter to exit.\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    return 0;
}
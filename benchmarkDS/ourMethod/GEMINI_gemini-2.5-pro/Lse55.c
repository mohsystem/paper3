/*
This code requires libmicrohttpd.
On Debian/Ubuntu: sudo apt-get install libmicrohttpd-dev
On Fedora/CentOS: sudo yum install libmicrohttpd-devel
On macOS (Homebrew): brew install libmicrohttpd

To compile:
gcc -o server server.c -lmicrohttpd

This C example is more complex than others due to the low-level nature of the language for web servers.
Memory management must be handled carefully.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <microhttpd.h>

#define PORT 8080
#define MAX_POST_BUFFER_SIZE 1024

// Structure to hold connection-specific data, especially for POST requests.
struct connection_info_struct {
    char *username;
    char *password;
    struct MHD_PostProcessor *post_processor;
};

// --- Page Content ---
const char *INDEX_PAGE = "<h1>Welcome to the Index Page!</h1>";
const char *LOGIN_PAGE = "<h1>Login Page</h1>"
                         "<form action='/do_login' method='post'>"
                         "Username: <input type='text' name='username'><br>"
                         "Password: <input type='password' name='password'><br>"
                         "<input type='submit' value='Login'>"
                         "</form>";
const char *USER_PAGE = "<h1>Welcome to the User Page!</h1>";
const char *ADMIN_PAGE = "<h1>Welcome to the Admin Page!</h1>";
const char *NOT_FOUND_PAGE = "<h1>404 Not Found</h1>";
const char *BAD_REQUEST_PAGE = "<h1>400 Bad Request</h1>";
const char *LOGIN_SUCCESS_ADMIN = "<h1>Admin Login Successful!</h1>";
const char *LOGIN_SUCCESS_USER = "<h1>User Login Successful!</h1>";
const char *LOGIN_FAILURE = "<h1>Login Failed: Invalid credentials.</h1>";

// Helper to send a response
static int send_page(struct MHD_Connection *connection, const char *page, int status_code) {
    struct MHD_Response *response;
    int ret;
    response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    if (!response) {
        return MHD_NO;
    }
    MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret;
}

// Iterator for processing POST data
static int iterate_post(void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                        const char *filename, const char *content_type,
                        const char *transfer_encoding, const char *data, uint64_t off, size_t size) {
    struct connection_info_struct *con_info = coninfo_cls;
    
    // Rules#8: check buffer boundaries
    if (size == 0 || size > MAX_POST_BUFFER_SIZE) {
        return MHD_NO; // Stop processing if data is empty or too large
    }

    if (strcmp(key, "username") == 0) {
        // Only accept the first value, free if it already exists
        if(con_info->username) free(con_info->username);
        con_info->username = strndup(data, size);
        if (!con_info->username) return MHD_NO;
    } else if (strcmp(key, "password") == 0) {
        if(con_info->password) free(con_info->password);
        con_info->password = strndup(data, size);
        if (!con_info->password) return MHD_NO;
    }
    return MHD_YES;
}

// Callback to free connection-specific data
static void request_completed(void *cls, struct MHD_Connection *connection,
                              void **con_cls, enum MHD_RequestTerminationCode toe) {
    struct connection_info_struct *con_info = *con_cls;

    if (con_info == NULL) {
        return;
    }
    if (con_info->post_processor) {
        MHD_destroy_post_processor(con_info->post_processor);
    }
    if (con_info->username) {
        free(con_info->username);
    }
    if (con_info->password) {
        free(con_info->password);
    }
    free(con_info);
    *con_cls = NULL;
}


// Main request handler
static int answer_to_connection(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls) {

    // First time this connection is handled
    if (*con_cls == NULL) {
        struct connection_info_struct *con_info;
        con_info = calloc(1, sizeof(struct connection_info_struct));
        if (con_info == NULL) {
            return MHD_NO;
        }
        *con_cls = (void *)con_info;
        return MHD_YES;
    }

    // Handle GET requests
    if (strcmp(method, "GET") == 0) {
        if (strcmp(url, "/") == 0 || strcmp(url, "/index") == 0) return send_page(connection, INDEX_PAGE, MHD_HTTP_OK);
        if (strcmp(url, "/login") == 0) return send_page(connection, LOGIN_PAGE, MHD_HTTP_OK);
        if (strcmp(url, "/user") == 0) return send_page(connection, USER_PAGE, MHD_HTTP_OK);
        if (strcmp(url, "/admin") == 0) return send_page(connection, ADMIN_PAGE, MHD_HTTP_OK);
        return send_page(connection, NOT_FOUND_PAGE, MHD_HTTP_NOT_FOUND);
    }

    // Handle POST requests
    if (strcmp(method, "POST") == 0 && strcmp(url, "/do_login") == 0) {
        struct connection_info_struct *con_info = *con_cls;

        // Process POST data
        if (*upload_data_size != 0) {
            if (con_info->post_processor == NULL) {
                con_info->post_processor = MHD_create_post_processor(connection, 1024, iterate_post, (void *)con_info);
                if (con_info->post_processor == NULL) {
                    return MHD_NO;
                }
            }
            MHD_post_process(con_info->post_processor, upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        } else {
            // End of POST data, perform login check
            // Rules#6: Input validation
            if (con_info->username == NULL || con_info->password == NULL || 
                strlen(con_info->username) == 0 || strlen(con_info->password) == 0) {
                return send_page(connection, BAD_REQUEST_PAGE, MHD_HTTP_BAD_REQUEST);
            }
            
            // Rules#1, #3, #4, #5: NEVER store or compare passwords in plaintext.
            // Use a strong, salted hashing library like libsodium for Argon2.
            // The following is for demonstration purposes ONLY and is highly insecure.
            if (strcmp(con_info->username, "admin") == 0 && strcmp(con_info->password, "password123") == 0) {
                return send_page(connection, LOGIN_SUCCESS_ADMIN, MHD_HTTP_OK);
            } else if (strcmp(con_info->username, "user") == 0 && strcmp(con_info->password, "password123") == 0) {
                return send_page(connection, LOGIN_SUCCESS_USER, MHD_HTTP_OK);
            } else {
                return send_page(connection, LOGIN_FAILURE, MHD_HTTP_UNAUTHORIZED);
            }
        }
    }

    return send_page(connection, NOT_FOUND_PAGE, MHD_HTTP_NOT_FOUND);
}

int main() {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL,
                              MHD_OPTION_NOTIFY_COMPLETED, &request_completed, NULL,
                              MHD_OPTION_END);
    if (daemon == NULL) {
        fprintf(stderr, "Error starting daemon on port %d\n", PORT);
        return 1;
    }

    printf("Server running on port %d. Press Enter to exit.\n\n", PORT);
    
    printf("--- 5 Test Cases (run these commands in another terminal) ---\n");
    printf("1. Test index function: curl http://localhost:%d/index\n", PORT);
    printf("2. Test login function: curl http://localhost:%d/login\n", PORT);
    printf("3. Test do_login function: curl -X POST -d 'username=admin&password=password123' http://localhost:%d/do_login\n", PORT);
    printf("4. Test user_page function: curl http://localhost:%d/user\n", PORT);
    printf("5. Test admin_page function: curl http://localhost:%d/admin\n", PORT);
    printf("----------------------------------------------------------\n");

    (void)getchar();

    MHD_stop_daemon(daemon);
    printf("Server stopped.\n");
    return 0;
}
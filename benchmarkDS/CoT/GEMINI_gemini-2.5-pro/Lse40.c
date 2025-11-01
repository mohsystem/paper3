/*
================================================================================
IMPORTANT NOTE:
C does not have a standard library for creating web applications. This task
is not feasible in a single, dependency-free C file.

This code is a conceptual example using the 'libmicrohttpd' library.
It WILL NOT COMPILE as a standalone file. You must:
1. Install libmicrohttpd (e.g., `sudo apt-get install libmicrohttpd-dev` on Debian/Ubuntu).
2. Compile and link against the library, for example:
   gcc -o server Lse40.c -lmicrohttpd

This code demonstrates the principles but is far more complex than in other languages.
Session management is simulated with a simple, non-scalable global array.
================================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <microhttpd.h>

#define PORT 8082
#define MAX_SESSIONS 100
#define SESSION_ID_SIZE 16

// --- Data Structures for Session Simulation ---
typedef struct {
    char username[256];
    char email[256];
    char ssn[16];
} UserData;

typedef struct {
    char id[SESSION_ID_SIZE + 1];
    UserData data;
} Session;

// Global session store (not thread-safe without locks, which are omitted for simplicity)
static Session sessions[MAX_SESSIONS];
static int session_count = 0;

// --- Utility Functions ---

// Simple HTML escaper
char* escape_html(const char* input) {
    if (!input) return strdup("");
    size_t len = strlen(input);
    char* escaped = malloc(len * 6 + 1); // Worst case: all chars are special
    if (!escaped) return NULL;
    
    char* p = escaped;
    for (size_t i = 0; i < len; ++i) {
        switch (input[i]) {
            case '&': p += sprintf(p, "&amp;"); break;
            case '<': p += sprintf(p, "&lt;"); break;
            case '>': p += sprintf(p, "&gt;"); break;
            case '"': p += sprintf(p, "&quot;"); break;
            case '\'': p += sprintf(p, "&#x27;"); break;
            default: *p++ = input[i]; break;
        }
    }
    *p = '\0';
    return escaped;
}


// The main logic function for the /info route
char* info(const UserData* data) {
    // SECURITY: Never store full SSN in session in a real application.
    char last4_ssn[5] = {0};
    size_t ssn_len = strlen(data->ssn);
    if (ssn_len >= 4) {
        strncpy(last4_ssn, data->ssn + ssn_len - 4, 4);
    } else {
        strncpy(last4_ssn, data->ssn, 4);
    }
    
    // SECURITY: Escape user data for HTML output
    char* escaped_user = escape_html(data->username);
    char* escaped_email = escape_html(data->email);

    // Create the HTML response
    char* response_html = malloc(2048);
    if (response_html) {
        sprintf(response_html,
            "<!DOCTYPE html><html><head><title>User Info</title></head>"
            "<body><h1>User Information</h1>"
            "<p><strong>Username:</strong> %s</p>"
            "<p><strong>Email:</strong> %s</p>"
            "<p><strong>Last 4 Digits of SSN:</strong> %s</p>"
            "<a href=\"/test\">Back to Test Cases</a></body></html>",
            escaped_user, escaped_email, last4_ssn);
    }
    
    free(escaped_user);
    free(escaped_email);
    return response_html;
}


// --- Main Request Handler ---
int answer_to_connection(void *cls, struct MHD_Connection *connection,
                         const char *url, const char *method,
                         const char *version, const char *upload_data,
                         size_t *upload_data_size, void **con_cls) {
    
    struct MHD_Response *response;
    int ret;

    if (0 != strcmp(method, "GET")) {
        return MHD_NO; // Only accept GET
    }

    if (strcmp(url, "/info") == 0) {
        const char *cookie = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "sessionId");
        char* page = NULL;
        int status_code = MHD_HTTP_UNAUTHORIZED;

        if (cookie) {
            for (int i = 0; i < session_count; ++i) {
                if (strcmp(sessions[i].id, cookie) == 0) {
                    page = info(&sessions[i].data);
                    status_code = MHD_HTTP_OK;
                    break;
                }
            }
        }
        if (!page) {
             page = strdup("<h1>Unauthorized</h1><p>Please <a href='/test'>login</a> first.</p>");
        }
        
        response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_MUST_FREE);
        MHD_add_response_header(response, "Content-Type", "text/html");
        ret = MHD_queue_response(connection, status_code, response);
        MHD_destroy_response(response);
    } else if (strncmp(url, "/login", 6) == 0) {
        if (session_count < MAX_SESSIONS) {
            const char* user = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "user");
            const char* email = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "email");
            const char* ssn = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "ssn");

            if (user && email && ssn) {
                Session* s = &sessions[session_count];
                snprintf(s->id, SESSION_ID_SIZE, "%ld", time(NULL) + rand());
                strncpy(s->data.username, user, sizeof(s->data.username) - 1);
                strncpy(s->data.email, email, sizeof(s->data.email) - 1);
                strncpy(s->data.ssn, ssn, sizeof(s->data.ssn) - 1);
                session_count++;

                char cookie_val[256];
                snprintf(cookie_val, sizeof(cookie_val), "sessionId=%s; Path=/", s->id);
                
                response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
                MHD_add_response_header(response, "Location", "/info");
                MHD_add_response_header(response, "Set-Cookie", cookie_val);
                ret = MHD_queue_response(connection, MHD_HTTP_FOUND, response);
                MHD_destroy_response(response);
            } else {
                 const char* msg = "Bad request";
                 response = MHD_create_response_from_buffer(strlen(msg), (void*)msg, MHD_RESPMEM_PERSISTENT);
                 ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
                 MHD_destroy_response(response);
            }
        }
    } else if (strcmp(url, "/test") == 0 || strcmp(url, "/") == 0) {
        const char *page =
            "<!DOCTYPE html><html><head><title>Test Cases</title></head><body>"
            "<h1>Click a user to log in</h1><ul>"
            "<li><a href='/login?user=alice&email=alice@example.com&ssn=111223344'>Login as Alice</a></li>"
            "<li><a href='/login?user=bob&email=bob@example.com&ssn=555667788'>Login as Bob</a></li>"
            "<li><a href='/login?user=charlie<script>alert(1)</script>&email=charlie@bad.com&ssn=999887766'>Login as Charlie (XSS Test)</a></li>"
            "<li><a href='/login?user=dave&email=dave@example.com&ssn=123456789'>Login as Dave</a></li>"
            "<li><a href='/login?user=eve&email=eve@example.com&ssn=000000001'>Login as Eve</a></li>"
            "</ul></body></html>";
        response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/html");
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
    } else {
        const char *page = "Not Found";
        response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
    }

    return ret;
}


int main(void) {
    struct MHD_Daemon *daemon;
    
    srand(time(NULL));

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server.\n");
        return 1;
    }

    printf("Server started on port %d\n", PORT);
    printf("--- Test Cases ---\n");
    printf("Visit http://localhost:%d/test to run test cases.\n", PORT);
    printf("Press Enter to exit.\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    return 0;
}
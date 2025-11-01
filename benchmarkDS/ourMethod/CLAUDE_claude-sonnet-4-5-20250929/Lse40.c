
#include <microhttpd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#define MAX_SESSIONS 100
#define SESSION_TIMEOUT 1800  /* 30 minutes in seconds */
#define MAX_SESSION_ID_LEN 65
#define MAX_USERNAME_LEN 256
#define MAX_EMAIL_LEN 256
#define MAX_SSN_LEN 16

/* Session data structure with security features */
typedef struct {
    char session_id[MAX_SESSION_ID_LEN];
    char username[MAX_USERNAME_LEN];
    char email[MAX_EMAIL_LEN];
    char ssn[MAX_SSN_LEN];  /* In production, encrypt this */
    time_t last_access;
    bool active;
} SessionData;

/* Global session storage */
static SessionData sessions[MAX_SESSIONS];
static int session_count = 0;

/* Sanitize HTML to prevent XSS - returns newly allocated string that must be freed */
static char* sanitize_html(const char* input) {
    if (!input) return NULL;
    
    size_t len = strlen(input);
    /* Worst case: every char becomes 6 chars (&#x27;) */
    char* output = calloc(len * 6 + 1, 1);
    if (!output) return NULL;
    
    size_t out_pos = 0;
    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
            case '&':
                strcpy(output + out_pos, "&amp;");
                out_pos += 5;
                break;
            case '<':
                strcpy(output + out_pos, "&lt;");
                out_pos += 4;
                break;
            case '>':
                strcpy(output + out_pos, "&gt;");
                out_pos += 4;
                break;
            case '"':
                strcpy(output + out_pos, "&quot;");
                out_pos += 6;
                break;
            case '\\'':
                strcpy(output + out_pos, "&#x27;");
                out_pos += 6;
                break;
            case '/':
                strcpy(output + out_pos, "&#x2F;");
                out_pos += 6;
                break;
            default:
                output[out_pos++] = input[i];
                break;
        }
    }
    output[out_pos] = '\\0';
    return output;
}

/* Validate SSN format */
static bool is_valid_ssn(const char* ssn) {
    if (!ssn) return false;
    
    int digit_count = 0;
    for (size_t i = 0; ssn[i] != '\\0'; i++) {
        if (ssn[i] >= '0' && ssn[i] <= '9') {
            digit_count++;
        } else if (ssn[i] != '-') {
            return false;
        }
    }
    
    return digit_count == 9;
}

/* Extract last 4 digits securely */
static void get_last_four_digits(const char* ssn, char* output, size_t output_size) {
    if (!ssn || !output || output_size < 5) {
        if (output && output_size > 0) {
            strncpy(output, "****", output_size - 1);
            output[output_size - 1] = '\\0';
        }
        return;
    }
    
    size_t len = strlen(ssn);
    if (len < 4) {
        strncpy(output, "****", output_size - 1);
        output[output_size - 1] = '\\0';
        return;
    }
    
    /* Copy last 4 characters */
    strncpy(output, ssn + len - 4, output_size - 1);
    output[output_size - 1] = '\\0';
}

/* Generate simple session ID (in production use cryptographically secure random) */
static void generate_session_id(char* output, size_t size) {
    if (!output || size < MAX_SESSION_ID_LEN) return;
    
    const char* hex_chars = "0123456789abcdef";
    for (size_t i = 0; i < MAX_SESSION_ID_LEN - 1; i++) {
        output[i] = hex_chars[rand() % 16];
    }
    output[MAX_SESSION_ID_LEN - 1] = '\\0';
}

/* Find session by ID */
static SessionData* find_session(const char* session_id) {
    if (!session_id) return NULL;
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active && strcmp(sessions[i].session_id, session_id) == 0) {
            return &sessions[i];
        }
    }
    return NULL;
}

/* Check if session is expired */
static bool is_session_expired(const SessionData* session) {
    if (!session) return true;
    return (time(NULL) - session->last_access) > SESSION_TIMEOUT;
}

/* Extract session ID from cookie header */
static char* get_session_id_from_cookie(const char* cookie_header) {
    if (!cookie_header) return NULL;
    
    const char* prefix = "sessionId=";
    const char* start = strstr(cookie_header, prefix);
    if (!start) return NULL;
    
    start += strlen(prefix);
    char* session_id = malloc(MAX_SESSION_ID_LEN);
    if (!session_id) return NULL;
    
    strncpy(session_id, start, MAX_SESSION_ID_LEN - 1);
    session_id[MAX_SESSION_ID_LEN - 1] = '\\0';
    
    return session_id;
}

/* HTTP request handler */
static enum MHD_Result handle_request(void* cls,
                                     struct MHD_Connection* connection,
                                     const char* url,
                                     const char* method,
                                     const char* version,
                                     const char* upload_data,
                                     size_t* upload_data_size,
                                     void** con_cls) {
    struct MHD_Response* response = NULL;
    enum MHD_Result ret;
    
    /* Only accept GET to /info */
    if (strcmp(method, "GET") != 0 || strcmp(url, "/info") != 0) {
        const char* error_msg = "Method Not Allowed or Invalid Path";
        response = MHD_create_response_from_buffer(strlen(error_msg),
                                                   (void*)error_msg,
                                                   MHD_RESPMEM_PERSISTENT);
        if (!response) return MHD_NO;
        MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");
        ret = MHD_queue_response(connection, 405, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    /* Get cookie header */
    const char* cookie_header = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Cookie");
    char* session_id = get_session_id_from_cookie(cookie_header);
    
    if (!session_id) {
        const char* error_msg = "Unauthorized: No valid session found";
        response = MHD_create_response_from_buffer(strlen(error_msg),
                                                   (void*)error_msg,
                                                   MHD_RESPMEM_PERSISTENT);
        if (!response) return MHD_NO;
        MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");
        ret = MHD_queue_response(connection, 401, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    /* Find session */
    SessionData* session = find_session(session_id);
    free(session_id);
    
    if (!session || is_session_expired(session)) {
        if (session) session->active = false;
        const char* error_msg = "Unauthorized: Session expired or invalid";
        response = MHD_create_response_from_buffer(strlen(error_msg),
                                                   (void*)error_msg,
                                                   MHD_RESPMEM_PERSISTENT);
        if (!response) return MHD_NO;
        MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");
        ret = MHD_queue_response(connection, 401, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    /* Update access time */
    session->last_access = time(NULL);
    
    /* Sanitize outputs */
    char* safe_username = sanitize_html(session->username);
    char* safe_email = sanitize_html(session->email);
    char last_four[5];
    get_last_four_digits(session->ssn, last_four, sizeof(last_four));
    
    if (!safe_username || !safe_email) {
        free(safe_username);
        free(safe_email);
        return MHD_NO;
    }
    
    /* Build HTML response */
    char* html = malloc(4096);
    if (!html) {
        free(safe_username);
        free(safe_email);
        return MHD_NO;
    }
    
    snprintf(html, 4096,
             "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">"
             "<title>User Info</title></head><body>"
             "<h1>User Information</h1>"
             "<p>Username: %s</p>"
             "<p>Email: %s</p>"
             "<p>SSN (last 4): ***-**-%s</p>"
             "</body></html>",
             safe_username, safe_email, last_four);
    
    free(safe_username);
    free(safe_email);
    
    response = MHD_create_response_from_buffer(strlen(html), html, MHD_RESPMEM_MUST_FREE);
    if (!response) {
        free(html);
        return MHD_NO;
    }
    
    /* Add security headers */
    MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
    MHD_add_response_header(response, "X-Content-Type-Options", "nosniff");
    MHD_add_response_header(response, "X-Frame-Options", "DENY");
    MHD_add_response_header(response, "Content-Security-Policy", "default-src 'self'");
    
    ret = MHD_queue_response(connection, 200, response);
    MHD_destroy_response(response);
    return ret;
}

/* Create test session */
static char* create_session(const char* username, const char* email, const char* ssn) {
    if (!is_valid_ssn(ssn) || session_count >= MAX_SESSIONS) {
        return NULL;
    }
    
    SessionData* session = &sessions[session_count++];
    generate_session_id(session->session_id, sizeof(session->session_id));
    strncpy(session->username, username, MAX_USERNAME_LEN - 1);
    session->username[MAX_USERNAME_LEN - 1] = '\\0';
    strncpy(session->email, email, MAX_EMAIL_LEN - 1);
    session->email[MAX_EMAIL_LEN - 1] = '\\0';
    strncpy(session->ssn, ssn, MAX_SSN_LEN - 1);
    session->ssn[MAX_SSN_LEN - 1] = '\\0';
    session->last_access = time(NULL);
    session->active = true;
    
    char* result = malloc(MAX_SESSION_ID_LEN);
    if (result) {
        strcpy(result, session->session_id);
    }
    return result;
}

int main(void) {
    srand((unsigned int)time(NULL));
    
    /* Initialize sessions */
    memset(sessions, 0, sizeof(sessions));
    
    /* Create 5 test sessions */
    char* s1 = create_session("john_doe", "john@example.com", "123-45-6789");
    char* s2 = create_session("jane_smith", "jane@example.com", "987-65-4321");
    char* s3 = create_session("bob_jones", "bob@example.com", "555-12-3456");
    char* s4 = create_session("alice_williams", "alice@example.com", "111-22-3333");
    char* s5 = create_session("charlie_brown", "charlie@example.com", "999-88-7777");
    
    printf("Test session IDs created:\\n");
    if (s1) { printf("Session 1: %s\\n", s1); free(s1); }
    if (s2) { printf("Session 2: %s\\n", s2); free(s2); }
    if (s3) { printf("Session 3: %s\\n", s3); free(s3); }
    if (s4) { printf("Session 4: %s\\n", s4); free(s4); }
    if (s5) { printf("Session 5: %s\\n", s5); free(s5); }
    
    /* Start HTTP server */
    struct MHD_Daemon* daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                                                 8000,
                                                 NULL, NULL,
                                                 &handle_request, NULL,
                                                 MHD_OPTION_END);
    
    if (!daemon) {
        fprintf(stderr, "Failed to start server\\n");
        return 1;
    }
    
    printf("\\nServer started on http://localhost:8000\\n");
    printf("Access /info endpoint with Cookie header: sessionId=<session_id>\\n");
    printf("Press Enter to stop\\n");
    
    getchar();
    
    MHD_stop_daemon(daemon);
    
    return 0;
}

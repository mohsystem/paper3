// Note: This code requires the libmicrohttpd library and a POSIX-compliant system for regex.
// On Debian/Ubuntu: sudo apt-get install libmicrohttpd-dev
// On Fedora/CentOS: sudo dnf install libmicrohttpd-devel
//
// Compile with: gcc -o server Lse29.c -lmicrohttpd

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <microhttpd.h>

#define PORT 8082

// Callback function to get a specific query parameter's value
int get_param_iterator(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
    // cls is a pointer to a char pointer where we store the result
    if (strcmp(key, "target") == 0) {
        *(const char**)cls = value;
        return MHD_NO; // Stop iterating, we found what we wanted
    }
    return MHD_YES; // Continue iteration
}

// Main request handler
int request_handler(void *cls, struct MHD_Connection *connection,
                    const char *url, const char *method,
                    const char *version, const char *upload_data,
                    size_t *upload_data_size, void **con_cls) {
    
    struct MHD_Response *response;
    int ret;

    // Handle root URL ("/")
    if (0 == strcmp(url, "/")) {
        const char *page = "Welcome! This is the default page.";
        response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Handle redirect URL ("/redirect" or "/redirect?...")
    if (0 == strncmp(url, "/redirect", 9) && (url[9] == '?' || url[9] == '\0')) {
        const char *target = NULL;
        MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, &get_param_iterator, (void*)&target);

        int should_redirect_to_target = 0;
        if (target != NULL) {
            regex_t regex;
            if (regcomp(&regex, ".*example\\.com.*", REG_EXTENDED | REG_NOSUB) == 0) {
                if (regexec(&regex, target, 0, NULL, 0) == 0) {
                    should_redirect_to_target = 1; // Match found
                }
                regfree(&regex);
            }
        }
        
        response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
        const char* location = should_redirect_to_target ? target : "/";
        MHD_add_response_header(response, MHD_HTTP_HEADER_LOCATION, location);
        
        ret = MHD_queue_response(connection, MHD_HTTP_FOUND, response); // 302 Found
        MHD_destroy_response(response);
        return ret;
    }
    
    // Default: 404 Not Found for any other URL
    const char *page_404 = "404 Not Found";
    response = MHD_create_response_from_buffer(strlen(page_404), (void*)page_404, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

void print_test_cases(int port) {
    char base_url[100];
    snprintf(base_url, sizeof(base_url), "http://localhost:%d", port);
    printf("--- 5 Test Cases ---\n");
    printf("Execute these commands in a separate terminal:\n");
    printf("1. Valid Target: curl -I \"%s/redirect?target=http://www.example.com\"\n", base_url);
    printf("   Expected: 302 Redirect to http://www.example.com\n");
    printf("2. Valid Subdomain: curl -I \"%s/redirect?target=https://blog.example.com/article/1\"\n", base_url);
    printf("   Expected: 302 Redirect to https://blog.example.com/article/1\n");
    printf("3. Invalid Target: curl -I \"%s/redirect?target=http://www.google.com\"\n", base_url);
    printf("   Expected: 302 Redirect to /\n");
    printf("4. Invalid TLD: curl -I \"%s/redirect?target=http://example.community.net\"\n", base_url);
    printf("   Expected: 302 Redirect to /\n");
    printf("5. Missing Target: curl -I \"%s/redirect\"\n", base_url);
    printf("   Expected: 302 Redirect to /\n");
}

int main() {
    // The concept of a main class 'Lse29' is not applicable in C.
    // This main function serves as the entry point for the 'Lse29' program.
    struct MHD_Daemon *daemon;
    
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &request_handler, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server on port %d\n", PORT);
        return 1;
    }
    
    printf("C server started on http://localhost:%d\n", PORT);
    print_test_cases(PORT);
    
    printf("Press Enter to stop the server\n");
    (void)getchar();
    
    MHD_stop_daemon(daemon);
    return 0;
}
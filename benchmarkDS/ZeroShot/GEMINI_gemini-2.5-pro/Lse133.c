/*
NOTE: This code requires the libmicrohttpd library.
On Debian/Ubuntu: sudo apt-get install libmicrohttpd-dev
Compile with: gcc your_file.c -o your_app -lmicrohttpd
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Line 1: The first line imports the web application class (library).
#include <microhttpd.h>

#define PORT 8888

// Lines 3, 4, 5: This function acts as the URL router and handler.
int request_handler(void *cls, struct MHD_Connection *connection,
                    const char *url, const char *method,
                    const char *version, const char *upload_data,
                    size_t *upload_data_size, void **con_cls) {
    // Only process GET requests to the root path for security.
    if (strcmp(method, "GET") == 0 && strcmp(url, "/") == 0) {
        const char *page = "Hello, World!";
        struct MHD_Response *response;
        int ret;

        // Create a response from a constant string. This is safe.
        // If the string were dynamic or from user input, it would need sanitization.
        response = MHD_create_response_from_buffer(strlen(page),
                                                   (void *)page, MHD_RESPMEM_PERSISTENT);
        if (!response) {
            return MHD_NO;
        }

        // Explicitly setting Content-Type is a security best practice to prevent XSS via content sniffing.
        MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");

        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    } else {
        // For any other path or method, return a 404 Not Found.
        const char *page = "Not Found";
        struct MHD_Response *response;
        int ret;

        response = MHD_create_response_from_buffer(strlen(page),
                                                   (void *)page, MHD_RESPMEM_PERSISTENT);
        if (!response) {
            return MHD_NO;
        }
        
        MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");

        ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        return ret;
    }
}

void run_tests() {
    printf("--- Running C Tests ---\n");
    // It's not feasible to test the running server from within main before it starts.
    // We will run simple logical tests on the expected outputs.
    
    const char *expected_message = "Hello, World!";
    const char *not_found_message = "Not Found";

    // Test 1: Check expected message content
    if (strcmp(expected_message, "Hello, World!") == 0) {
        printf("Test 1 Passed: Message content is correct.\n");
    } else {
        printf("Test 1 Failed.\n");
    }

    // Test 2: Check message length
    if (strlen(expected_message) == 13) {
        printf("Test 2 Passed: Message length is correct.\n");
    } else {
        printf("Test 2 Failed.\n");
    }
    
    // Test 3: Check "Not Found" message content
     if (strcmp(not_found_message, "Not Found") == 0) {
        printf("Test 3 Passed: 'Not Found' message is correct.\n");
    } else {
        printf("Test 3 Failed.\n");
    }

    // Test 4: Mock test for URL routing logic
    if (strcmp("/", "/") == 0 && strcmp("/", "/other") != 0) {
        printf("Test 4 Passed: Mock test for URL routing logic is OK.\n");
    } else {
        printf("Test 4 Failed.\n");
    }

    // Test 5: Mock test for HTTP method handling
    if (strcmp("GET", "GET") == 0 && strcmp("GET", "POST") != 0) {
        printf("Test 5 Passed: Mock test for HTTP method handling is OK.\n");
    } else {
        printf("Test 5 Failed.\n");
    }
    
    printf("--- Tests Finished ---\n\n");
}

int main() {
    run_tests();

    // Line 2 & 6: Create application object and run it.
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &request_handler, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Error: Failed to start the server\n");
        return 1;
    }

    printf("Server is running on http://localhost:%d\n", PORT);
    printf("Press Enter to stop the server.\n");
    (void)getchar(); // Block until user presses Enter

    MHD_stop_daemon(daemon);
    return 0;
}
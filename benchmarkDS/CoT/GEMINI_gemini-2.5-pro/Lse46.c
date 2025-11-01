// Note: This code requires the Mongoose Web Server Library.
// 1. Download "mongoose.c" and "mongoose.h" from https://github.com/cesanta/mongoose
// 2. Place both files in the same directory as this file.
// 3. Compile with:
//    Linux/macOS: gcc -o Lse46 Lse46.c mongoose.c
//    Windows: gcc -o Lse46.exe Lse46.c mongoose.c -lws2_32

#include "mongoose.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#endif

static const char *s_http_addr = "http://127.0.0.1:8083";
static const char *s_root_dir = "images";

/**
 * Event handler function for the Mongoose web server.
 * This function is called for every network event.
 */
static void event_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        // The "decorator" concept is implemented by checking the URL here.
        // We check if the request URI is '/img'.
        if (mg_http_match_uri(hm, "/img")) {
            char file_name_buf[256];
            
            // Get the 'name' parameter from the URL's query string.
            int result = mg_http_get_var(&hm->query, "name", file_name_buf, sizeof(file_name_buf));

            if (result <= 0) {
                // If 'name' parameter is missing or empty, send a 400 Bad Request.
                mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Missing 'name' parameter.\n");
                return;
            }

            // Security: Use Mongoose's built-in secure file serving function.
            // It safely joins the root directory and filename, and contains
            // robust protections against path traversal attacks (e.g., '../').
            struct mg_http_serve_opts opts = {.root_dir = s_root_dir};
            mg_http_serve_file(c, hm, file_name_buf, &opts);
        } else {
            // For any other URL, return a 404 Not Found error.
            mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Not Found\n");
        }
    }
    (void) fn_data; // Unused parameter
}

/**
 * Sets up a test directory and a sample file.
 */
void setupTestEnvironment() {
    printf("Setting up C test environment...\n");
    mkdir(s_root_dir, 0755);
    printf("Ensured directory exists: %s\n", s_root_dir);

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/test.txt", s_root_dir);
    
    FILE *f = fopen(filepath, "w");
    if (f != NULL) {
        fputs("This is a test file from C.\n", f);
        fclose(f);
        printf("Created test file: %s\n", filepath);
    } else {
        perror("Failed to create test file");
    }
}

// In C, the main class name "Lse46" is represented by the executable name.
int main(void) {
    setupTestEnvironment();
    struct mg_mgr mgr;

    mg_mgr_init(&mgr);
    
    printf("\n--- C Mongoose Server ---\n");
    printf("Starting server on %s\n", s_http_addr);

    // Create the listening connection.
    if (mg_http_listen(&mgr, s_http_addr, event_handler, NULL) == NULL) {
        fprintf(stderr, "Cannot start server on %s. Is the port in use?\n", s_http_addr);
        exit(EXIT_FAILURE);
    }

    printf("Press Ctrl+C to stop.\n");
    printf("\n--- 5 Test Cases ---\n");
    printf("1. Valid file:         %s/img?name=test.txt\n", s_http_addr);
    printf("2. File not found:     %s/img?name=nonexistent.txt\n", s_http_addr);
    printf("3. Path traversal 1:   %s/img?name=../Lse46.c (should be 403 Forbidden or 404)\n", s_http_addr);
    printf("4. Path traversal 2:   %s/img?name=..%%2ftest.txt (should be 403 or 404)\n", s_http_addr);
    printf("5. Missing parameter:  %s/img (should be 400)\n", s_http_addr);
    
    // Start the infinite event loop.
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    return 0;
}
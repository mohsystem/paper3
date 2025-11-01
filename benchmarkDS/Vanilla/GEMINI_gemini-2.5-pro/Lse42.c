/*
 * This code requires the Mongoose single-file web server library.
 * Download 'mongoose.c' and 'mongoose.h' from https://github.com/cesanta/mongoose
 * and place them in the same directory as this source file.
 *
 * Compilation command:
 * gcc -o server Lse42.c mongoose.c -I. -lpthread -std=c99
 * On Windows, you might need to link with -lws2_32
 * gcc -o server.exe Lse42.c mongoose.c -I. -lws2_32
 */
#include <stdio.h>
#include <string.h>
#include "mongoose.h"

// This function simulates retrieving data from a session and returns a JSON string.
// Note: In C, we must manage memory carefully. Here we use a static buffer.
// This is not thread-safe but is simple for demonstration.
const char* info() {
    static char json_buffer[256];
    const char* username = "c_user";
    const char* email = "c_user@example.com";
    const char* ssn = "444-55-6666"; // Sensitive data for demonstration only
    
    snprintf(json_buffer, sizeof(json_buffer),
             "{ \"username\": \"%s\", \"email\": \"%s\", \"ssn\": \"%s\" }",
             username, email, ssn);
             
    return json_buffer;
}

// Mongoose event handler function
static void event_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        // Check if the request is for GET /info
        if (mg_http_match_uri(hm, "/info") && mg_vcasecmp(&hm->method, "GET") == 0) {
            const char* response = info();
            mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s\n", response);
        } else {
            // For any other route, return a 404 Not Found
             mg_http_reply(c, 404, "", "Not Found\n");
        }
    }
    (void) fn_data; // Unused parameter
}

int main(void) {
    // The main function starts the web server.
    // The "5 test cases" are to be run manually by accessing the URL.
    struct mg_mgr mgr;
    int port = 8082;
    char addr[20];
    snprintf(addr, sizeof(addr), "http://0.0.0.0:%d", port);

    mg_mgr_init(&mgr);
    
    printf("C server starting on port %d\n", port);
    printf("Run the following 5 test cases in another terminal:\n");
    for (int i = 1; i <= 5; ++i) {
        printf("Test Case %d: curl http://localhost:%d/info\n", i, port);
    }
    
    mg_http_listen(&mgr, addr, event_handler, NULL);

    // Event loop
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    return 0;
}
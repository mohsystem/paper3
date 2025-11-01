/*
 * Instructions:
 * 1. This code requires the 'mongoose' single-header library.
 *    Download 'mongoose.h' and 'mongoose.c' from https://github.com/cesanta/mongoose
 *    Place them in the same directory as this source file.
 * 2. Compile this file along with mongoose.c.
 *    Example on Linux/macOS: gcc -o server Lse112.c mongoose.c -lpthread
 *    Example on Windows (MinGW): gcc -o server.exe Lse112.c mongoose.c -lws2_32
 * 3. Run the compiled executable: ./server
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mongoose.h"

// The URL/IP to ping is a hardcoded, trusted constant to prevent command injection.
static const char *s_url_to_ping = "8.8.8.8";
static const char *s_http_port = "8082";

// Executes a command and captures its output into a buffer.
static char* execute_command(const char* command) {
    FILE *fp;
    char buffer[1024];
    size_t total_size = 1; // Start with 1 for the null terminator
    char *result = (char*)malloc(total_size);
    if (!result) {
        return NULL;
    }
    result[0] = '\0';

#ifdef _WIN32
    fp = _popen(command, "r");
#else
    fp = popen(command, "r");
#endif

    if (fp == NULL) {
        fprintf(stderr, "Failed to run command\n");
        free(result);
        return NULL;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t len = strlen(buffer);
        char* new_result = (char*)realloc(result, total_size + len);
        if (!new_result) {
            fprintf(stderr, "realloc failed\n");
            free(result);
#ifdef _WIN32
            _pclose(fp);
#else
            pclose(fp);
#endif
            return NULL;
        }
        result = new_result;
        strcat(result, buffer);
        total_size += len;
    }

#ifdef _WIN32
    _pclose(fp);
#else
    pclose(fp);
#endif
    return result;
}

// Event handler function for the Mongoose web server.
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        // Check if the request is for the /ping URL.
        if (mg_http_match_uri(hm, "/ping")) {
            char command[128];
#ifdef _WIN32
            snprintf(command, sizeof(command), "ping -n 4 %s", s_url_to_ping);
#else
            snprintf(command, sizeof(command), "ping -c 4 %s", s_url_to_ping);
#endif
            
            char* output = execute_command(command);
            if (output) {
                mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "%s", output);
                free(output);
            } else {
                mg_http_reply(c, 500, "Content-Type: text/plain\r\n", "Error executing command.");
            }
        } else {
            // For any other URL, return a 404 Not Found.
            mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Not Found");
        }
    }
    (void) fn_data;
}

int main(void) {
    /*
     * Test Cases:
     * 1. Compile and run the server as per the instructions above.
     * 2. Open a terminal or command prompt.
     * 3. Run the following command: curl http://localhost:8082/ping
     *    - This should return the output of pinging 8.8.8.8.
     * 4. Access http://localhost:8082/ in a browser, which should show "Not Found".
     * 5. The design is secure because the address to be pinged is hardcoded.
     *    Any attempt to pass a parameter in the URL will be ignored.
    */
    struct mg_mgr mgr;
    
    mg_mgr_init(&mgr);
    printf("Starting web server on port %s\n", s_http_port);
    printf("Access http://localhost:%s/ping to execute the ping command.\n", s_http_port);

    if (mg_http_listen(&mgr, s_http_port, fn, NULL) == NULL) {
        fprintf(stderr, "Failed to create listener\n");
        return 1;
    }

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    return 0;
}
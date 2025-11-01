/*
 * This code requires the libmicrohttpd library.
 *
 * On Debian/Ubuntu: sudo apt-get install libmicrohttpd-dev
 * On Fedora/CentOS: sudo dnf install libmicrohttpd-devel
 * On macOS (Homebrew): brew install libmicrohttpd
 *
 * Compilation command:
 * gcc -o server Lse112.c -lmicrohttpd
 *
 * Then run with: ./server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef _WIN32
#include <winsock2.h>
#define popen _popen
#define pclose _pclose
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <microhttpd.h>

#define PORT 8080
#define MAX_RESPONSE_SIZE (100 * 1024) // 100 KB limit for the ping output

// The URL to ping is stored as a variable.
// This must be a static, trusted value to prevent command injection.
const char* PING_TARGET_URL = "8.8.8.8";

int answer_to_connection(void *cls, struct MHD_Connection *connection,
                         const char *url, const char *method,
                         const char *version, const char *upload_data,
                         size_t *upload_data_size, void **con_cls) {
    (void)cls; (void)version; (void)upload_data; (void)upload_data_size; (void)con_cls;

    if (0 != strcmp(method, "GET")) {
        return MHD_NO; // Only GET is allowed
    }

    if (0 == strcmp(url, "/ping")) {
        char command[256];
        int ret;
        
        #ifdef _WIN32
            ret = snprintf(command, sizeof(command), "ping -n 4 %s", PING_TARGET_URL);
        #else
            ret = snprintf(command, sizeof(command), "ping -c 4 %s", PING_TARGET_URL);
        #endif

        if (ret < 0 || (size_t)ret >= sizeof(command)) {
            const char *error_msg = "Command string is too long.";
            struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_msg), (void*)error_msg, MHD_RESPMEM_PERSISTENT);
            return MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        }

        FILE *pipe = popen(command, "r");
        if (!pipe) {
            const char *error_msg = "Failed to run ping command.";
            struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_msg), (void*)error_msg, MHD_RESPMEM_PERSISTENT);
            return MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        }

        size_t initial_capacity = 4096;
        char *buffer = malloc(initial_capacity);
        if (!buffer) {
            pclose(pipe);
            return MHD_NO; // Internal memory error
        }
        
        size_t current_size = 0;
        size_t capacity = initial_capacity;
        size_t bytes_read;

        while ((bytes_read = fread(buffer + current_size, 1, capacity - current_size, pipe)) > 0) {
            current_size += bytes_read;
            if (current_size >= MAX_RESPONSE_SIZE) {
                break; // Stop reading if max size is reached
            }
            if (current_size == capacity) {
                size_t new_capacity = capacity * 2;
                if (new_capacity > MAX_RESPONSE_SIZE) {
                    new_capacity = MAX_RESPONSE_SIZE;
                }
                char *new_buffer = realloc(buffer, new_capacity);
                if (!new_buffer) {
                    free(buffer);
                    pclose(pipe);
                    return MHD_NO; // Internal memory error
                }
                buffer = new_buffer;
                capacity = new_capacity;
            }
        }
        
        pclose(pipe);

        struct MHD_Response *response = MHD_create_response_from_buffer(current_size, (void *)buffer, MHD_RESPMEM_MUST_FREE);
        MHD_add_response_header(response, "Content-Type", "text/plain");
        int queue_ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return queue_ret;
    }

    const char *page = "Not Found";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    int queue_ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return queue_ret;
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return 1;
    }
#endif

    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server daemon.\n");
        return 1;
    }

    printf("Server started on port %d\n", PORT);
    printf("Access http://localhost:%d/ping to execute the command.\n", PORT);
    printf("Press Enter to exit.\n");
    
    // --- Test Cases ---
    // To test the server, compile and run this program, then use a tool like curl or a web browser.
    // Test Case 1: Access the ping URL from a browser:
    // Open http://localhost:8080/ping
    //
    // Test Case 2: Use curl in a terminal:
    // curl http://localhost:8080/ping
    //
    // Test Case 3: Test non-existent URL (should give 404 Not Found):
    // curl http://localhost:8080/other
    //
    // Test Case 4: Test a different HTTP method (should give 405 Method Not Allowed):
    // curl -X POST http://localhost:8080/ping
    //
    // Test Case 5: The main function itself acts as the primary test harness by running the server.
    // The server will run until the program is manually stopped.

    (void)getchar();

    MHD_stop_daemon(daemon);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
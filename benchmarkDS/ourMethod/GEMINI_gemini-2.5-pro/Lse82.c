/*
 * This C code creates a simple web server.
 * It depends on two external libraries:
 * 1. libmicrohttpd: For the HTTP server functionality.
 * 2. jansson: For JSON parsing.
 *
 * How to compile and run on a Debian-based system (like Ubuntu):
 * 1. Install the required libraries:
 *    sudo apt-get update
 *    sudo apt-get install libmicrohttpd-dev libjansson-dev
 * 2. Compile the source code:
 *    gcc -o server this_file.c -lmicrohttpd -ljansson
 * 3. Run the executable:
 *    ./server
 * 4. The server will start on http://localhost:8080.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <jansson.h>

#define PORT 8080

// Forward declaration
static int answer_to_connection(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls);

int main(void) {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Error: Failed to start MHD daemon\n");
        return 1;
    }

    printf("Server running on http://localhost:%d\n", PORT);
    printf("Use curl to test the API endpoint:\n");
    // Test cases (to be executed from a separate terminal):
    // Use %% to escape % in printf format string
    // 1. Valid request:
    printf("curl 'http://localhost:%d/api?payload=%%7B%%22name%%22%%3A%%22world%%22%%7D'\n", PORT);
    // 2. Missing 'payload' parameter:
    printf("curl 'http://localhost:%d/api'\n", PORT);
    // 3. Invalid JSON in 'payload':
    printf("curl 'http://localhost:%d/api?payload=invalid-json'\n", PORT);
    // 4. 'name' key missing in JSON:
    printf("curl 'http://localhost:%d/api?payload=%%7B%%22other_key%%22%%3A%%22value%%22%%7D'\n", PORT);
    // 5. 'name' value is not a string:
    printf("curl 'http://localhost:%d/api?payload=%%7B%%22name%%22%%3A123%%7D'\n", PORT);

    printf("Press Enter to stop the server.\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    return 0;
}

static int send_error_response(struct MHD_Connection *connection, const char *message, unsigned int status_code) {
    struct MHD_Response *response;
    int ret;
    response = MHD_create_response_from_buffer(strlen(message), (void *)message, MHD_RESPMEM_PERSISTENT);
    if (!response) return MHD_NO;
    MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret;
}


static int handle_api_request(struct MHD_Connection *connection) {
    const char *payload_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "payload");

    if (payload_str == NULL) {
        return send_error_response(connection, "'payload' parameter is missing", MHD_HTTP_BAD_REQUEST);
    }

    json_error_t error;
    json_t *root = json_loads(payload_str, 0, &error);

    if (!root) {
        return send_error_response(connection, "Invalid JSON format in 'payload'", MHD_HTTP_BAD_REQUEST);
    }

    if (!json_is_object(root)) {
        json_decref(root);
        return send_error_response(connection, "Payload must be a JSON object.", MHD_HTTP_BAD_REQUEST);
    }

    json_t *name_json = json_object_get(root, "name");
    if (!name_json || !json_is_string(name_json)) {
        json_decref(root);
        return send_error_response(connection, "'name' key is missing or not a string in JSON payload", MHD_HTTP_BAD_REQUEST);
    }

    const char *name_value = json_string_value(name_json);
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(name_value), (void *)name_value, MHD_RESPMEM_COPY);
    if(!response) {
        json_decref(root);
        return MHD_NO;
    }
    MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    json_decref(root);

    return ret;
}


static int answer_to_connection(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls) {
    (void)cls; (void)version; (void)upload_data; (void)upload_data_size; (void)con_cls; // Unused parameters

    if (0 == strcmp(method, "GET")) {
        if (0 == strcmp(url, "/api")) {
            return handle_api_request(connection);
        }
    }

    return send_error_response(connection, "Not Found", MHD_HTTP_NOT_FOUND);
}
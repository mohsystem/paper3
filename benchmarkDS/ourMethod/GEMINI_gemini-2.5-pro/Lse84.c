/*
 * Dependencies:
 * 1. libmicrohttpd: A small C library for embedding an HTTP server.
 *    Install (e.g., `sudo apt-get install libmicrohttpd-dev` or `brew install libmicrohttpd`).
 * 2. libyaml: A C library for parsing and emitting YAML.
 *    Install (e.g., `sudo apt-get install libyaml-dev` or `brew install libyaml`).
 *
 * Compilation:
 * gcc -std=c11 -o server Lse84.c -lmicrohttpd -lyaml
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <microhttpd.h>
#include <yaml.h>

#define PORT 5000
#define MAX_PAYLOAD_SIZE 4096

// State for our simple YAML parser
enum ParserState { STATE_START, STATE_IN_MAP, STATE_AWAITING_TYPE_VALUE };

char* process_payload(const char* payload, int* status_code_ptr) {
    if (payload == NULL || *payload == '\0') {
        *status_code_ptr = MHD_HTTP_BAD_REQUEST;
        return strdup("Error: 'payload' parameter is missing or empty.");
    }
    
    if (strlen(payload) > MAX_PAYLOAD_SIZE) {
        *status_code_ptr = MHD_HTTP_BAD_REQUEST;
        return strdup("Error: Payload exceeds maximum size limit.");
    }

    yaml_parser_t parser;
    yaml_token_t token;
    bool create_found = false;

    if (!yaml_parser_initialize(&parser)) {
        *status_code_ptr = MHD_HTTP_INTERNAL_SERVER_ERROR;
        return strdup("Error: Failed to initialize YAML parser.");
    }
    yaml_parser_set_input_string(&parser, (const unsigned char*)payload, strlen(payload));

    enum ParserState state = STATE_START;
    do {
        if (!yaml_parser_scan(&parser, &token)) {
            *status_code_ptr = MHD_HTTP_BAD_REQUEST;
            char* err_msg = strdup("Error: Invalid YAML format in payload.");
            yaml_token_delete(&token); // Free partially initialized token
            yaml_parser_delete(&parser);
            return err_msg;
        }

        switch (token.type) {
            case YAML_MAPPING_START_TOKEN: state = STATE_IN_MAP; break;
            case YAML_SCALAR_TOKEN:
                if (state == STATE_IN_MAP && strcmp((char*)token.data.scalar.value, "type") == 0) {
                    state = STATE_AWAITING_TYPE_VALUE;
                } else if (state == STATE_AWAITING_TYPE_VALUE) {
                    if (strcmp((char*)token.data.scalar.value, "Create") == 0) {
                        create_found = true;
                    }
                    state = STATE_IN_MAP; // Reset state after finding the value for "type"
                }
                break;
            default: break;
        }
        if (create_found) break; // We can stop early
        yaml_token_delete(&token);
    } while (token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    
    if (create_found) {
        *status_code_ptr = MHD_HTTP_BAD_REQUEST;
        return strdup("Error: 'Create' type is not allowed.");
    }
    *status_code_ptr = MHD_HTTP_OK;
    return strdup("Success: Payload processed.");
}

static enum MHD_Result request_handler(void* cls, struct MHD_Connection* connection,
                                       const char* url, const char* method,
                                       const char* version, const char* upload_data,
                                       size_t* upload_data_size, void** con_cls) {
    if (strcmp(method, "GET") != 0) return MHD_NO;

    const char* payload = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "payload");
    int status_code;
    
    char* response_str = process_payload(payload, &status_code);
    if (response_str == NULL) {
         status_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
         response_str = strdup("Internal Server Error");
    }

    struct MHD_Response* response = MHD_create_response_from_buffer(strlen(response_str),
                                                                    response_str,
                                                                    MHD_RESPMEM_MUST_FREE);
    if (!response) {
        free(response_str);
        return MHD_NO;
    }

    MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");
    enum MHD_Result ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret;
}

int main() {
    printf("--- Test Cases (run in another terminal) ---\n");
    printf("1. No payload parameter:\n");
    printf("   curl -v \"http://localhost:5000/\"\n");
    printf("2. Payload with type 'Create' (should be rejected):\n");
    printf("   curl -v \"http://localhost:5000/?payload=type%%3A%%20Create%%0Aname%%3A%%20test\"\n");
    printf("3. Payload with a different type (should be accepted):\n");
    printf("   curl -v \"http://localhost:5000/?payload=type%%3A%%20Update%%0Aname%%3A%%20test\"\n");
    printf("4. Malformed YAML payload (should be rejected):\n");
    printf("   curl -v \"http://localhost:5000/?payload=type%%3A%%20Create%%3A%%20Fail\"\n");
    printf("5. Empty payload (should be rejected):\n");
    printf("   curl -v \"http://localhost:5000/?payload=\"\n");

    struct MHD_Daemon* daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                                                 &request_handler, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Error: Could not start server.\n");
        return 1;
    }
    printf("\nServer running on port %d. Press Enter to exit.\n", PORT);
    (void)getchar();
    MHD_stop_daemon(daemon);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <json-c/json.h>

/* Maximum payload size to prevent DoS attacks */
#define MAX_PAYLOAD_SIZE 4096
/* Maximum name field length */
#define MAX_NAME_LENGTH 256

/*
 * Security: Safely extract name from JSON payload
 * - Validates JSON structure
 * - Checks for "name" field existence
 * - Bounds checking on output
 * - Returns NULL on any validation failure
 */
static char* extract_name_from_payload(const char* payload_str) {
    char *result = NULL;
    struct json_object *parsed_json = NULL;
    struct json_object *name_obj = NULL;
    
    /* Security: Validate input pointer */
    if (payload_str == NULL || strlen(payload_str) == 0) {
        return NULL;
    }
    
    /* Security: Validate payload size before parsing */
    size_t len = strlen(payload_str);
    if (len > MAX_PAYLOAD_SIZE) {
        return NULL;
    }
    
    /* Security: Use safe JSON parser (json-c), not eval or unsafe deserialization */
    parsed_json = json_tokener_parse(payload_str);
    if (parsed_json == NULL) {
        return NULL;
    }
    
    /* Security: Validate that parsed object is actually a JSON object */
    if (!json_object_is_type(parsed_json, json_type_object)) {
        json_object_put(parsed_json);
        return NULL;
    }
    
    /* Security: Safely check for "name" field existence */
    if (!json_object_object_get_ex(parsed_json, "name", &name_obj)) {
        json_object_put(parsed_json);
        return NULL;
    }
    
    /* Security: Validate that name field is a string */
    if (!json_object_is_type(name_obj, json_type_string)) {
        json_object_put(parsed_json);
        return NULL;
    }
    
    const char *name_str = json_object_get_string(name_obj);
    
    /* Security: Validate name string and enforce length limit */
    if (name_str == NULL || strlen(name_str) >= MAX_NAME_LENGTH) {
        json_object_put(parsed_json);
        return NULL;
    }
    
    /* Security: Allocate buffer with explicit size and check result */
    result = (char*)calloc(MAX_NAME_LENGTH, sizeof(char));
    if (result == NULL) {
        json_object_put(parsed_json);
        return NULL;
    }
    
    /* Security: Use bounded copy with explicit size */
    snprintf(result, MAX_NAME_LENGTH, "%s", name_str);
    
    /* Cleanup: Release JSON object */
    json_object_put(parsed_json);
    
    return result;
}

/*
 * HTTP request handler for /api endpoint
 * Security: Validates all inputs and returns generic errors
 */
static enum MHD_Result answer_to_connection(void *cls, struct MHD_Connection *connection,
                                           const char *url, const char *method,
                                           const char *version, const char *upload_data,
                                           size_t *upload_data_size, void **con_cls) {
    struct MHD_Response *response = NULL;
    enum MHD_Result ret;
    int status_code = MHD_HTTP_OK;
    char *response_text = NULL;
    
    (void)cls; (void)version; (void)upload_data; (void)upload_data_size; (void)con_cls;
    
    /* Security: Only accept GET requests */
    if (strcmp(method, "GET") != 0) {
        response_text = strdup("Method not allowed");
        status_code = MHD_HTTP_METHOD_NOT_ALLOWED;
        goto send_response;
    }
    
    /* Security: Only handle /api route */
    if (strcmp(url, "/api") != 0) {
        response_text = strdup("Not found");
        status_code = MHD_HTTP_NOT_FOUND;
        goto send_response;
    }
    
    /* Security: Get payload parameter safely */
    const char *payload = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "payload");
    
    /* Security: Validate payload parameter exists */
    if (payload == NULL) {
        response_text = strdup("{\\"error\\":\\"Missing payload parameter\\"}");
        status_code = MHD_HTTP_BAD_REQUEST;
        goto send_response;
    }
    
    /* Security: Extract name with full validation */
    char *name = extract_name_from_payload(payload);
    if (name == NULL) {
        /* Security: Generic error message, no internal details leaked */
        response_text = strdup("{\\"error\\":\\"Invalid payload format\\"}");
        status_code = MHD_HTTP_BAD_REQUEST;
        goto send_response;
    }
    
    /* Security: Allocate response buffer with explicit size check */
    size_t response_size = strlen(name) + 64;
    response_text = (char*)calloc(response_size, sizeof(char));
    if (response_text == NULL) {
        free(name);
        response_text = strdup("{\\"error\\":\\"Internal error\\"}");
        status_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
        goto send_response;
    }
    
    /* Security: Use snprintf with explicit bound and constant format string */
    snprintf(response_text, response_size, "{\\"name\\":\\"%s\\"}", name);
    
    /* Cleanup: Free allocated name buffer */
    free(name);
    
send_response:
    /* Security: Ensure response_text is never NULL */
    if (response_text == NULL) {
        response_text = strdup("{\\"error\\":\\"Internal error\\"}");
        status_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
    }
    
    /* Create and send response */
    response = MHD_create_response_from_buffer(strlen(response_text),
                                               (void*)response_text,
                                               MHD_RESPMEM_MUST_FREE);
    
    /* Security: Check response creation succeeded */
    if (response == NULL) {
        free(response_text);
        return MHD_NO;
    }
    
    /* Add content type header */
    MHD_add_response_header(response, "Content-Type", "application/json");
    
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    
    return ret;
}

int main(void) {
    struct MHD_Daemon *daemon;
    
    /* Test cases */
    printf("Starting web server on port 8888...\\n");
    printf("Test with:\\n");
    printf("1. curl 'http://localhost:8888/api?payload={\\"name\\":\\"Alice\\"}'\\n");
    printf("2. curl 'http://localhost:8888/api?payload={\\"name\\":\\"Bob123\\"}'\\n");
    printf("3. curl 'http://localhost:8888/api?payload={\\"name\\":\\"\\"}'\\n");
    printf("4. curl 'http://localhost:8888/api?payload={\\"invalid\\":\\"data\\"}'\\n");
    printf("5. curl 'http://localhost:8888/api?payload=notjson'\\n");
    
    /* Security: Start daemon with minimal privileges */
    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, 8888, NULL, NULL,
                             &answer_to_connection, NULL, MHD_OPTION_END);
    
    /* Security: Check daemon creation succeeded */
    if (daemon == NULL) {
        fprintf(stderr, "Failed to start server\\n");
        return EXIT_FAILURE;
    }
    
    printf("Server started. Press Enter to stop.\\n");
    (void)getchar();
    
    MHD_stop_daemon(daemon);
    return EXIT_SUCCESS;
}

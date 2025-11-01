
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_PAYLOAD_SIZE 10240
#define PORT 5000
#define BUFFER_SIZE 65536

/* Note: Full YAML parsing in C requires external libraries like libyaml.
   This implementation provides a simplified YAML parser that handles basic key-value pairs.
   For production use, integrate libyaml with proper security configurations. */

/* URL decode function - validates and decodes safely */
char* url_decode(const char* str) {
    if (str == NULL) return NULL;
    
    size_t len = strlen(str);
    char* result = (char*)calloc(len + 1, sizeof(char));
    if (result == NULL) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len && j < len; i++) {
        if (str[i] == '%' && i + 2 < len && isxdigit(str[i + 1]) && isxdigit(str[i + 2])) {
            char hex[3] = {str[i + 1], str[i + 2], '\\0'};
            result[j++] = (char)strtol(hex, NULL, 16);
            i += 2;
        } else if (str[i] == '+') {
            result[j++] = ' ';
        } else {
            result[j++] = str[i];
        }
    }
    result[j] = '\\0';
    return result;
}

/* Extract query parameter value - returns newly allocated string or NULL */
char* get_query_param(const char* query, const char* key) {
    if (query == NULL || key == NULL) return NULL;
    
    size_t key_len = strlen(key);
    const char* pos = query;
    
    while (pos != NULL && *pos != '\\0') {
        if (strncmp(pos, key, key_len) == 0 && pos[key_len] == '=') {
            pos += key_len + 1;
            const char* end = strchr(pos, '&');
            size_t value_len = end ? (size_t)(end - pos) : strlen(pos);
            
            if (value_len > MAX_PAYLOAD_SIZE) {
                return NULL;
            }
            
            char* encoded_value = (char*)calloc(value_len + 1, sizeof(char));
            if (encoded_value == NULL) return NULL;
            
            strncpy(encoded_value, pos, value_len);
            encoded_value[value_len] = '\\0';
            
            char* decoded_value = url_decode(encoded_value);
            free(encoded_value);
            return decoded_value;
        }
        
        pos = strchr(pos, '&');
        if (pos) pos++;
    }
    
    return NULL;
}

/* Simplified YAML parser - checks for "type: Create" pattern */
int check_yaml_type_create(const char* payload) {
    if (payload == NULL) return 0;
    
    size_t len = strlen(payload);
    if (len > MAX_PAYLOAD_SIZE) return -1;
    
    /* Look for "type:" followed by optional whitespace and "Create" */
    const char* type_pos = strstr(payload, "type:");
    if (type_pos == NULL) {
        return 0; /* No type field found - OK */
    }
    
    /* Skip "type:" and whitespace */
    type_pos += 5;
    while (*type_pos == ' ' || *type_pos == '\\t') {
        type_pos++;
    }
    
    /* Check if value is "Create" */
    if (strncmp(type_pos, "Create", 6) == 0) {
        char next_char = type_pos[6];
        if (next_char == '\\0' || next_char == '\\n' || next_char == '\\r' || next_char == ' ') {
            return 1; /* Found "type: Create" */
        }
    }
    
    return 0;
}

/* Process payload and return response message */
const char* process_payload(const char* payload) {
    if (payload == NULL) {
        return "Error: Invalid payload format";
    }
    
    int result = check_yaml_type_create(payload);
    
    if (result == -1) {
        return "Error: Payload too large";
    } else if (result == 1) {
        return "Error: Create type not allowed";
    } else {
        return "Success: Payload processed";
    }
}

/* Handle HTTP request */
void handle_request(int client_socket) {
    char* buffer = NULL;
    char* payload = NULL;
    
    buffer = (char*)calloc(BUFFER_SIZE, sizeof(char));
    if (buffer == NULL) {
        const char* error_msg = "HTTP/1.1 500 Internal Server Error\\r\\n\\r\\nError: Memory allocation failed";
        send(client_socket, error_msg, strlen(error_msg), 0);
        return;
    }
    
    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
        free(buffer);
        return;
    }
    
    buffer[bytes_read] = '\\0';
    
    /* Parse request line */
    char method[16] = {0};
    char path[1024] = {0};
    if (sscanf(buffer, "%15s %1023s", method, path) != 2) {
        const char* error_msg = "HTTP/1.1 400 Bad Request\\r\\n\\r\\nError: Invalid request";
        send(client_socket, error_msg, strlen(error_msg), 0);
        free(buffer);
        return;
    }
    
    /* Only accept POST requests */
    if (strcmp(method, "POST") != 0) {
        const char* error_msg = "HTTP/1.1 405 Method Not Allowed\\r\\n\\r\\nError: Method not allowed";
        send(client_socket, error_msg, strlen(error_msg), 0);
        free(buffer);
        return;
    }
    
    /* Extract query string */
    char* query = strchr(path, '?');
    if (query == NULL) {
        const char* error_msg = "HTTP/1.1 400 Bad Request\\r\\n\\r\\nError: Missing payload parameter";
        send(client_socket, error_msg, strlen(error_msg), 0);
        free(buffer);
        return;
    }
    
    query++; /* Skip '?' */
    
    /* Get payload parameter */
    payload = get_query_param(query, "payload");
    if (payload == NULL) {
        const char* error_msg = "HTTP/1.1 400 Bad Request\\r\\n\\r\\nError: Missing payload parameter";
        send(client_socket, error_msg, strlen(error_msg), 0);
        free(buffer);
        return;
    }
    
    /* Process payload */
    const char* result = process_payload(payload);
    
    /* Build response */
    char response[2048];
    int response_len = snprintf(response, sizeof(response), "HTTP/1.1 200 OK\\r\\n\\r\\n%s", result);
    if (response_len > 0 && response_len < (int)sizeof(response)) {
        send(client_socket, response, response_len, 0);
    }
    
    /* Clean up */
    free(payload);
    free(buffer);
}

void run_test_cases(void) {
    printf("\\n=== Running Test Cases ===\\n");
    
    printf("\\nTest 1: Valid YAML with type=Create\\n");
    printf("%s\\n", process_payload("type: Create\\ndata: test"));
    
    printf("\\nTest 2: Valid YAML with type=Update\\n");
    printf("%s\\n", process_payload("type: Update\\ndata: test"));
    
    printf("\\nTest 3: Valid YAML without type field\\n");
    printf("%s\\n", process_payload("data: test\\nvalue: 123"));
    
    printf("\\nTest 4: Invalid YAML syntax\\n");
    printf("%s\\n", process_payload("invalid: [yaml: syntax"));
    
    printf("\\nTest 5: Empty payload\\n");
    printf("%s\\n", process_payload(""));
}

int main(void) {
    run_test_cases();
    
    int server_fd = -1;
    int client_socket = -1;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(server_fd);
        perror("Setsockopt failed");
        return 1;
    }
    
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        close(server_fd);
        perror("Bind failed");
        return 1;
    }
    
    if (listen(server_fd, 3) == -1) {
        close(server_fd);
        perror("Listen failed");
        return 1;
    }
    
    printf("\\nServer started on port %d\\n", PORT);
    
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket == -1) {
            continue;
        }
        
        handle_request(client_socket);
        close(client_socket);
    }
    
    close(server_fd);
    return 0;
}

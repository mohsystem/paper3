
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERNAME_LENGTH 100
#define MAX_LINE_LENGTH 1024

typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} StringBuffer;

void string_buffer_init(StringBuffer* sb) {
    sb->capacity = 256;
    sb->data = (char*)malloc(sb->capacity);
    sb->size = 0;
    if (sb->data) {
        sb->data[0] = '\\0';
    }
}

void string_buffer_append(StringBuffer* sb, const char* str) {
    size_t len = strlen(str);
    if (sb->size + len + 1 > sb->capacity) {
        sb->capacity = (sb->size + len + 1) * 2;
        sb->data = (char*)realloc(sb->data, sb->capacity);
    }
    if (sb->data) {
        strcat(sb->data, str);
        sb->size += len;
    }
}

void string_buffer_free(StringBuffer* sb) {
    if (sb->data) {
        free(sb->data);
        sb->data = NULL;
    }
}

int is_safe_char(char c) {
    return isalnum(c) || c == '_' || c == '-' || c == ' ';
}

void sanitize_input(char* output, const char* input, size_t max_len) {
    size_t i, j = 0;
    size_t input_len = strlen(input);
    
    if (input_len > max_len) {
        input_len = max_len;
    }
    
    for (i = 0; i < input_len && j < max_len - 1; i++) {
        if (is_safe_char(input[i])) {
            output[j++] = input[i];
        }
    }
    output[j] = '\\0';
    
    // Trim whitespace
    while (j > 0 && isspace(output[j - 1])) {
        output[--j] = '\\0';
    }
}

void html_encode(StringBuffer* sb, const char* input) {
    for (size_t i = 0; input[i] != '\\0'; i++) {
        switch (input[i]) {
            case '&': string_buffer_append(sb, "&amp;"); break;
            case '<': string_buffer_append(sb, "&lt;"); break;
            case '>': string_buffer_append(sb, "&gt;"); break;
            case '"': string_buffer_append(sb, "&quot;"); break;
            case '\\'': string_buffer_append(sb, "&#x27;"); break;
            case '/': string_buffer_append(sb, "&#x2F;"); break;
            default: {
                char temp[2] = {input[i], '\\0'};
                string_buffer_append(sb, temp);
            }
        }
    }
}

char* extract_username(FILE* request) {
    char line[MAX_LINE_LENGTH];
    static char username[MAX_USERNAME_LENGTH + 1];
    
    while (fgets(line, sizeof(line), request)) {
        if (line[0] == '\\n' || line[0] == '\\r') break;
        
        if (strncasecmp(line, "username:", 9) == 0) {
            char* start = line + 9;
            while (isspace(*start)) start++;
            
            size_t len = strlen(start);
            while (len > 0 && (start[len - 1] == '\\n' || start[len - 1] == '\\r')) {
                start[--len] = '\\0';
            }
            
            strncpy(username, start, MAX_USERNAME_LENGTH);
            username[MAX_USERNAME_LENGTH] = '\\0';
            return username;
        }
    }
    
    strcpy(username, "Guest");
    return username;
}

char* http_hello_page(FILE* request, FILE* response) {
    static char result[MAX_USERNAME_LENGTH * 8];
    char* username = extract_username(request);
    char sanitized[MAX_USERNAME_LENGTH + 1];
    
    if (username == NULL || strlen(username) == 0) {
        strcpy(sanitized, "Guest");
    } else {
        sanitize_input(sanitized, username, MAX_USERNAME_LENGTH);
        if (strlen(sanitized) == 0) {
            strcpy(sanitized, "Guest");
        }
    }
    
    StringBuffer encoded;
    string_buffer_init(&encoded);
    html_encode(&encoded, sanitized);
    
    snprintf(result, sizeof(result), "hello %s", encoded.data);
    
    fprintf(response, "HTTP/1.1 200 OK\\r\\n");
    fprintf(response, "Content-Type: text/html; charset=UTF-8\\r\\n");
    fprintf(response, "Content-Length: %zu\\r\\n", strlen(result));
    fprintf(response, "\\r\\n");
    fprintf(response, "%s", result);
    fflush(response);
    
    string_buffer_free(&encoded);
    return result;
}

void test_case(const char* input, const char* description) {
    FILE* request = fmemopen((void*)input, strlen(input), "r");
    FILE* response = tmpfile();
    
    if (request && response) {
        char* result = http_hello_page(request, response);
        printf("Test: %s\\n", description);
        printf("Input: %s\\n", input);
        printf("Output: %s\\n", result);
        printf("---\\n");
        
        fclose(request);
        fclose(response);
    }
}

int main() {
    // Test case 1: Normal username
    test_case("username: John", "Normal username");
    
    // Test case 2: Empty username
    test_case("", "Empty username");
    
    // Test case 3: XSS attempt
    test_case("username: <script>alert('xss')</script>", "XSS attempt");
    
    // Test case 4: Long username
    char long_input[200];
    sprintf(long_input, "username: %s", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    test_case(long_input, "Long username");
    
    // Test case 5: SQL injection attempt
    test_case("username: admin' OR '1'='1", "SQL injection attempt");
    
    return 0;
}

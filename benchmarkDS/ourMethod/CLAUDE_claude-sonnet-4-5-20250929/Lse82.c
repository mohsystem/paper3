
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Security: Maximum payload size to prevent DoS attacks */
#define MAX_PAYLOAD_SIZE 10240
#define MAX_NAME_SIZE 1024

/* Security: Safe string operations with bounds checking */
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} SafeString;

/* Security: Initialize SafeString with size validation */
SafeString* safestring_create(size_t initial_capacity) {
    if (initial_capacity == 0 || initial_capacity > MAX_PAYLOAD_SIZE) {
        return NULL;
    }
    
    SafeString* str = (SafeString*)malloc(sizeof(SafeString));
    if (str == NULL) {
        return NULL;
    }
    
    str->data = (char*)calloc(initial_capacity + 1, sizeof(char));
    if (str->data == NULL) {
        free(str);
        return NULL;
    }
    
    str->length = 0;
    str->capacity = initial_capacity;
    return str;
}

/* Security: Safe cleanup of SafeString */
void safestring_destroy(SafeString* str) {
    if (str != NULL) {
        if (str->data != NULL) {
            /* Security: Clear memory before free to prevent data leakage */
            memset(str->data, 0, str->capacity + 1);
            free(str->data);
        }
        free(str);
    }
}

/* Security: Safe character append with bounds checking */
int safestring_append_char(SafeString* str, char c) {
    if (str == NULL || str->data == NULL) {
        return 0;
    }
    
    /* Security: Check bounds before writing */
    if (str->length >= str->capacity) {
        return 0;
    }
    
    str->data[str->length++] = c;
    str->data[str->length] = '\\0';
    return 1;
}

/* Security: Escape special JSON characters to prevent injection */
char* escape_json_string(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t input_len = strlen(input);
    if (input_len > MAX_NAME_SIZE) {
        return NULL;
    }
    
    /* Security: Allocate buffer with size validation */
    SafeString* output = safestring_create(input_len * 2);
    if (output == NULL) {
        return NULL;
    }
    
    /* Security: Process each character safely with bounds checking */
    for (size_t i = 0; i < input_len; i++) {
        char c = input[i];
        switch (c) {
            case '\\\\':\n                if (!safestring_append_char(output, '\\\\') || \n                    !safestring_append_char(output, '\\\\')) {\n                    safestring_destroy(output);\n                    return NULL;\n                }\n                break;\n            case '"':\n                if (!safestring_append_char(output, '\\\\') || \n                    !safestring_append_char(output, '"')) {\n                    safestring_destroy(output);\n                    return NULL;\n                }\n                break;\n            case '\
':\n                if (!safestring_append_char(output, '\\\\') || \n                    !safestring_append_char(output, 'n')) {\n                    safestring_destroy(output);\n                    return NULL;\n                }\n                break;\n            case '\\r':\n                if (!safestring_append_char(output, '\\\\') || \n                    !safestring_append_char(output, 'r')) {\n                    safestring_destroy(output);\n                    return NULL;\n                }\n                break;\n            case '\\t':\n                if (!safestring_append_char(output, '\\\\') || \n                    !safestring_append_char(output, 't')) {\n                    safestring_destroy(output);\n                    return NULL;\n                }\n                break;\n            default:\n                if (!safestring_append_char(output, c)) {\n                    safestring_destroy(output);\n                    return NULL;\n                }\n                break;\n        }\n    }\n    \n    /* Security: Duplicate string safely before destroying SafeString */\n    char* result = strdup(output->data);\n    safestring_destroy(output);\n    return result;\n}\n\n/* Security: Extract name from JSON payload with safe parsing */\nchar* extract_name_from_json(const char* payload) {\n    if (payload == NULL) {\n        return NULL;\n    }\n    \n    /* Security: Find "name" key safely with bounds checking */\n    const char* name_key = strstr(payload, "\\"name\\"");\n    if (name_key == NULL) {\n        return NULL;\n    }\n    \n    /* Security: Find the colon after "name" */\n    const char* colon = strchr(name_key, ':');\n    if (colon == NULL) {\n        return NULL;\n    }\n    \n    /* Security: Skip whitespace and find opening quote */\n    const char* ptr = colon + 1;\n    while (*ptr && isspace((unsigned char)*ptr)) {\n        ptr++;\n    }\n    \n    if (*ptr != '"') {\n        return NULL;\n    }\n    ptr++; /* Skip opening quote */\n    \n    /* Security: Extract value with length validation */\n    const char* value_start = ptr;\n    size_t value_len = 0;\n    \n    while (*ptr && *ptr != '"' && value_len < MAX_NAME_SIZE) {\n        if (*ptr == '\\\\' && *(ptr + 1)) {\n            ptr++; /* Skip escape character */\n        }\n        ptr++;\n        value_len++;\n    }\n    \n    if (*ptr != '"' || value_len == 0 || value_len >= MAX_NAME_SIZE) {\n        return NULL;\n    }\n    \n    /* Security: Allocate and copy with size validation */\n    char* name = (char*)malloc(value_len + 1);\n    if (name == NULL) {\n        return NULL;\n    }\n    \n    /* Security: Use safe string copy with explicit size */\n    memcpy(name, value_start, value_len);\n    name[value_len] = '\\0';\n    \n    return name;\n}\n\n/* Security: Process payload with comprehensive input validation */\nchar* process_payload(const char* payload) {\n    /* Security: Validate input is not NULL and within size limits */\n    if (payload == NULL) {\n        return strdup("{\\"error\\":\\"Invalid input\\"}");
    }
    
    size_t payload_len = strlen(payload);
    if (payload_len == 0) {
        return strdup("{\\"error\\":\\"Invalid input\\"}");
    }
    
    if (payload_len > MAX_PAYLOAD_SIZE) {
        return strdup("{\\"error\\":\\"Payload too large\\"}");
    }
    
    /* Security: Extract name safely with bounds checking */
    char* name = extract_name_from_json(payload);
    if (name == NULL) {
        return strdup("{\\"error\\":\\"Missing name field\\"}");
    }
    
    /* Security: Escape name for safe JSON output */
    char* escaped_name = escape_json_string(name);
    
    /* Security: Clear and free original name */
    memset(name, 0, strlen(name));
    free(name);
    
    if (escaped_name == NULL) {
        return strdup("{\\"error\\":\\"Processing error\\"}");
    }
    
    /* Security: Build result with size validation */
    size_t result_size = strlen(escaped_name) + 32;
    char* result = (char*)malloc(result_size);
    if (result == NULL) {
        memset(escaped_name, 0, strlen(escaped_name));
        free(escaped_name);
        return strdup("{\\"error\\":\\"Memory error\\"}");
    }
    
    /* Security: Use snprintf for safe string formatting */
    snprintf(result, result_size, "{\\"name\\":\\"%s\\"}", escaped_name);
    
    /* Security: Clear and free escaped name */
    memset(escaped_name, 0, strlen(escaped_name));
    free(escaped_name);
    
    return result;
}

int main(void) {
    printf("Web API simulation (C version)\\n");
    printf("Note: C doesn't have built-in HTTP server.\\n");
    printf("This demonstrates the payload processing logic.\\n\\n");
    
    /* Test cases */
    printf("Test cases:\\n");
    
    char* result1 = process_payload("{\\"name\\":\\"John\\"}");
    printf("1. Valid JSON: %s\\n", result1);
    free(result1);
    
    char* result2 = process_payload("{\\"name\\":\\"O'Brien\\"}");
    printf("2. Valid JSON with special chars: %s\\n", result2);
    free(result2);
    
    char* result3 = process_payload("{\\"age\\":30}");
    printf("3. Missing name field: %s\\n", result3);
    free(result3);
    
    char* result4 = process_payload("{invalid}");
    printf("4. Invalid JSON: %s\\n", result4);
    free(result4);
    
    char* result5 = process_payload(NULL);
    printf("5. NULL payload: %s\\n", result5);
    free(result5);
    
    return 0;
}

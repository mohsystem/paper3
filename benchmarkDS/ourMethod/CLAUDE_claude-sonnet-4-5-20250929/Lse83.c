
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_PAYLOAD_SIZE (10 * 1024)  /* 10KB limit to prevent DoS */
#define MAX_NAME_LENGTH 100
#define MAX_ERROR_MSG 256

/* Validates if character is allowed in name field */
static bool is_valid_char(char c) {
    return isalnum((unsigned char)c) || c == ' ' || c == '.' || c == '_' || c == '-';
}

/* Extracts value from simple JSON key-value pair */
static char* extract_json_value(const char* json, const char* key) {
    if (json == NULL || key == NULL) {
        return NULL;
    }
    
    /* Build search pattern: "key":" */\n    char pattern[256];\n    int ret = snprintf(pattern, sizeof(pattern), "\\"%s\\"", key);
    if (ret < 0 || (size_t)ret >= sizeof(pattern)) {
        return NULL;
    }
    
    /* Find key in JSON */
    const char* key_pos = strstr(json, pattern);
    if (key_pos == NULL) {
        return NULL;
    }
    
    /* Find opening quote of value */
    const char* value_start = strchr(key_pos + strlen(pattern), '"');
    if (value_start == NULL) {
        return NULL;
    }
    value_start++; /* Move past opening quote */
    
    /* Find closing quote of value */
    const char* value_end = strchr(value_start, '"');
    if (value_end == NULL) {
        return NULL;
    }
    
    /* Calculate value length with bounds check */
    size_t value_len = (size_t)(value_end - value_start);
    if (value_len > MAX_NAME_LENGTH) {
        return NULL;
    }
    
    /* Allocate and copy value */
    char* value = (char*)malloc(value_len + 1);
    if (value == NULL) {
        return NULL;
    }
    
    memcpy(value, value_start, value_len);
    value[value_len] = '\\0';
    
    return value;
}

/* Validates and extracts the 'name' field from JSON payload */
char* process_payload(const char* payload) {
    char* result = NULL;
    char* name = NULL;
    
    /* Input validation: check payload is not NULL */
    if (payload == NULL) {
        result = (char*)malloc(MAX_ERROR_MSG);
        if (result != NULL) {
            snprintf(result, MAX_ERROR_MSG, "Error: NULL payload");
        }
        return result;
    }
    
    /* Input validation: check payload size to prevent DoS */
    size_t payload_len = strlen(payload);
    if (payload_len == 0) {
        result = (char*)malloc(MAX_ERROR_MSG);
        if (result != NULL) {
            snprintf(result, MAX_ERROR_MSG, "Error: Empty payload");
        }
        return result;
    }
    
    if (payload_len > MAX_PAYLOAD_SIZE) {
        result = (char*)malloc(MAX_ERROR_MSG);
        if (result != NULL) {
            snprintf(result, MAX_ERROR_MSG, "Error: Payload too large");
        }
        return result;
    }
    
    /* Extract 'name' field from JSON */
    name = extract_json_value(payload, "name");
    
    if (name == NULL) {
        result = (char*)malloc(MAX_ERROR_MSG);
        if (result != NULL) {
            snprintf(result, MAX_ERROR_MSG, "Error: Name field not found");
        }
        return result;
    }
    
    /* Validate name length */
    size_t name_len = strlen(name);
    if (name_len == 0) {
        result = (char*)malloc(MAX_ERROR_MSG);
        if (result != NULL) {
            snprintf(result, MAX_ERROR_MSG, "Error: Name is empty");
        }
        free(name);
        return result;
    }
    
    if (name_len > MAX_NAME_LENGTH) {
        result = (char*)malloc(MAX_ERROR_MSG);
        if (result != NULL) {
            snprintf(result, MAX_ERROR_MSG, "Error: Name too long");
        }
        free(name);
        return result;
    }
    
    /* Input validation: sanitize by checking for valid characters only */
    for (size_t i = 0; i < name_len; i++) {
        if (!is_valid_char(name[i])) {
            result = (char*)malloc(MAX_ERROR_MSG);
            if (result != NULL) {
                snprintf(result, MAX_ERROR_MSG, "Error: Invalid characters in name");
            }
            free(name);
            return result;
        }
    }
    
    /* Return validated and sanitized name */
    return name;
}

int main(void) {
    char* result = NULL;
    
    /* Test Case 1 - Valid name */
    printf("Test Case 1 - Valid name:\\n");
    result = process_payload("{\\"name\\":\\"John_Doe\\"}");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test Case 2 - Missing name field */
    printf("\\nTest Case 2 - Missing name field:\\n");
    result = process_payload("{\\"age\\":30}");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test Case 3 - Invalid JSON */
    printf("\\nTest Case 3 - Invalid JSON:\\n");
    result = process_payload("{invalid json}");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test Case 4 - Name with invalid characters */
    printf("\\nTest Case 4 - Name with invalid characters:\\n");
    result = process_payload("{\\"name\\":\\"<script>alert('xss')</script>\\"}");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test Case 5 - Empty payload */
    printf("\\nTest Case 5 - Empty payload:\\n");
    result = process_payload("");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_INPUT_SIZE (5 * 1024 * 1024)
#define MAX_DECODE_SIZE (10 * 1024 * 1024)

// Base64 decoding table
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Structure to hold decoded data
typedef struct {
    uint8_t* data;
    size_t size;
} DecodedData;

// Function to find character position in base64 table
static int base64_char_value(char c) {
    const char* pos = strchr(base64_chars, c);
    if (pos == NULL) {
        return -1;
    }
    return (int)(pos - base64_chars);
}

// Validate if character is valid base64
static bool is_base64_char(unsigned char c) {
    return (isalnum(c) || c == '+' || c == '/');
}

// Validate UTF-8 encoding
static bool is_valid_utf8(const uint8_t* data, size_t size) {
    if (data == NULL) {
        return false;
    }
    
    size_t i = 0;
    while (i < size) {
        if (data[i] <= 0x7F) {
            i++;
        } else if ((data[i] & 0xE0) == 0xC0) {
            if (i + 1 >= size || (data[i+1] & 0xC0) != 0x80) {
                return false;
            }
            i += 2;
        } else if ((data[i] & 0xF0) == 0xE0) {
            if (i + 2 >= size || (data[i+1] & 0xC0) != 0x80 || (data[i+2] & 0xC0) != 0x80) {
                return false;
            }
            i += 3;
        } else if ((data[i] & 0xF8) == 0xF0) {
            if (i + 3 >= size || (data[i+1] & 0xC0) != 0x80 || (data[i+2] & 0xC0) != 0x80 || (data[i+3] & 0xC0) != 0x80) {
                return false;
            }
            i += 4;
        } else {
            return false;
        }
    }
    return true;
}

// Base64 decode function with security checks
static DecodedData* base64_decode(const char* encoded_string) {
    if (encoded_string == NULL) {
        return NULL;
    }
    
    size_t in_len = strlen(encoded_string);
    
    // Validate input length
    if (in_len == 0 || in_len % 4 != 0) {
        return NULL;
    }
    
    // Check maximum size to prevent excessive memory allocation
    if (in_len / 4 * 3 > MAX_DECODE_SIZE) {
        return NULL;
    }
    
    // Allocate decoded data structure
    DecodedData* result = (DecodedData*)calloc(1, sizeof(DecodedData));
    if (result == NULL) {
        return NULL;
    }
    
    // Allocate buffer for decoded data with bounds check
    size_t max_output_size = (in_len / 4) * 3;
    result->data = (uint8_t*)calloc(max_output_size + 1, sizeof(uint8_t));
    if (result->data == NULL) {
        free(result);
        return NULL;
    }
    
    int i = 0;
    int j = 0;
    size_t in_ = 0;
    size_t out_idx = 0;
    unsigned char char_array_4[4];
    unsigned char char_array_3[3];
    
    while (in_len > 0 && encoded_string[in_] != '=') {
        // Validate character is valid base64
        if (!is_base64_char((unsigned char)encoded_string[in_])) {
            free(result->data);
            free(result);
            return NULL;
        }
        
        char_array_4[i++] = encoded_string[in_];
        in_++;
        in_len--;
        
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                int val = base64_char_value(char_array_4[i]);
                if (val < 0) {
                    free(result->data);
                    free(result);
                    return NULL;
                }
                char_array_4[i] = (unsigned char)val;
            }
            
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            // Bounds check before writing
            if (out_idx + 3 > max_output_size) {
                free(result->data);
                free(result);
                return NULL;
            }
            
            for (i = 0; i < 3; i++) {
                result->data[out_idx++] = char_array_3[i];
            }
            i = 0;
        }
    }
    
    if (i) {
        for (j = 0; j < i; j++) {
            int val = base64_char_value(char_array_4[j]);
            if (val < 0) {
                free(result->data);
                free(result);
                return NULL;
            }
            char_array_4[j] = (unsigned char)val;
        }
        
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        
        // Bounds check before writing
        if (out_idx + i - 1 > max_output_size) {
            free(result->data);
            free(result);
            return NULL;
        }
        
        for (j = 0; j < i - 1; j++) {
            result->data[out_idx++] = char_array_3[j];
        }
    }
    
    result->size = out_idx;
    result->data[out_idx] = '\\0'; // Null terminate for safety
    
    return result;
}

// Simple JSON validation
static bool validate_json(const uint8_t* data, size_t size) {
    if (data == NULL || size == 0 || size > 1024 * 1024) {
        return false;
    }
    
    int brace_count = 0;
    int bracket_count = 0;
    bool in_string = false;
    
    for (size_t i = 0; i < size; i++) {
        char c = (char)data[i];
        
        if (c == '"' && (i == 0 || data[i-1] != '\\\\')) {\n            in_string = !in_string;\n        }\n        \n        if (!in_string) {\n            if (c == '{') brace_count++;\n            else if (c == '}') brace_count--;\n            else if (c == '[') bracket_count++;\n            else if (c == ']') bracket_count--;\n            \n            if (brace_count < 0 || bracket_count < 0) {\n                return false;\n            }\n        }\n    }\n    \n    return (brace_count == 0 && bracket_count == 0);\n}\n\n// Main request processing function\nstatic char* process_request(const char* raw_data) {\n    if (raw_data == NULL) {\n        return NULL;\n    }\n    \n    // Validate input size\n    size_t input_len = strlen(raw_data);\n    if (input_len == 0 || input_len > MAX_INPUT_SIZE) {\n        return NULL;\n    }\n    \n    // Validate characters in raw_data\n    for (size_t i = 0; i < input_len; i++) {\n        unsigned char c = (unsigned char)raw_data[i];\n        if (!(isalnum(c) || c == '+' || c == '/' || c == '=' || \n              c == '\
' || c == '\\r' || c == ' ')) {\n            return NULL;\n        }\n    }\n    \n    // Remove whitespace characters with bounds checking\n    char* cleaned_data = (char*)calloc(input_len + 1, sizeof(char));\n    if (cleaned_data == NULL) {\n        return NULL;\n    }\n    \n    size_t cleaned_idx = 0;\n    for (size_t i = 0; i < input_len; i++) {\n        if (raw_data[i] != '\
' && raw_data[i] != '\\r' && raw_data[i] != ' ') {\n            // Bounds check\n            if (cleaned_idx >= input_len) {\n                free(cleaned_data);\n                return NULL;\n            }\n            cleaned_data[cleaned_idx++] = raw_data[i];\n        }\n    }\n    cleaned_data[cleaned_idx] = '\\0';\n    \n    // Decode base64\n    DecodedData* decoded = base64_decode(cleaned_data);\n    free(cleaned_data);\n    \n    if (decoded == NULL) {\n        return NULL;\n    }\n    \n    // Validate UTF-8 encoding\n    if (!is_valid_utf8(decoded->data, decoded->size)) {\n        free(decoded->data);\n        free(decoded);\n        return NULL;\n    }\n    \n    // Validate JSON structure\n    if (!validate_json(decoded->data, decoded->size)) {\n        free(decoded->data);\n        free(decoded);\n        return NULL;\n    }\n    \n    // Create result string with bounds check\n    char* result = (char*)calloc(decoded->size + 1, sizeof(char));\n    if (result == NULL) {\n        free(decoded->data);\n        free(decoded);\n        return NULL;\n    }\n    \n    // Copy with bounds check\n    if (decoded->size > 0) {\n        memcpy(result, decoded->data, decoded->size);\n    }\n    result[decoded->size] = '\\0';
    
    // Clean up decoded data
    free(decoded->data);
    free(decoded);
    
    return result;
}

int main(void) {
    // Test case 1: Valid JSON object
    printf("Test 1: Valid JSON object\\n");
    const char* test1 = "eyJuYW1lIjoiSm9obiIsImFnZSI6MzB9";
    char* result1 = process_request(test1);
    if (result1 != NULL) {
        printf("Result: %s\\n", result1);
        free(result1);
    } else {
        printf("Error processing request\\n");
    }
    
    // Test case 2: Valid JSON array
    printf("\\nTest 2: Valid JSON array\\n");
    const char* test2 = "WyJhcHBsZSIsImJhbmFuYSIsImNoZXJyeSJd";
    char* result2 = process_request(test2);
    if (result2 != NULL) {
        printf("Result: %s\\n", result2);
        free(result2);
    } else {
        printf("Error processing request\\n");
    }
    
    // Test case 3: Simple string
    printf("\\nTest 3: Simple string\\n");
    const char* test3 = "ImhlbGxvIHdvcmxkIg==";
    char* result3 = process_request(test3);
    if (result3 != NULL) {
        printf("Result: %s\\n", result3);
        free(result3);
    } else {
        printf("Error processing request\\n");
    }
    
    // Test case 4: Empty input (should fail)
    printf("\\nTest 4: Empty input\\n");
    const char* test4 = "";
    char* result4 = process_request(test4);
    if (result4 != NULL) {
        printf("Result: %s\\n", result4);
        free(result4);
    } else {
        printf("Error processing request (expected)\\n");
    }
    
    // Test case 5: Invalid base64 (should fail)
    printf("\\nTest 5: Invalid base64\\n");
    const char* test5 = "Invalid!@#$%";
    char* result5 = process_request(test5);
    if (result5 != NULL) {
        printf("Result: %s\\n", result5);
        free(result5);
    } else {
        printf("Error processing request (expected)\\n");
    }
    
    return 0;
}

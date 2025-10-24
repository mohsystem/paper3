
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_DATA_SIZE (1024 * 1024)

// Base64 decoding table
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Find character in base64 table
static int base64_char_value(char c) {
    const char* ptr = strchr(base64_table, c);
    if (ptr) {
        return ptr - base64_table;
    }
    return -1;
}

// Base64 decode function
static char* base64_decode(const char* input, size_t* output_length) {
    if (!input || !output_length) {
        return NULL;
    }
    
    size_t input_length = strlen(input);
    if (input_length % 4 != 0) {
        fprintf(stderr, "Invalid Base64 string length\\n");
        return NULL;
    }
    
    size_t padding = 0;
    if (input_length >= 2) {
        if (input[input_length - 1] == '=') padding++;
        if (input[input_length - 2] == '=') padding++;
    }
    
    *output_length = (input_length / 4) * 3 - padding;
    char* output = (char*)malloc(*output_length + 1);
    if (!output) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    size_t out_idx = 0;
    for (size_t i = 0; i < input_length; i += 4) {
        int val1 = base64_char_value(input[i]);
        int val2 = base64_char_value(input[i + 1]);
        int val3 = (i + 2 < input_length && input[i + 2] != '=') ? base64_char_value(input[i + 2]) : 0;
        int val4 = (i + 3 < input_length && input[i + 3] != '=') ? base64_char_value(input[i + 3]) : 0;
        
        if (val1 < 0 || val2 < 0 || (val3 < 0 && input[i + 2] != '=') || (val4 < 0 && input[i + 3] != '=')) {
            fprintf(stderr, "Invalid Base64 character\\n");
            free(output);
            return NULL;
        }
        
        output[out_idx++] = (val1 << 2) | (val2 >> 4);
        if (i + 2 < input_length && input[i + 2] != '=') {
            output[out_idx++] = (val2 << 4) | (val3 >> 2);
        }
        if (i + 3 < input_length && input[i + 3] != '=') {
            output[out_idx++] = (val3 << 6) | val4;
        }
    }
    
    output[*output_length] = '\\0';
    return output;
}

// Base64 encode function for testing
static char* base64_encode(const char* input, size_t input_length) {
    if (!input) return NULL;
    
    size_t output_length = 4 * ((input_length + 2) / 3);
    char* output = (char*)malloc(output_length + 1);
    if (!output) return NULL;
    
    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? (unsigned char)input[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)input[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)input[i++] : 0;
        
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        output[j++] = base64_table[(triple >> 18) & 0x3F];
        output[j++] = base64_table[(triple >> 12) & 0x3F];
        output[j++] = base64_table[(triple >> 6) & 0x3F];
        output[j++] = base64_table[triple & 0x3F];
    }
    
    int padding = (3 - input_length % 3) % 3;
    for (int k = 0; k < padding; k++) {
        output[output_length - 1 - k] = '=';
    }
    
    output[output_length] = '\\0';
    return output;
}

// Simple JSON validation
static int is_valid_json(const char* str) {
    if (!str) return 0;
    
    // Trim whitespace
    while (isspace(*str)) str++;
    
    size_t len = strlen(str);
    if (len == 0) return 0;
    
    // Remove trailing whitespace
    while (len > 0 && isspace(str[len - 1])) len--;
    
    // Check for basic JSON structure
    if ((str[0] == '{' && str[len - 1] == '}') ||
        (str[0] == '[' && str[len - 1] == ']')) {
        return 1;
    }
    return 0;
}

// Main processing function
char* process_request(const char* raw_data) {
    if (!raw_data || strlen(raw_data) == 0) {
        fprintf(stderr, "Raw data cannot be null or empty\\n");
        return NULL;
    }
    
    // Validate data size to prevent DoS attacks
    if (strlen(raw_data) > MAX_DATA_SIZE) {
        fprintf(stderr, "Data exceeds maximum allowed size\\n");
        return NULL;
    }
    
    // Decode Base64
    size_t decoded_length;
    char* decoded_data = base64_decode(raw_data, &decoded_length);
    if (!decoded_data) {
        fprintf(stderr, "Base64 decoding failed\\n");
        return NULL;
    }
    
    // Validate decoded size
    if (decoded_length > MAX_DATA_SIZE) {
        fprintf(stderr, "Decoded data exceeds maximum allowed size\\n");
        free(decoded_data);
        return NULL;
    }
    
    // Validate JSON structure
    if (!is_valid_json(decoded_data)) {
        fprintf(stderr, "Invalid JSON format\\n");
        free(decoded_data);
        return NULL;
    }
    
    // Return deserialized data
    return decoded_data;
}

int main() {
    printf("Testing Task63 - Secure Request Processing\\n\\n");
    
    // Test Case 1: Valid JSON object
    printf("Test 1 - Valid JSON:\\n");
    char* test1_encoded = base64_encode("{\\"name\\":\\"John\\",\\"age\\":30}", 25);
    if (test1_encoded) {
        char* result1 = process_request(test1_encoded);
        if (result1) {
            printf("Result: %s\\n", result1);
            free(result1);
        }
        free(test1_encoded);
    }
    
    // Test Case 2: Valid JSON array
    printf("\\nTest 2 - Valid JSON Array:\\n");
    char* test2_encoded = base64_encode("[\\"item1\\",\\"item2\\",\\"item3\\"]", 27);
    if (test2_encoded) {
        char* result2 = process_request(test2_encoded);
        if (result2) {
            printf("Result: %s\\n", result2);
            free(result2);
        }
        free(test2_encoded);
    }
    
    // Test Case 3: Nested JSON
    printf("\\nTest 3 - Nested JSON:\\n");
    char* test3_encoded = base64_encode("{\\"user\\":{\\"id\\":123,\\"status\\":\\"active\\"}}", 38);
    if (test3_encoded) {
        char* result3 = process_request(test3_encoded);
        if (result3) {
            printf("Result: %s\\n", result3);
            free(result3);
        }
        free(test3_encoded);
    }
    
    // Test Case 4: Invalid Base64
    printf("\\nTest 4 - Invalid Base64:\\n");
    char* result4 = process_request("Invalid@Base64!Data");
    if (result4) {
        printf("Result: %s\\n", result4);
        free(result4);
    } else {
        printf("Failed (Expected)\\n");
    }
    
    // Test Case 5: Invalid JSON
    printf("\\nTest 5 - Invalid JSON:\\n");
    char* test5_encoded = base64_encode("{invalid json}", 14);
    if (test5_encoded) {
        char* result5 = process_request(test5_encoded);
        if (result5) {
            printf("Result: %s\\n", result5);
            free(result5);
        } else {
            printf("Failed (Expected)\\n");
        }
        free(test5_encoded);
    }
    
    return 0;
}

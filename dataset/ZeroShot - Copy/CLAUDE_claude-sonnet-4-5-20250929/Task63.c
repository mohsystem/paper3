
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple Base64 decoder
char* base64_decode(const char* encoded, size_t* out_len) {
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t len = strlen(encoded);
    size_t decoded_len = len * 3 / 4;
    char* decoded = (char*)malloc(decoded_len + 1);
    
    if (!decoded) return NULL;
    
    int val = 0, valb = -8;
    size_t pos = 0;
    
    for (size_t i = 0; i < len; i++) {
        char c = encoded[i];
        if (c == '=') break;
        
        const char* p = strchr(base64_chars, c);
        if (!p) continue;
        
        val = (val << 6) + (p - base64_chars);
        valb += 6;
        
        if (valb >= 0) {
            decoded[pos++] = (val >> valb) & 0xFF;
            valb -= 8;
        }
    }
    
    decoded[pos] = '\\0';
    *out_len = pos;
    return decoded;
}

// Simple Base64 encoder for testing
char* base64_encode(const char* input, size_t len) {
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t encoded_len = 4 * ((len + 2) / 3);
    char* encoded = (char*)malloc(encoded_len + 1);
    
    if (!encoded) return NULL;
    
    int val = 0, valb = -6;
    size_t pos = 0;
    
    for (size_t i = 0; i < len; i++) {
        val = (val << 8) + (unsigned char)input[i];
        valb += 8;
        while (valb >= 0) {
            encoded[pos++] = base64_chars[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    
    if (valb > -6) encoded[pos++] = base64_chars[((val << 8) >> (valb + 8)) & 0x3F];
    while (pos % 4) encoded[pos++] = '=';
    
    encoded[pos] = '\\0';
    return encoded;
}

char* processRequest(const char* rawData) {
    size_t decoded_len;
    char* decodedData = base64_decode(rawData, &decoded_len);
    
    if (!decodedData) {
        fprintf(stderr, "Error: Failed to decode Base64\\n");
        return NULL;
    }
    
    // Return the decoded data (JSON parsing would require external library)
    return decodedData;
}

int main() {
    // Test Case 1: Simple JSON object
    char* test1 = base64_encode("{\\"name\\":\\"John\\",\\"age\\":30}", 24);
    char* result1 = processRequest(test1);
    printf("Test 1: %s\\n", result1 ? result1 : "NULL");
    free(test1);
    free(result1);
    
    // Test Case 2: JSON array
    char* test2 = base64_encode("[1,2,3,4,5]", 11);
    char* result2 = processRequest(test2);
    printf("Test 2: %s\\n", result2 ? result2 : "NULL");
    free(test2);
    free(result2);
    
    // Test Case 3: Nested JSON
    char* test3 = base64_encode("{\\"user\\":{\\"id\\":1,\\"name\\":\\"Alice\\"}}", 33);
    char* result3 = processRequest(test3);
    printf("Test 3: %s\\n", result3 ? result3 : "NULL");
    free(test3);
    free(result3);
    
    // Test Case 4: JSON with special characters
    char* test4 = base64_encode("{\\"message\\":\\"Hello, World!\\"}", 28);
    char* result4 = processRequest(test4);
    printf("Test 4: %s\\n", result4 ? result4 : "NULL");
    free(test4);
    free(result4);
    
    // Test Case 5: Empty JSON object
    char* test5 = base64_encode("{}", 2);
    char* result5 = processRequest(test5);
    printf("Test 5: %s\\n", result5 ? result5 : "NULL");
    free(test5);
    free(result5);
    
    return 0;
}

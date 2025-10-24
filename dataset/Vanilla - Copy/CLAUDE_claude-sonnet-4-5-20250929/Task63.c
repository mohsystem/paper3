
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple Base64 decoding implementation
char* base64_decode(const char* encoded_string, size_t* output_length) {
    const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    int vec[256];
    memset(vec, -1, sizeof(vec));
    for (int i = 0; i < 64; i++)
        vec[(unsigned char)base64_chars[i]] = i;
    
    size_t len = strlen(encoded_string);
    char* decoded = (char*)malloc(len);
    int val = 0, valb = -8, idx = 0;
    
    for (size_t i = 0; i < len; i++) {
        unsigned char c = encoded_string[i];
        if (vec[c] == -1) break;
        val = (val << 6) + vec[c];
        valb += 6;
        if (valb >= 0) {
            decoded[idx++] = (char)((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    decoded[idx] = '\\0';
    *output_length = idx;
    return decoded;
}

char* processRequest(const char* rawData) {
    // Step 1: Ensure raw_data is in UTF-8 (assuming input is already UTF-8)
    
    // Step 2: Decode the raw_data using Base64
    size_t decoded_length;
    char* decodedData = base64_decode(rawData, &decoded_length);
    
    // Step 3 & 4: Deserialize and store the decoded data
    // For simplicity, returning the decoded JSON string as-is
    
    // Step 5: Return the stored data
    return decodedData;
}

// Helper function to encode for testing
char* base64_encode(const char* input) {
    const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    size_t len = strlen(input);
    size_t encoded_len = ((len + 2) / 3) * 4;
    char* encoded = (char*)malloc(encoded_len + 1);
    
    int val = 0, valb = -6, idx = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = input[i];
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded[idx++] = base64_chars[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    if (valb > -6) encoded[idx++] = base64_chars[((val << 8) >> (valb + 8)) & 0x3F];
    while (idx % 4) encoded[idx++] = '=';
    encoded[idx] = '\\0';
    return encoded;
}

int main() {
    // Test case 1: Simple JSON object
    char* test1 = base64_encode("{\\"name\\":\\"John\\",\\"age\\":30}");
    char* result1 = processRequest(test1);
    printf("Test 1: %s\\n", result1);
    free(test1);
    free(result1);
    
    // Test case 2: JSON with nested object
    char* test2 = base64_encode("{\\"user\\":{\\"id\\":1,\\"name\\":\\"Alice\\"}}");
    char* result2 = processRequest(test2);
    printf("Test 2: %s\\n", result2);
    free(test2);
    free(result2);
    
    // Test case 3: JSON with array
    char* test3 = base64_encode("{\\"items\\":[1,2,3,4,5]}");
    char* result3 = processRequest(test3);
    printf("Test 3: %s\\n", result3);
    free(test3);
    free(result3);
    
    // Test case 4: JSON with boolean and null
    char* test4 = base64_encode("{\\"active\\":true,\\"data\\":null}");
    char* result4 = processRequest(test4);
    printf("Test 4: %s\\n", result4);
    free(test4);
    free(result4);
    
    // Test case 5: Complex JSON
    char* test5 = base64_encode("{\\"company\\":\\"TechCorp\\",\\"employees\\":[{\\"name\\":\\"Bob\\",\\"role\\":\\"Developer\\"}]}");
    char* result5 = processRequest(test5);
    printf("Test 5: %s\\n", result5);
    free(test5);
    free(result5);
    
    return 0;
}

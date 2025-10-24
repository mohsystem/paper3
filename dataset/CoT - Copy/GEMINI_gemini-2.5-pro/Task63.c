#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Base64 decoding function (public domain) ---
// Source: https://stackoverflow.com/a/3424844/1332015
// Caller is responsible for freeing the returned memory.
char *base64_decode(const char *data, size_t input_length, size_t *output_length) {
    // Build the decoding table
    static char decoding_table[256];
    if (decoding_table[0] == 0) { // Build only once
        memset(decoding_table, 0, 256);
        for (int i = 0; i < 64; i++)
            decoding_table[(unsigned char)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
    }
    
    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (input_length > 0 && data[input_length - 1] == '=') (*output_length)--;
    if (input_length > 1 && data[input_length - 2] == '=') (*output_length)--;

    char *decoded_data = (char*)malloc(*output_length + 1);
    if (decoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        int sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        int sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        int sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        int sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];

        unsigned int triple = (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6) | sextet_d;

        if (j < *output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = triple & 0xFF;
    }
    
    decoded_data[*output_length] = '\0';
    return decoded_data;
}
// --- End of Base64 decoding function ---

/**
 * @brief Processes a request by decoding a Base64 encoded UTF-8 string.
 *
 * This function returns a dynamically allocated string containing the decoded data.
 * It is the RESPONSIBILITY OF THE CALLER to free this memory using free().
 *
 * @param base64Utf8Data The Base64 encoded string.
 * @return A pointer to the null-terminated decoded string, or NULL on failure.
 */
char* processRequest(const char* base64Utf8Data) {
    if (base64Utf8Data == NULL) {
        return NULL;
    }
    
    size_t input_len = strlen(base64Utf8Data);
    if (input_len == 0) {
        char* empty_str = (char*)malloc(1);
        if (empty_str) empty_str[0] = '\0';
        return empty_str;
    }
    
    // Basic validation: Base64 length must be a multiple of 4.
    if (input_len % 4 != 0) {
        fprintf(stderr, "Error: Invalid Base64 string length.\n");
        return NULL;
    }
    
    size_t output_len = 0;
    char* decoded_string = base64_decode(base64Utf8Data, input_len, &output_len);
    
    if (decoded_string == NULL) {
        fprintf(stderr, "Error: Base64 decoding failed. Check input for invalid characters or memory issues.\n");
        return NULL;
    }
    
    return decoded_string;
}

void run_test(int test_num, const char* test_case) {
    printf("--- Test Case %d ---\n", test_num);
    printf("Input: %s\n", test_case);
    char *result = processRequest(test_case);
    if (result != NULL) {
        // In a real application, 'result' would be passed to a JSON parser (e.g., cJSON).
        printf("Output: %s\n", result);
        free(result); // IMPORTANT: Free the memory allocated by processRequest
    } else {
        printf("Output: Processing failed.\n");
    }
    printf("\n");
}

int main() {
    // Test Case 1: Simple valid JSON
    run_test(1, "eyJpZCI6IDEyMywgInVzZXIiOiAidGVzdCJ9");
    // Test Case 2: More complex JSON
    run_test(2, "eyJuYW1lIjogIkJvYiIsICJhY3RpdmUiOiB0cnVlLCAicm9sZXMiOiBbImFkbWluIiwgImVkaXRvciJdfQ==");
    // Test Case 3: Empty JSON object
    run_test(3, "e30=");
    // Test Case 4: Invalid Base64 string (will fail length check)
    run_test(4, "invalid-base64@#$");
    // Test Case 5: Valid Base64, but not valid JSON
    run_test(5, "VGhpcyBpcyBub3QgSlNPTg==");

    return 0;
}
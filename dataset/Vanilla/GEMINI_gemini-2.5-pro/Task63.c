#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For building the lookup table
static char decoding_table[256];
static int is_table_built = 0;

void build_decoding_table() {
    if (is_table_built) return;

    const char* b64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (int i = 0; i < 256; i++) decoding_table[i] = -1;
    for (int i = 0; i < 64; i++) decoding_table[(unsigned char)b64_chars[i]] = i;
    
    is_table_built = 1;
}

// Self-contained Base64 decoding function
char* base64_decode(const char* data, size_t input_length, size_t* output_length) {
    build_decoding_table();

    // The input length must be a multiple of 4.
    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (input_length > 0 && data[input_length - 1] == '=') (*output_length)--;
    if (input_length > 1 && data[input_length - 2] == '=') (*output_length)--;

    char* decoded_data = (char*)malloc(*output_length + 1);
    if (decoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        unsigned int sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        unsigned int sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        unsigned int sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        unsigned int sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];

        unsigned int triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

        if (j < *output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = triple & 0xFF;
    }
    
    decoded_data[*output_length] = '\0';
    return decoded_data;
}


/**
 * Processes a raw request by Base64 decoding the data.
 *
 * @param rawData The Base64 encoded, UTF-8 string data.
 * @return A dynamically allocated string with the deserialized data (JSON string).
 *         The caller is responsible for freeing this memory.
 *         For full deserialization into a C struct, a library like Jansson would be used.
 */
char* processRequest(const char* rawData) {
    // Step 1: Retrieve raw_data (passed as parameter)
    size_t input_len = strlen(rawData);
    size_t output_len;
    
    // Step 2 & 3: Decode from Base64 and deserialize (to a C string).
    char* deserializedData = base64_decode(rawData, input_len, &output_len);

    // Step 4 & 5: Store in a variable and return.
    return deserializedData;
}

// C does not have classes, so "Task63" is used to describe the purpose of the file.
// The main function and test runner are provided globally.
void runTests() {
    const char* testCases[] = {
        "eyJpZCI6IDEyMywgIm5hbWUiOiAidGVzdF91c2VyIiwgImFjdGl2ZSI6IHRydWV9",
        "eyJtZXNzYWdlIjogIkhlbGxvLCBXb3JsZCEifQ==",
        "eyJkYXRhIjogWzEsIDIsIDNdLCAic3RhdHVzIjogIm9rIn0=",
        "eyJlcnJvciI6ICJOb3QgRm91bmQiLCAiY29kZSI6IDQwNH0=",
        "e30="
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < numTestCases; i++) {
        printf("Test Case %d:\n", i + 1);
        const char* rawData = testCases[i];
        printf("Input (Base64): %s\n", rawData);
        
        char* result = processRequest(rawData);
        if (result) {
            printf("Output (Deserialized): %s\n", result);
            free(result); // IMPORTANT: Free the allocated memory
        } else {
            printf("Decoding failed.\n");
        }
        printf("\n");
    }
}

int main() {
    runTests();
    return 0;
}
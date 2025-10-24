#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// --- BEGIN 3rd Party Library: cJSON (cJSON.h) ---
// Copyright (c) 2009-2017 Dave Gamble and cJSON contributors.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
typedef struct cJSON {
    struct cJSON *next, *prev;
    struct cJSON *child;
    int type;
    char *valuestring;
    int valueint;
    double valuedouble;
    char *string;
} cJSON;
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7)
cJSON *cJSON_Parse(const char *value);
void   cJSON_Delete(cJSON *c);
char  *cJSON_Print(const cJSON *item);
const char *cJSON_GetErrorPtr(void);
// --- (cJSON.c implementation would go here, but is omitted for brevity) ---
// NOTE: For this code to compile, the full implementation of cJSON.c is required.
// For demonstration, mock functions are provided below.
const char *cJSON_error_ptr = NULL;
cJSON *cJSON_Parse(const char *value) {
    if (value == NULL || *value != '{') { cJSON_error_ptr = "Invalid format"; return NULL; }
    cJSON *item = (cJSON*)malloc(sizeof(cJSON));
    if(!item) return NULL;
    item->valuestring = (char*)malloc(strlen(value) + 1);
    if(!item->valuestring) { free(item); return NULL; }
    strcpy(item->valuestring, value);
    return item;
}
void cJSON_Delete(cJSON *c) { if(!c) return; free(c->valuestring); free(c); }
char *cJSON_Print(const cJSON *item) {
    if(!item || !item->valuestring) return NULL;
    char *p = (char*)malloc(strlen(item->valuestring) + 1);
    if(!p) return NULL;
    strcpy(p, item->valuestring);
    return p;
}
const char *cJSON_GetErrorPtr(void) { return cJSON_error_ptr; }


// --- BEGIN 3rd Party Library: Base64 ---
// Public Domain Base64 decoder
// A simple, self-contained implementation.
#include <ctype.h>
unsigned char* base64_decode(const char *data, size_t input_length, size_t *output_length) {
    static char decoding_table[] = {
        62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1,
        -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1,
        -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
        39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };
    if (input_length % 4 != 0) return NULL;
    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;
    unsigned char *decoded_data = (unsigned char*)malloc(*output_length + 1);
    if (decoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {
        unsigned int sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++] - 43];
        unsigned int sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++] - 43];
        unsigned int sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++] - 43];
        unsigned int sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++] - 43];
        unsigned int triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;
        if (j < *output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = triple & 0xFF;
    }
    decoded_data[*output_length] = '\0';
    return decoded_data;
}
// --- END 3rd Party Library: Base64 ---

/**
 * @brief Processes a raw request by Base64 decoding and JSON deserializing the data.
 * 
 * @param rawData The Base64 encoded, UTF-8 string from the request.
 * @return A pointer to a cJSON object. The caller is responsible for freeing this
 *         memory using cJSON_Delete(). Returns NULL on error.
 */
cJSON* processRequest(const char* rawData) {
    if (rawData == NULL || *rawData == '\0') {
        fprintf(stderr, "Error: Input data is null or empty.\n");
        return NULL;
    }
    
    size_t rawDataLen = strlen(rawData);
    if (rawDataLen % 4 != 0) {
        fprintf(stderr, "Error: Invalid Base64 input length.\n");
        return NULL;
    }

    // Step 1 & 2: Decode Base64 string.
    size_t decoded_len;
    unsigned char *decoded_data = base64_decode(rawData, rawDataLen, &decoded_len);

    if (decoded_data == NULL) {
        fprintf(stderr, "Error: Invalid Base64 input or memory allocation failed.\n");
        return NULL;
    }

    // Step 3: Deserialize the decoded data using a safe JSON parser.
    // SECURITY: cJSON is a C-based parser, which is inherently safer than
    // object-oriented deserializers that can instantiate arbitrary classes.
    // It just parses data structures and does not execute code.
    cJSON *deserialized_json = cJSON_Parse((const char*)decoded_data);
    
    // Free the intermediate decoded string.
    free(decoded_data);

    if (deserialized_json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error: Failed to deserialize JSON. Details: %s\n", error_ptr);
        } else {
            fprintf(stderr, "Error: Failed to deserialize JSON. The decoded data is not valid JSON.\n");
        }
        return NULL;
    }

    // Step 4 & 5: Return the stored data.
    return deserialized_json;
}

int main() {
    const char* testCases[] = {
        // Test Case 1: Valid JSON data
        "eyJ1c2VyIjogImFkbWluIiwgImlkIjogMTIzfQ==", // Decodes to: {"user": "admin", "id": 123}
        // Test Case 2: Another valid JSON
        "eyJrZXkiOiAidmFsdWUiLCAibnVtYmVyIjogNDJ9", // Decodes to: {"key": "value", "number": 42}
        // Test Case 3: Invalid Base64 string (bad length)
        "invalid-base64-string",
        // Test Case 4: Valid Base64, but the content is not JSON
        "SGVsbG8gV29ybGQ=", // Decodes to: "Hello World"
        // Test Case 5: Empty string
        ""
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("--- Test Case %d ---\n", i + 1);
        printf("Input: %s\n", testCases[i]);
        cJSON* result = processRequest(testCases[i]);
        if (result != NULL) {
            // cJSON_Print allocates memory for the string.
            char* json_string = cJSON_Print(result);
            if (json_string != NULL) {
                printf("Processed Data: %s\n", json_string);
                free(json_string);
            }
            // Free the cJSON object returned by processRequest.
            cJSON_Delete(result);
        } else {
            printf("Processing failed.\n");
        }
        printf("\n");
    }

    return 0;
}
/*
 * This code simulates the backend logic of a web route that processes a JSON payload.
 * It is designed to be run as a single file with its dependency.
 *
 * Dependency: cJSON. A single-file JSON parser library in C.
 * 1. Get cJSON.c and cJSON.h from https://github.com/DaveGamble/cJSON
 * 2. Place them in the same directory as this file.
 * 3. Compile everything together:
 *    gcc your_file_name.c cJSON.c -o your_program -lm
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cJSON.h"

// Helper function to convert a hexadecimal character to its integer value
static int hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1; // Invalid hex character
}

/**
 * Decodes a URL-encoded string.
 * @param src The URL-encoded source string.
 * @return A new, dynamically allocated string that has been decoded.
 *         The caller is responsible for freeing this memory. Returns NULL on failure.
 */
static char* url_decode(const char* src) {
    if (src == NULL) return NULL;
    size_t src_len = strlen(src);
    char* decoded = (char*)malloc(src_len + 1);
    if (decoded == NULL) return NULL;

    char* p = decoded;
    for (size_t i = 0; i < src_len; ++i) {
        if (src[i] == '%' && i + 2 < src_len) {
            int d1 = hex_to_int(src[i + 1]);
            int d2 = hex_to_int(src[i + 2]);
            if (d1 != -1 && d2 != -1) {
                *p++ = (char)((d1 << 4) | d2);
                i += 2;
            } else {
                *p++ = src[i];
            }
        } else if (src[i] == '+') {
            *p++ = ' ';
        } else {
            *p++ = src[i];
        }
    }
    *p = '\0';
    return decoded;
}

// Helper to create a dynamically allocated string for error messages or results.
static char* alloc_and_copy_string(const char* str) {
    if (str == NULL) return NULL;
    char* new_str = (char*)malloc(strlen(str) + 1);
    if (new_str) {
        strcpy(new_str, str);
    }
    return new_str;
}

/**
 * Simulates processing a request to an /api endpoint. It extracts a 'payload'
 * parameter from the query string, parses it as JSON, and returns the value 
 * associated with the 'name' key.
 *
 * @param queryString The query string from the URL (e.g., "payload=%7B...%7D").
 * @return A dynamically allocated string with the result or an error message.
 *         The caller MUST free the returned string. Returns NULL on allocation failure.
 */
char* processApiRequest(const char* queryString) {
    if (queryString == NULL) {
        return alloc_and_copy_string("Error: 'payload' parameter not found.");
    }
    
    const char* payload_key = "payload=";
    const char* payload_start = strstr(queryString, payload_key);
    if (payload_start == NULL) {
        return alloc_and_copy_string("Error: 'payload' parameter not found.");
    }
    payload_start += strlen(payload_key);
    
    const char* payload_end = strchr(payload_start, '&');
    size_t payload_len = (payload_end != NULL) ? (payload_end - payload_start) : strlen(payload_start);
    
    if (payload_len == 0) {
        return alloc_and_copy_string("Error: 'payload' parameter is empty.");
    }

    char* payload_encoded = (char*)malloc(payload_len + 1);
    if (!payload_encoded) return NULL;
    strncpy(payload_encoded, payload_start, payload_len);
    payload_encoded[payload_len] = '\0';

    char* payload_decoded = url_decode(payload_encoded);
    free(payload_encoded);
    if (!payload_decoded) return NULL;

    char* result_str = NULL;
    cJSON* json = cJSON_Parse(payload_decoded);
    if (json == NULL) {
        result_str = alloc_and_copy_string("Error: Malformed JSON in payload.");
    } else {
        if (!cJSON_IsObject(json)) {
             result_str = alloc_and_copy_string("Error: Payload is not a JSON object.");
        } else {
            const cJSON* name_item = cJSON_GetObjectItemCaseSensitive(json, "name");
            if (name_item == NULL) {
                result_str = alloc_and_copy_string("Error: 'name' key not found in payload.");
            } else if (!cJSON_IsString(name_item)) {
                result_str = alloc_and_copy_string("Error: 'name' value is not a string.");
            } else {
                result_str = alloc_and_copy_string(name_item->valuestring);
            }
        }
        cJSON_Delete(json);
    }
    
    free(payload_decoded);
    return result_str;
}

int main() {
    const char* testCases[] = {
        // 1. Valid case with a 'name' key
        "payload=%7B%22name%22%3A%22Alice%22%2C%20%22id%22%3A123%7D",
        // 2. Valid JSON but missing the 'name' key
        "payload=%7B%22id%22%3A456%7D",
        // 3. Malformed JSON (extra comma)
        "payload=%7B%22name%22%3A%22Bob%22%2C%7D",
        // 4. Query string without a 'payload' parameter
        "other_param=abc&another=123",
        // 5. 'name' key exists but its value is not a string
        "payload=%7B%22name%22%3A12345%7D"
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    printf("Running test cases...\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Input: %s\n", testCases[i]);
        char* result = processApiRequest(testCases[i]);
        if (result != NULL) {
            printf("Output: %s\n", result);
            free(result); // IMPORTANT: Free the memory returned by the function
        } else {
            printf("Output: (null) - A memory allocation error likely occurred.\n");
        }
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Processes a JSON-like string payload to extract the value of the "name" field.
 *
 * This is a basic, manual parser for a simple, expected format. It is not a
 * robust, general-purpose JSON parser. It is designed to be memory-safe
 * for its limited scope.
 *
 * @param payload The null-terminated string payload to process. Max length is 1024.
 * @return A dynamically allocated string containing the value of the "name" field.
 *         The caller is responsible for freeing this memory.
 *         Returns NULL if the "name" field is not found, the payload is malformed,
 *         or a memory allocation error occurs.
 */
char* process_payload(const char* payload) {
    if (payload == NULL) {
        return NULL;
    }

    // Set a maximum payload size to prevent excessive processing
    const size_t MAX_PAYLOAD_LEN = 1024;
    if (strnlen(payload, MAX_PAYLOAD_LEN + 1) > MAX_PAYLOAD_LEN) {
        fprintf(stderr, "Error: Payload exceeds maximum length of %zu.\n", MAX_PAYLOAD_LEN);
        return NULL;
    }

    const char* key_to_find = "\"name\"";
    const char* key_ptr = strstr(payload, key_to_find);

    if (key_ptr == NULL) {
        return NULL; // Key not found
    }

    const char* colon_ptr = strchr(key_ptr + strlen(key_to_find), ':');
    if (colon_ptr == NULL) {
        return NULL; // Malformed, no colon after key
    }

    const char* value_start_quote = strchr(colon_ptr, '"');
    if (value_start_quote == NULL) {
        return NULL; // Malformed, no opening quote for value
    }

    const char* value_end_quote = strchr(value_start_quote + 1, '"');
    if (value_end_quote == NULL) {
        return NULL; // Malformed, no closing quote for value
    }

    // Pointer to the first character of the actual value
    const char* value_start = value_start_quote + 1;

    // Calculate the length of the value
    size_t value_len = value_end_quote - value_start;
    if (value_len >= MAX_PAYLOAD_LEN) { // Sanity check on value length
        return NULL;
    }

    // Allocate memory for the result (+1 for null terminator)
    char* result = (char*)malloc(value_len + 1);
    if (result == NULL) {
        perror("Failed to allocate memory for result");
        return NULL; // Memory allocation failed
    }

    // Safely copy the value string
    memcpy(result, value_start, value_len);
    result[value_len] = '\0'; // Ensure null termination

    return result;
}

void run_test_case(int case_num, const char* payload, const char* expected) {
    printf("Test Case %d: Payload='%s'\n", case_num, payload);
    char* result = process_payload(payload);
    
    int is_correct = 0;
    if (expected == NULL && result == NULL) {
        is_correct = 1;
    } else if (expected != NULL && result != NULL && strcmp(result, expected) == 0) {
        is_correct = 1;
    }

    printf("  -> Expected: %s, Got: %s. Correct: %s\n", 
           expected ? expected : "null", 
           result ? result : "null", 
           is_correct ? "true" : "false");

    // IMPORTANT: Free the memory allocated by process_payload
    if (result != NULL) {
        free(result);
        result = NULL;
    }
}

int main() {
    printf("--- Running Test Cases ---\n");
    // Note: C does not have a built-in web server. This main function demonstrates
    // the core logic by running test cases against the processing function.

    // Test Case 1: Simple valid case
    run_test_case(1, "{\"name\":\"Alice\"}", "Alice");

    // Test Case 2: Valid case with other fields
    run_test_case(2, "{\"age\":30, \"name\":\"Bob\"}", "Bob");

    // Test Case 3: Valid case with whitespace
    run_test_case(3, "   { \"name\" : \"Charlie\" }   ", "Charlie");

    // Test Case 4: Missing 'name' field
    run_test_case(4, "{\"city\":\"New York\"}", NULL);

    // Test Case 5: Malformed payload
    run_test_case(5, "invalid json", NULL);
    
    printf("--- Test Cases Finished ---\n");

    return 0;
}
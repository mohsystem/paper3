#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Note: Creating a full web application in a single C file without external
// libraries is highly complex. It involves low-level socket programming and
// manual HTTP parsing.
//
// The following code simulates the core logic: processing a payload string
// to find and extract the 'name' field. This function could be integrated
// into a web server built with a library like libmicrohttpd or Mongoose.

/**
 * @brief Parses a URL-encoded string payload to find the value of the 'name' field.
 *
 * @param payload The input string, formatted as "key1=value1&key2=value2...".
 * @return A dynamically allocated string containing the value of the 'name' field.
 *         The caller is responsible for freeing this memory.
 *         Returns NULL if the payload is NULL/empty or the 'name' field is not found.
 */
char* processPayload(const char* payload) {
    if (payload == NULL || *payload == '\0') {
        return NULL;
    }
    
    // Search for "name="
    // We look for "&name=" or the payload starting with "name=" to avoid matching fields like "username=".
    const char* name_key = "name=";
    const char* name_start = strstr(payload, name_key);

    // If we found "name=", ensure it's a valid key start
    if (name_start != NULL && name_start != payload && *(name_start - 1) != '&') {
        const char* temp_search = name_start + 1;
        // Search for the next occurrence which must be preceded by '&'
        while ((name_start = strstr(temp_search, name_key)) != NULL) {
            if (*(name_start - 1) == '&') {
                break; // Found a valid "&name="
            }
            temp_search = name_start + 1;
        }
    }

    if (name_start == NULL) {
        return NULL;
    }

    // The value starts right after "name="
    const char* value_start = name_start + strlen(name_key);
    
    // Find the end of the value, which is the next '&' or the end of the string
    const char* value_end = strchr(value_start, '&');
    
    size_t value_len;
    if (value_end == NULL) {
        // No more '&', value is the rest of the string
        value_len = strlen(value_start);
    } else {
        // Value is the substring between value_start and value_end
        value_len = value_end - value_start;
    }

    // Allocate memory for the result (+1 for null terminator)
    // Using malloc requires the caller to free the memory.
    char* result = (char*)malloc(value_len + 1);
    if (result == NULL) {
        // Memory allocation failed
        perror("Failed to allocate memory");
        return NULL;
    }

    // Safely copy the value into the new buffer
    strncpy(result, value_start, value_len);
    result[value_len] = '\0'; // Null-terminate the string

    return result;
}

void run_test(const char* test_name, const char* payload) {
    printf("%s Payload: %s\n", test_name, payload ? payload : "(null)");
    char* name = processPayload(payload);
    if (name != NULL) {
        printf("%s Result: %s\n", test_name, name);
        free(name); // Free the memory allocated by processPayload
    } else {
        printf("%s Result: Field 'name' not found or payload empty.\n", test_name);
    }
}

int main() {
    printf("--- Running Test Cases ---\n");

    // Test Case 1: 'name' field in the middle
    run_test("Test 1", "id=1&name=Alice&role=user");

    // Test Case 2: 'name' field is missing
    run_test("Test 2", "id=2&role=admin");

    // Test Case 3: Empty payload
    run_test("Test 3", "");
    
    // Test Case 4: 'name' is the first field
    run_test("Test 4", "name=Bob&id=3");
    
    // Test Case 5: 'name' is the last field
    run_test("Test 5", "id=4&role=guest&name=Charlie");

    printf("--------------------------\n");

    return 0;
}
#include <stdio.h>
#include <string.h>

// Define a fixed size for our input buffer.
#define BUFFER_SIZE 128

/**
 * Processes a string by printing it and its length.
 * The function itself assumes 'input' is a valid, null-terminated string.
 * The security must be enforced by the caller when reading data into the buffer.
 * @param input A constant pointer to a character buffer (C-string).
 */
void processString(const char* input) {
    printf("Processed String: %s\n", input);
    printf("Length: %zu\n", strlen(input));
    printf("--------------------\n");
}

/**
 * A helper function to simulate safely reading a test string into a fixed buffer.
 * In a real application, you would use fgets() to read from a user or a file.
 * Here, we use snprintf() to safely copy the test data, which truncates
 * the input if it's too long, thus preventing a buffer overflow.
 * @param test_input The string to be used as test data.
 */
void run_safe_test(const char* test_input) {
    char buffer[BUFFER_SIZE];

    // Safely copy the test string into the buffer.
    // snprintf will write at most BUFFER_SIZE-1 characters and a null terminator.
    snprintf(buffer, BUFFER_SIZE, "%s", test_input);
    
    // Process the (potentially truncated) and safe string in the buffer.
    processString(buffer);
}

int main() {
    printf("--- Running C Test Cases ---\n");

    // Test Case 1: Regular string (fits in buffer)
    run_safe_test("Hello, World!");

    // Test Case 2: Another string (fits in buffer)
    run_safe_test("This is a test string.");

    // Test Case 3: Empty string
    run_safe_test("");

    // Test Case 4: A very long string that will be truncated
    run_safe_test(
        "This is a very long string designed to be larger than the 128-byte buffer. "
        "An unsafe function like strcpy() would cause a buffer overflow, but because "
        "we use snprintf() for this test (or fgets() for real input), the string is "
        "safely truncated, preventing memory corruption."
    );

    // Test Case 5: String with special characters (fits in buffer)
    run_safe_test("!@#$%^&*()_+-=[]{}|;:'\",.<>?`~");

    printf("--- End of Test Cases ---\n");
    return 0;
}
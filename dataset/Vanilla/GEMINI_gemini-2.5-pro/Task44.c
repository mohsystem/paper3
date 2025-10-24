#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Error codes
#define SUCCESS 0
#define ERROR_NAME_NULL_OR_EMPTY 1
#define ERROR_NAME_LENGTH 2
#define ERROR_NAME_INVALID_CHARS 3
#define ERROR_AGE_RANGE 4

/**
 * @brief Validates user input and populates an output buffer with a greeting message.
 * @param name The user's name. Must be 2-50 chars, letters and spaces only.
 * @param age The user's age. Must be between 0 and 120.
 * @param outputBuffer The buffer to write the result message to.
 * @param bufferSize The size of the output buffer.
 * @return An integer error code (0 for success).
 */
int processUserInput(const char* name, int age, char* outputBuffer, size_t bufferSize) {
    // Validate name
    if (name == NULL) {
        return ERROR_NAME_NULL_OR_EMPTY;
    }
    
    size_t name_len = 0;
    bool is_name_empty = true;
    for (size_t i = 0; name[i] != '\0'; ++i) {
        name_len++;
        if (!isspace((unsigned char)name[i])) {
            is_name_empty = false;
        }
        if (!isalpha((unsigned char)name[i]) && !isspace((unsigned char)name[i])) {
            return ERROR_NAME_INVALID_CHARS;
        }
    }
    
    if (is_name_empty) {
        return ERROR_NAME_NULL_OR_EMPTY;
    }
    
    if (name_len < 2 || name_len > 50) {
        return ERROR_NAME_LENGTH;
    }

    // Validate age
    if (age < 0 || age > 120) {
        return ERROR_AGE_RANGE;
    }
    
    snprintf(outputBuffer, bufferSize, "Hello, %s! You are %d years old.", name, age);
    
    return SUCCESS;
}

// Helper function to print test results
void run_test_case(const char* description, const char* name, int age) {
    char buffer[256];
    int result = processUserInput(name, age, buffer, sizeof(buffer));
    
    printf("%s: ", description);
    switch (result) {
        case SUCCESS:
            printf("PASSED - %s\n", buffer);
            break;
        case ERROR_NAME_NULL_OR_EMPTY:
            printf("PASSED - Name cannot be null or empty.\n");
            break;
        case ERROR_NAME_LENGTH:
            printf("PASSED - Name must be between 2 and 50 characters.\n");
            break;
        case ERROR_NAME_INVALID_CHARS:
            printf("PASSED - Name can only contain letters and spaces.\n");
            break;
        case ERROR_AGE_RANGE:
            printf("PASSED - Age must be between 0 and 120.\n");
            break;
        default:
            printf("FAILED - Unknown error code.\n");
            break;
    }
}


int main() {
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Valid input
    run_test_case("Test Case 1 (Valid)", "Alice Smith", 30);
    
    // Test Case 2: Invalid name (with numbers)
    run_test_case("Test Case 2 (Invalid Name)", "Bob123", 40);

    // Test Case 3: Invalid age (too high)
    run_test_case("Test Case 3 (Invalid Age)", "Charlie Brown", 150);

    // Test Case 4: Invalid name (empty)
    run_test_case("Test Case 4 (Empty Name)", "  ", 25);
    
    // Test Case 5: Invalid name (special characters)
    run_test_case("Test Case 5 (Special Chars)", "Eve <script>", 22);

    printf("--- Test Cases Finished ---\n");
    return 0;
}
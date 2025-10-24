#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

/**
 * Validates that a string contains only alphanumeric characters.
 * @param str The string to validate.
 * @return true if the string is alphanumeric, false otherwise.
 */
bool isAlphanumeric(const char* str) {
    if (str == NULL || *str == '\0') {
        return false;
    }
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isalnum((unsigned char)str[i])) {
            return false;
        }
    }
    return true;
}

/**
 * Processes user input after validation.
 * @param username A string representing the username.
 * @param ageStr A string representing the user's age.
 */
void performOperation(const char* username, const char* ageStr) {
    // 1. Validate Username
    if (username == NULL || username[0] == '\0') {
        printf("Error: Username cannot be empty.\n");
        return;
    }
    size_t name_len = strlen(username);
    if (name_len < 3 || name_len > 20) {
        printf("Error: Username must be between 3 and 20 characters long.\n");
        return;
    }
    if (!isAlphanumeric(username)) {
        printf("Error: Username must contain only alphanumeric characters.\n");
        return;
    }

    // 2. Validate Age using strtol for safety
    if (ageStr == NULL || ageStr[0] == '\0') {
        printf("Error: Age must be a valid integer.\n");
        return;
    }
    char* endptr;
    errno = 0; // Reset errno before the call
    long age = strtol(ageStr, &endptr, 10);

    // Check for conversion errors
    if (endptr == ageStr) {
        printf("Error: Age must be a valid integer (no digits found).\n");
        return;
    }
    if (*endptr != '\0') {
        printf("Error: Age must be a valid integer (extra characters found).\n");
        return;
    }
    if ((errno == ERANGE && (age == LONG_MAX || age == LONG_MIN)) || (errno != 0 && age == 0)) {
        printf("Error: Age is out of the valid integer range.\n");
        return;
    }

    if (age < 0 || age > 120) {
        printf("Error: Age must be between 0 and 120.\n");
        return;
    }

    // 3. Perform Operation if all validations pass
    printf("Success: Hello %s, you are %ld years old.\n", username, age);
}

int main() {
    // Main function with 5 test cases
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Valid input
    printf("Test 1 (Valid):\n");
    performOperation("Alice", "30");
    printf("\n");

    // Test Case 2: Invalid age (non-numeric)
    printf("Test 2 (Invalid Age - Text):\n");
    performOperation("Bob", "twenty");
    printf("\n");

    // Test Case 3: Invalid age (out of range)
    printf("Test 3 (Invalid Age - Range):\n");
    performOperation("Charlie", "150");
    printf("\n");

    // Test Case 4: Invalid username (special characters)
    printf("Test 4 (Invalid Username - Chars):\n");
    performOperation("David!@", "40");
    printf("\n");
    
    // Test Case 5: Invalid username (too short)
    printf("Test 5 (Invalid Username - Length):\n");
    performOperation("Ed", "25");
    printf("\n");

    return 0;
}
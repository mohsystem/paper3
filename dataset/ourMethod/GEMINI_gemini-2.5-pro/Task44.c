#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#define USERNAME_MIN_LEN 3
#define USERNAME_MAX_LEN 16
#define AGE_MIN 0
#define AGE_MAX 120

// Validates a username.
// Rules: 3 to 16 characters long, alphanumeric characters only.
bool isValidUsername(const char* username) {
    if (username == NULL) {
        return false;
    }
    size_t len = strlen(username);
    if (len < USERNAME_MIN_LEN || len > USERNAME_MAX_LEN) {
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isalnum((unsigned char)username[i])) {
            return false;
        }
    }
    return true;
}

// Validates an age string.
// Rules: Must be a valid integer between 0 and 120 (inclusive).
bool isValidAge(const char* ageStr, int* age) {
    if (ageStr == NULL || age == NULL || ageStr[0] == '\0') {
        return false;
    }

    char* endptr = NULL;
    errno = 0; // Reset errno before the call

    long val = strtol(ageStr, &endptr, 10);

    // Check for conversion errors from strtol.
    // 1. Range error: value out of `long` range.
    // 2. No conversion: no digits were found.
    // 3. Incomplete conversion: trailing characters exist.
    if (errno == ERANGE || endptr == ageStr || *endptr != '\0') {
        return false;
    }

    // Check if the value fits in an `int`.
    if (val > INT_MAX || val < INT_MIN) {
       return false;
    }

    // Check the logical range for an age.
    if (val < AGE_MIN || val > AGE_MAX) {
        return false;
    }

    *age = (int)val;
    return true;
}

// A dummy function to simulate processing valid data.
void processUserData(const char* username, int age) {
    printf("Processing data for user '%s' with age %d.\n", username, age);
    // In a real application, business logic would go here.
    printf("Operation successful.\n");
}

// Helper function to run a single test case.
void runTestCase(const char* username_input, const char* age_input) {
    printf("--- Test Case ---\n");
    printf("Input Username: \"%s\", Input Age: \"%s\"\n", username_input, age_input);
    
    int age = 0;
    bool is_username_valid = isValidUsername(username_input);
    bool is_age_valid = isValidAge(age_input, &age);

    if (is_username_valid && is_age_valid) {
        printf("Validation successful.\n");
        processUserData(username_input, age);
    } else {
        printf("Validation failed.\n");
        if (!is_username_valid) {
            printf("  - Invalid username: Must be 3-16 alphanumeric characters.\n");
        }
        if (!is_age_valid) {
            printf("  - Invalid age: Must be a whole number between 0 and 120.\n");
        }
    }
    printf("\n");
}

int main(void) {
    // A series of test cases to demonstrate input validation.
    runTestCase("testuser1", "25");       // 1. Valid case
    runTestCase("sh", "30");              // 2. Invalid username (too short)
    runTestCase("longusername1234567", "40"); // 3. Invalid username (too long)
    runTestCase("user_name", "35");       // 4. Invalid username (non-alphanumeric character)
    runTestCase("testuser2", "150");      // 5. Invalid age (out of range)
    runTestCase("testuser3", "-5");       // 6. Invalid age (out of range)
    runTestCase("testuser4", "twenty");   // 7. Invalid age (not a number)
    runTestCase("testuser5", "25a");      // 8. Invalid age (contains non-digit characters)
    return 0;
}
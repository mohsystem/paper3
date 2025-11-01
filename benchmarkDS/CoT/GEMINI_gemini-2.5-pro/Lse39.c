#include <stdio.h>
#include <string.h>

/**
 * @brief Checks if the provided username and password are correct.
 * NOTE: Hardcoding credentials and using plaintext passwords are insecure
 * and done here for demonstration purposes only.
 * 
 * @param username The username to check.
 * @param password The password to check.
 * @return A constant character pointer to a string indicating success or failure.
 */
const char* login(const char* username, const char* password) {
    const char* CORRECT_USERNAME = "admin";
    const char* CORRECT_PASSWORD = "password123";
    
    // NOTE: strcmp is not a constant-time comparison function and can be
    // vulnerable to timing attacks. This is acceptable for this simple
    // demonstration but not for a production environment.
    if (strcmp(username, CORRECT_USERNAME) == 0 && strcmp(password, CORRECT_PASSWORD) == 0) {
        return "Login successful! Redirecting to the session page.";
    } else {
        return "Incorrect username or password.";
    }
}

int main() {
    printf("--- Login System Test Cases ---\n");

    // Test Case 1: Correct credentials
    printf("\nTest Case 1 (Correct):\n");
    printf("Input: user='admin', pass='password123'\n");
    printf("Output: %s\n", login("admin", "password123"));

    // Test Case 2: Incorrect password
    printf("\nTest Case 2 (Incorrect Password):\n");
    printf("Input: user='admin', pass='wrongpassword'\n");
    printf("Output: %s\n", login("admin", "wrongpassword"));

    // Test Case 3: Incorrect username
    printf("\nTest Case 3 (Incorrect Username):\n");
    printf("Input: user='user', pass='password123'\n");
    printf("Output: %s\n", login("user", "password123"));

    // Test Case 4: Both incorrect
    printf("\nTest Case 4 (Both Incorrect):\n");
    printf("Input: user='user', pass='wrongpassword'\n");
    printf("Output: %s\n", login("user", "wrongpassword"));

    // Test Case 5: Case-sensitive check
    printf("\nTest Case 5 (Case-Sensitive):\n");
    printf("Input: user='Admin', pass='password123'\n");
    printf("Output: %s\n", login("Admin", "password123"));
    
    printf("\n--- End of Test Cases ---\n");

    return 0;
}
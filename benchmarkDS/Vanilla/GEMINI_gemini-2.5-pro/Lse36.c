#include <stdio.h>
#include <string.h>

// Struct to hold user credentials
typedef struct {
    char username[50];
    char password[50];
} User;

// Simulating a user database
static User user_database[] = {
    {"admin", "admin123"},
    {"user_test", "password456"},
    {"john_doe", "jd_pass"}
};
static int db_size = sizeof(user_database) / sizeof(user_database[0]);

/**
 * Simulates a login function.
 * @param username The username entered by the user.
 * @param password The password entered by the user.
 * @return A constant string literal indicating the login status.
 */
const char* login(const char* username, const char* password) {
    int user_found = 0;
    
    // Iterate through the database to check if the username exists
    for (int i = 0; i < db_size; i++) {
        if (strcmp(user_database[i].username, username) == 0) {
            user_found = 1;
            // Username found, now check the password
            if (strcmp(user_database[i].password, password) == 0) {
                return "Login Successful. Redirecting to home page.";
            } else {
                return "Incorrect Password.";
            }
        }
    }

    // If loop finishes and user was not found
    return "User Not Found.";
}

int main() {
    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Successful login
    printf("Test 1 (admin, admin123):\n");
    const char* result1 = login("admin", "admin123");
    printf("Result: %s\n\n", result1);

    // Test Case 2: Incorrect password
    printf("Test 2 (user_test, wrongpassword):\n");
    const char* result2 = login("user_test", "wrongpassword");
    printf("Result: %s\n\n", result2);

    // Test Case 3: User does not exist
    printf("Test 3 (unknown_user, password123):\n");
    const char* result3 = login("unknown_user", "password123");
    printf("Result: %s\n\n", result3);

    // Test Case 4: Another successful login
    printf("Test 4 (john_doe, jd_pass):\n");
    const char* result4 = login("john_doe", "jd_pass");
    printf("Result: %s\n\n", result4);

    // Test Case 5: Existing user, incorrect password
    printf("Test 5 (admin, password456):\n");
    const char* result5 = login("admin", "password456");
    printf("Result: %s\n\n", result5);

    return 0;
}
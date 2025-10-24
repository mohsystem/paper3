#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// User structure
typedef struct {
    char username[50];
    char password[50];
} User;

// In a real application, passwords should be hashed and stored securely.
// For this example, we use a simple array of structs with plain text for demonstration.
User userDatabase[] = {
    {"admin", "password123"},
    {"user1", "securepass"},
    {"john.doe", "johndoe_pass"}
};
int numUsers = sizeof(userDatabase) / sizeof(User);

/**
 * @brief Authenticates a user based on username and password.
 * 
 * @param username The username to check.
 * @param password The password to verify.
 * @return true if authentication is successful, false otherwise.
 */
bool authenticate(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return false;
    }

    for (int i = 0; i < numUsers; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            // User found, check password
            if (strcmp(userDatabase[i].password, password) == 0) {
                return true; // Authentication successful
            }
            // Password incorrect for this user, no need to check others
            return false;
        }
    }

    // User not found
    return false;
}

int main() {
    printf("--- User Authentication Test Cases ---\n");

    // Test Case 1: Successful login
    bool result1 = authenticate("admin", "password123");
    printf("1. admin / password123 -> %s\n", result1 ? "Success" : "Failure");

    // Test Case 2: Wrong password
    bool result2 = authenticate("user1", "wrongpass");
    printf("2. user1 / wrongpass -> %s\n", result2 ? "Success" : "Failure");

    // Test Case 3: Non-existent user
    bool result3 = authenticate("unknown_user", "password123");
    printf("3. unknown_user / password123 -> %s\n", result3 ? "Success" : "Failure");

    // Test Case 4: Case-sensitive username
    bool result4 = authenticate("Admin", "password123");
    printf("4. Admin / password123 -> %s\n", result4 ? "Success" : "Failure");

    // Test Case 5: Null input
    bool result5 = authenticate(NULL, "some_password");
    printf("5. NULL / some_password -> %s\n", result5 ? "Success" : "Failure");

    return 0;
}
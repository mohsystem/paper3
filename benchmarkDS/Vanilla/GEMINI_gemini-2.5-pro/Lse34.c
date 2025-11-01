#include <stdio.h>
#include <string.h>

#define MAX_USERS 10
#define MAX_LEN 50

// Simulating a user database using an array of structs
typedef struct {
    char username[MAX_LEN];
    char password[MAX_LEN];
} User;

User userDatabase[MAX_USERS];
int userCount = 0;

/**
 * @brief Initializes the user database with some default users.
 */
void initializeDatabase() {
    strcpy(userDatabase[0].username, "admin");
    strcpy(userDatabase[0].password, "password123");
    strcpy(userDatabase[1].username, "user1");
    strcpy(userDatabase[1].password, "userpass");
    strcpy(userDatabase[2].username, "john.doe");
    strcpy(userDatabase[2].password, "johndoe@123");
    userCount = 3;
}

/**
 * @brief Checks user credentials against the database.
 * @param username The username entered by the user.
 * @param password The password entered by the user.
 * @return A string literal indicating the redirection path.
 */
const char* login(const char* username, const char* password) {
    for (int i = 0; i < userCount; ++i) {
        // Check if usernames match
        if (strcmp(userDatabase[i].username, username) == 0) {
            // If usernames match, check if passwords also match
            if (strcmp(userDatabase[i].password, password) == 0) {
                return "Login successful. Redirecting to dashboard page.";
            }
            // Username found but password incorrect, no need to check further
            return "Invalid credentials. Redirecting back to login page.";
        }
    }
    // Loop finished, username was not found
    return "Invalid credentials. Redirecting back to login page.";
}

int main() {
    initializeDatabase();

    // Test Cases
    printf("--- 5 Test Cases ---\n");

    // Test Case 1: Correct username and password
    printf("Test 1 (admin, password123): %s\n", login("admin", "password123"));

    // Test Case 2: Correct username, incorrect password
    printf("Test 2 (admin, wrongpassword): %s\n", login("admin", "wrongpassword"));

    // Test Case 3: Incorrect username
    printf("Test 3 (unknownuser, password123): %s\n", login("unknownuser", "password123"));
    
    // Test Case 4: Another valid user
    printf("Test 4 (user1, userpass): %s\n", login("user1", "userpass"));

    // Test Case 5: Case sensitivity check (username is case-sensitive)
    printf("Test 5 (Admin, password123): %s\n", login("Admin", "password123"));

    return 0;
}
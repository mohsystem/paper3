#include <stdio.h>
#include <string.h>

// User structure for database entries
typedef struct {
    const char* username;
    const char* password;
} User;

// In-memory array of structs to simulate a user database
User database[] = {
    {"admin", "password123"},
    {"user", "userpass"},
    {"testuser", "test"}
};
const int db_size = sizeof(database) / sizeof(database[0]);

/**
 * Checks user credentials against the database.
 * @param username The username to check.
 * @param password The password to check.
 * @return "success" on match, "cant_login" on mismatch, or "internal_server_error" on NULL input.
 */
const char* login(const char* username, const char* password) {
    // Handle NULL pointers as an internal error
    if (username == NULL || password == NULL) {
        return "internal_server_error";
    }

    for (int i = 0; i < db_size; i++) {
        // Check if username matches
        if (strcmp(database[i].username, username) == 0) {
            // If username matches, check password
            if (strcmp(database[i].password, password) == 0) {
                return "success";
            }
            // Username found but password incorrect
            return "cant_login";
        }
    }
    
    // Username not found in the database
    return "cant_login";
}

int main() {
    // Test Case 1: Correct username and password
    printf("Test 1: login(\"admin\", \"password123\") -> %s\n", login("admin", "password123"));

    // Test Case 2: Correct username, incorrect password
    printf("Test 2: login(\"user\", \"wrongpassword\") -> %s\n", login("user", "wrongpassword"));

    // Test Case 3: Incorrect username
    printf("Test 3: login(\"unknown\", \"userpass\") -> %s\n", login("unknown", "userpass"));

    // Test Case 4: Empty credentials
    printf("Test 4: login(\"\", \"\") -> %s\n", login("", ""));

    // Test Case 5: Another correct username and password
    printf("Test 5: login(\"testuser\", \"test\") -> %s\n", login("testuser", "test"));

    return 0;
}
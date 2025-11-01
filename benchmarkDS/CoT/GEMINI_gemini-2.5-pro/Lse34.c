#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// --- WARNING ---
// This example simulates a login system. It does NOT use a cryptographically
// secure hash function because that requires an external library like OpenSSL.
// DO NOT use this hashing function in a real application. It is for
// demonstration purposes only. In a real system, use a library to perform
// strong, salted hashing with an algorithm like Argon2 or BCrypt.

// A simple, non-secure hash function for demonstration.
// Returns a dynamically allocated string that must be freed.
char* simple_hash(const char* password) {
    if (!password) return NULL;
    unsigned long hash = 5381;
    for (int i = 0; password[i] != '\0'; i++) {
        hash = ((hash << 5) + hash) + password[i]; // hash * 33 + c
    }
    // Allocate memory for the string representation of the hash
    char* hash_str = (char*)malloc(21); // Max length for unsigned long + null terminator
    if (hash_str) {
        snprintf(hash_str, 21, "%lu", hash);
    }
    return hash_str;
}

// Constant-time string comparison to mitigate timing attacks.
// Compares every character regardless of when a mismatch is found.
// Returns 0 if strings are equal, 1 otherwise.
int secure_compare(const char* s1, const char* s2) {
    if (!s1 || !s2) return 1; // Not equal if one is null

    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    if (len1 != len2) {
        return 1;
    }

    int result = 0;
    for (size_t i = 0; i < len1; ++i) {
        result |= (s1[i] ^ s2[i]);
    }
    return result != 0;
}

// Structure to represent a user record
typedef struct {
    const char* username;
    const char* hashed_password;
} User;

// Simulates a database table of users.
// Hashed with simple_hash: "password123", "adminPass", "securePwd!"
User user_database[] = {
    {"alice", "1236394334"},
    {"bob", "2134377665"},
    {"charlie", "1914283839"},
    {NULL, NULL} // Sentinel to mark the end of the array
};

/**
 * Attempts to log a user in.
 * @param username The username entered by the user.
 * @param password The password entered by the user.
 * @return A string literal indicating the result.
 */
const char* login(const char* username, const char* password) {
    // 1. Basic input validation
    if (username == NULL || username[0] == '\0' || password == NULL || password[0] == '\0') {
        return "Invalid username or password. Redirecting to login page...";
    }

    const char* stored_hashed_password = NULL;
    int i = 0;
    while(user_database[i].username != NULL) {
        if (strcmp(user_database[i].username, username) == 0) {
            stored_hashed_password = user_database[i].hashed_password;
            break;
        }
        i++;
    }

    char* provided_hashed_password = simple_hash(password);

    // 4. If user does not exist, return a generic error message
    if (stored_hashed_password == NULL) {
        // We still compare against a dummy hash to make the timing similar
        // to a failed password attempt, which can help mitigate timing attacks.
        secure_compare(provided_hashed_password, "dummy_hash_for_timing");
        free(provided_hashed_password); // Clean up memory
        return "Invalid username or password. Redirecting to login page...";
    }

    // 5. Compare the hashes in a way that is resistant to timing attacks.
    int match_result = secure_compare(stored_hashed_password, provided_hashed_password);
    
    free(provided_hashed_password); // Clean up memory

    if (match_result == 0) {
        return "Login successful! Redirecting to dashboard page...";
    } else {
        return "Invalid username or password. Redirecting to login page...";
    }
}

int main() {
    printf("--- 5 Test Cases ---\n");

    // Test Case 1: Correct username and password
    printf("1. Testing with correct credentials (alice):\n");
    printf("   Result: %s\n\n", login("alice", "password123"));

    // Test Case 2: Correct username, incorrect password
    printf("2. Testing with incorrect password (alice):\n");
    printf("   Result: %s\n\n", login("alice", "wrongpassword"));

    // Test Case 3: Incorrect username
    printf("3. Testing with non-existent user (david):\n");
    printf("   Result: %s\n\n", login("david", "somepassword"));

    // Test Case 4: NULL input for username
    printf("4. Testing with null username:\n");
    printf("   Result: %s\n\n", login(NULL, "somepassword"));

    // Test Case 5: Correct credentials for another user
    printf("5. Testing with correct credentials (charlie):\n");
    printf("   Result: %s\n\n", login("charlie", "securePwd!"));

    return 0;
}
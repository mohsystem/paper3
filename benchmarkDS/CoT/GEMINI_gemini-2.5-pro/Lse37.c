#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// User structure to simulate a database record.
typedef struct {
    const char* username;
    const char* hashedPassword; // In a real app, this would be a proper hash.
} User;

// In a real application, this would be a secure database connection.
// Passwords must be stored as salted hashes using a strong algorithm like Argon2, scrypt, or bcrypt.
// This array is a simplified simulation for demonstration purposes.
User userDatabase[] = {
    {"admin", "hashed_password_for_admin"},
    {"testuser", "hashed_password_for_user"},
    {NULL, NULL} // Sentinel to mark the end of the array
};

/**
 * Performs a constant-time string comparison to mitigate timing attacks.
 * It compares every character regardless of when a mismatch is found.
 *
 * @param a The first string.
 * @param b The second string.
 * @return true if the strings are equal, false otherwise.
 */
bool secureStrcmp(const char* a, const char* b) {
    if (a == NULL || b == NULL) return false;

    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    
    if (len_a != len_b) {
        return false;
    }
    
    // Use a variable to accumulate differences without short-circuiting.
    int diff = 0;
    for (size_t i = 0; i < len_a; ++i) {
        diff |= a[i] ^ b[i];
    }
    return diff == 0;
}

/**
 * Simulates a user login process.
 *
 * @param username The username provided by the user.
 * @param password The password provided by the user.
 * @return A string indicating the result: "success", "cant_login", or "internal_server_error".
 */
const char* login(const char* username, const char* password) {
    // 1. Input Validation
    if (username == NULL || *username == '\0' || password == NULL || *password == '\0') {
        return "cant_login";
    }

    const char* storedHashedPassword = NULL;

    // 2. Database Lookup: Find the user in our simulated database.
    for (int i = 0; userDatabase[i].username != NULL; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            storedHashedPassword = userDatabase[i].hashedPassword;
            break;
        }
    }
    
    // 3. Password Verification
    // In a real app, you would hash the input password with the user's salt
    // and then compare the result with the stored hash.
    // The comparison MUST be done in constant time.
    bool is_match = false;
    if (storedHashedPassword != NULL) {
        // User found, compare passwords securely.
        is_match = secureStrcmp(storedHashedPassword, password);
    } else {
        // User not found, perform a dummy comparison to prevent timing leaks.
        secureStrcmp("dummy_hash_to_prevent_timing_attack", password);
    }

    if (is_match) {
        return "success";
    } else {
        // Generic error for both non-existent user and wrong password to prevent user enumeration.
        return "cant_login";
    }

    // In C, catching generic errors is more complex. A real implementation would have more robust error checking.
    // For this example, we assume no "internal_server_error" occurs unless memory allocation fails, etc.
}


int main() {
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Successful login
    const char* result1 = login("admin", "hashed_password_for_admin");
    printf("Test Case 1 (Correct Credentials): %s\n", result1);

    // Test Case 2: Incorrect password
    const char* result2 = login("admin", "wrong_password");
    printf("Test Case 2 (Incorrect Password): %s\n", result2);

    // Test Case 3: Non-existent user
    const char* result3 = login("unknown_user", "some_password");
    printf("Test Case 3 (Non-existent User): %s\n", result3);

    // Test Case 4: Null input
    const char* result4 = login(NULL, "some_password");
    printf("Test Case 4 (Null Username): %s\n", result4);

    // Test Case 5: Empty input
    const char* result5 = login("testuser", "");
    printf("Test Case 5 (Empty Password): %s\n", result5);

    printf("--- Test Cases Finished ---\n");

    return 0;
}
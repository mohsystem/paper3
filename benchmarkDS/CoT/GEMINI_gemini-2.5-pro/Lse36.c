#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_HASH_LEN 100
#define DB_SIZE 3

typedef struct {
    char username[MAX_USERNAME_LEN];
    char hashedPassword[MAX_HASH_LEN];
} User;

// Simulates a database. In a real application, passwords would be securely hashed.
User userDatabase[DB_SIZE] = {
    {"admin", "321drowssap_dehsah"},    // Simulated hash of "password123"
    {"jdoe", "#@!ssaPeruces_dehsah"},   // Simulated hash of "securePass!@#"
    {"testuser", "tset_dehsah"}         // Simulated hash of "test"
};

/**
 * Simulates hashing a password.
 * This is NOT a secure hash. For demonstration only.
 * WARNING: This function is not safe against buffer overflows if password is too long.
 * In a real application, use a dedicated crypto library.
 */
void hashPassword(const char* password, char* hashBuffer, size_t bufferSize) {
    char reversedPassword[MAX_PASSWORD_LEN];
    strncpy(reversedPassword, password, MAX_PASSWORD_LEN - 1);
    reversedPassword[MAX_PASSWORD_LEN - 1] = '\0';
    
    // Reverse the string
    int len = strlen(reversedPassword);
    for (int i = 0; i < len / 2; i++) {
        char temp = reversedPassword[i];
        reversedPassword[i] = reversedPassword[len - i - 1];
        reversedPassword[len - i - 1] = temp;
    }
    
    snprintf(hashBuffer, bufferSize, "%s_dehsah", reversedPassword);
}

/**
 * Performs a constant-time comparison of two strings to prevent timing attacks.
 * A simple strcmp() is NOT constant time.
 * A real implementation should use a verified crypto library function.
 */
bool secure_strcmp(const char* s1, const char* s2) {
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);

    if (len1 != len2) {
        return false;
    }

    int diff = 0;
    for (size_t i = 0; i < len1; ++i) {
        diff |= s1[i] ^ s2[i];
    }
    return diff == 0;
}


/**
 * Simulates a user login process.
 *
 * @param username The username entered by the user.
 * @param password The password entered by the user.
 * @return A string literal indicating the result of the login attempt.
 */
const char* login(const char* username, const char* password) {
    if (username == NULL || password == NULL || username[0] == '\0' || password[0] == '\0') {
        return "Username or password cannot be empty.";
    }

    char inputHashedPassword[MAX_HASH_LEN];
    hashPassword(password, inputHashedPassword, MAX_HASH_LEN);

    const User* foundUser = NULL;

    // Find user
    for (int i = 0; i < DB_SIZE; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            foundUser = &userDatabase[i];
            break;
        }
    }
    
    // To prevent timing attacks, always perform a comparison.
    // If user is not found, compare against a dummy hash.
    if (foundUser != NULL && secure_strcmp(foundUser->hashedPassword, inputHashedPassword)) {
        return "Login successful! Redirecting to home page...";
    } else {
        // Perform a dummy comparison if user was not found to mitigate timing attacks
        if(foundUser == NULL) {
            secure_strcmp(inputHashedPassword, "dummy_hash_for_timing_mitigation");
        }
        // Generic error message prevents username enumeration.
        return "Invalid username or password.";
    }
}

int main() {
    printf("--- Login Test Cases ---\n");

    // Test Case 1: Successful login
    printf("1. admin / password123 -> %s\n", login("admin", "password123"));

    // Test Case 2: Incorrect password
    printf("2. admin / wrongpassword -> %s\n", login("admin", "wrongpassword"));

    // Test Case 3: Non-existent user
    printf("3. unknownuser / password123 -> %s\n", login("unknownuser", "password123"));

    // Test Case 4: Another successful login
    printf("4. jdoe / securePass!@# -> %s\n", login("jdoe", "securePass!@#"));

    // Test Case 5: Empty credentials
    printf("5. \"\" / \"\" -> %s\n", login("", ""));

    return 0;
}
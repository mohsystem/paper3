#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERS 100
#define MAX_FIELD_LENGTH 50
#define RESPONSE_BUFFER_SIZE 256

// User struct to hold user information
typedef struct {
    char username[MAX_FIELD_LENGTH];
    char hashedPassword[MAX_FIELD_LENGTH];
    char firstName[MAX_FIELD_LENGTH];
    char lastName[MAX_FIELD_LENGTH];
    char email[MAX_FIELD_LENGTH];
} User;

// In-memory array to act as a database
static User database[MAX_USERS];
static int user_count = 0;

/**
 * !SECURITY WARNING!
 * This is a placeholder hashing function for demonstration purposes ONLY.
 * It is NOT secure. In a real-world C application, you MUST use a
 * well-vetted cryptographic library like OpenSSL (libcrypto) or libsodium
 * to implement a strong hashing algorithm like Argon2, scrypt, or bcrypt.
 *
 * @param password The password to "hash".
 * @param outputBuffer The buffer to store the "hashed" password.
 * @param bufferSize The size of the output buffer.
 */
void hashPassword(const char* password, char* outputBuffer, size_t bufferSize) {
    // This is a simple, insecure placeholder that reverses the password.
    // In a real app, a random salt should be generated for each user.
    snprintf(outputBuffer, bufferSize, "salted_%s", password);
    
    // Reverse the string in place (excluding the "salted_" prefix)
    char* to_reverse = outputBuffer + 7;
    int len = strlen(to_reverse);
    for (int i = 0; i < len / 2; i++) {
        char temp = to_reverse[i];
        to_reverse[i] = to_reverse[len - i - 1];
        to_reverse[len - i - 1] = temp;
    }
}

/**
 * Registers a new user.
 *
 * @param username The username.
 * @param password The user's password.
 * @param firstName The user's first name.
 * @param lastName The user's last name.
 * @param email The user's email address.
 * @return A dynamically allocated string with the status message. The caller must free this string.
 */
char* registerUser(const char* username, const char* password, const char* firstName, const char* lastName, const char* email) {
    char* response = (char*)malloc(RESPONSE_BUFFER_SIZE * sizeof(char));
    if (response == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    // Input validation
    if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
        snprintf(response, RESPONSE_BUFFER_SIZE, "Registration failed: Username and password cannot be empty.");
        return response;
    }
    
    // Check if username already exists
    for (int i = 0; i < user_count; i++) {
        if (strcmp(database[i].username, username) == 0) {
            snprintf(response, RESPONSE_BUFFER_SIZE, "Registration failed: Username '%s' already exists.", username);
            return response;
        }
    }
    
    // Check if database is full
    if (user_count >= MAX_USERS) {
        snprintf(response, RESPONSE_BUFFER_SIZE, "Registration failed: Database is full.");
        return response;
    }

    // Add new user
    User* newUser = &database[user_count];
    
    // Securely copy strings
    strncpy(newUser->username, username, MAX_FIELD_LENGTH - 1);
    newUser->username[MAX_FIELD_LENGTH - 1] = '\0';
    
    strncpy(newUser->firstName, firstName, MAX_FIELD_LENGTH - 1);
    newUser->firstName[MAX_FIELD_LENGTH - 1] = '\0';

    strncpy(newUser->lastName, lastName, MAX_FIELD_LENGTH - 1);
    newUser->lastName[MAX_FIELD_LENGTH - 1] = '\0';

    strncpy(newUser->email, email, MAX_FIELD_LENGTH - 1);
    newUser->email[MAX_FIELD_LENGTH - 1] = '\0';

    // Hash the password
    hashPassword(password, newUser->hashedPassword, MAX_FIELD_LENGTH);

    user_count++;
    
    snprintf(response, RESPONSE_BUFFER_SIZE, "Registration for '%s' succeeded.", username);
    return response;
}

void runTests() {
    printf("--- User Registration System ---\n");
    char* result;

    // Test Case 1: Successful registration
    result = registerUser("jdoe", "Password123", "John", "Doe", "jdoe@example.com");
    printf("Test Case 1: %s\n", result);
    free(result);

    // Test Case 2: Attempt to register with an existing username
    result = registerUser("jdoe", "AnotherPass", "Jane", "Doe", "jane.d@example.com");
    printf("Test Case 2: %s\n", result);
    free(result);

    // Test Case 3: Successful registration for another user
    result = registerUser("asmith", "SecureP@ss!", "Alice", "Smith", "asmith@example.com");
    printf("Test Case 3: %s\n", result);
    free(result);

    // Test Case 4: Registration with an empty password
    result = registerUser("bwhite", "", "Bob", "White", "bwhite@example.com");
    printf("Test Case 4: %s\n", result);
    free(result);

    // Test Case 5: Another successful registration
    result = registerUser("cgreen", "MyP@ssw0rd", "Charlie", "Green", "cgreen@example.com");
    printf("Test Case 5: %s\n", result);
    free(result);

    // Print database state for verification
    printf("\nCurrent database state:\n");
    for (int i = 0; i < user_count; i++) {
        printf("Username: %s, Hashed Password: %s\n", database[i].username, database[i].hashedPassword);
    }
}

int main() {
    // In C, there are no classes. The main function can call the test runner directly.
    runTests();
    return 0;
}
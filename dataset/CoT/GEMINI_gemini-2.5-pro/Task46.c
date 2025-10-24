#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_USERS 100
#define MAX_NAME_LEN 50
#define MAX_EMAIL_LEN 100
#define MAX_HASH_LEN 128 // Placeholder length
#define MIN_PASSWORD_LEN 8

// A simple structure to hold user data
typedef struct {
    char name[MAX_NAME_LEN];
    char email[MAX_EMAIL_LEN];
    char hashedPassword[MAX_HASH_LEN];
    char salt[32]; // Placeholder length
} User;

// In-memory array to act as a database
User userDatabase[MAX_USERS];
int userCount = 0;

/**
 * WARNING: THIS IS A PLACEHOLDER HASHING FUNCTION FOR DEMONSTRATION ONLY.
 * DO NOT USE IN A PRODUCTION ENVIRONMENT.
 * In a real application, use a well-vetted, strong cryptographic library
 * like OpenSSL, libsodium, or bcrypt for password hashing.
 * This function is not secure.
 */
void hashPassword(const char* password, const char* salt, char* outputBuffer) {
    // This is a dummy hash. It's not secure.
    snprintf(outputBuffer, MAX_HASH_LEN, "hashed_%s_%s", password, salt);
}

// Basic email validation (checks for '@' and '.')
bool isValidEmail(const char* email) {
    const char* at_char = strchr(email, '@');
    if (at_char == NULL || at_char == email) { // No '@' or starts with '@'
        return false;
    }
    const char* dot_char = strrchr(at_char, '.');
    if (dot_char == NULL || dot_char == at_char + 1 || dot_char[1] == '\0') { // No '.' after '@', or no characters after '.'
        return false;
    }
    return true;
}

/**
 * Registers a new user after validating the inputs.
 * @param name The user's name.
 * @param email The user's email.
 * @param password The user's password.
 * @return A string indicating the result of the registration attempt. The caller is responsible for freeing this string.
 */
char* registerUser(const char* name, const char* email, const char* password) {
    char* result = (char*)malloc(256 * sizeof(char));
    if (result == NULL) {
        // Not enough memory, should not happen in this simple case
        return NULL; 
    }

    // 1. Input Validation
    if (name == NULL || strlen(name) == 0 || name[0] == ' ') {
        snprintf(result, 256, "Registration failed: Name cannot be empty.");
        return result;
    }
    if (email == NULL || !isValidEmail(email)) {
        snprintf(result, 256, "Registration failed: Invalid email format.");
        return result;
    }
    if (password == NULL || strlen(password) < MIN_PASSWORD_LEN) {
        snprintf(result, 256, "Registration failed: Password must be at least %d characters long.", MIN_PASSWORD_LEN);
        return result;
    }
    if (userCount >= MAX_USERS) {
        snprintf(result, 256, "Registration failed: Database is full.");
        return result;
    }

    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].email, email) == 0) {
            snprintf(result, 256, "Registration failed: Email already exists.");
            return result;
        }
    }

    // 2. Secure Password Handling (using placeholder functions)
    const char* salt = "dummy_salt_123"; // Dummy salt
    char hashedPassword[MAX_HASH_LEN];
    hashPassword(password, salt, hashedPassword);
    
    // 3. Store User Information
    User* newUser = &userDatabase[userCount];
    strncpy(newUser->name, name, MAX_NAME_LEN - 1);
    newUser->name[MAX_NAME_LEN - 1] = '\0'; // Ensure null termination
    
    strncpy(newUser->email, email, MAX_EMAIL_LEN - 1);
    newUser->email[MAX_EMAIL_LEN - 1] = '\0';
    
    strncpy(newUser->salt, salt, 31);
    newUser->salt[31] = '\0';
    
    strncpy(newUser->hashedPassword, hashedPassword, MAX_HASH_LEN - 1);
    newUser->hashedPassword[MAX_HASH_LEN - 1] = '\0';
    
    userCount++;

    snprintf(result, 256, "User '%s' registered successfully.", name);
    return result;
}

int main() {
    printf("--- User Registration System Test Cases ---\n");
    char* result;

    // Test Case 1: Successful registration
    result = registerUser("Alice Smith", "alice@example.com", "Password123!");
    printf("1. %s\n", result);
    free(result);

    // Test Case 2: Attempt to register with an existing email
    result = registerUser("Alice Jones", "alice@example.com", "AnotherPassword456");
    printf("2. %s\n", result);
    free(result);

    // Test Case 3: Attempt to register with an invalid email format
    result = registerUser("Bob Johnson", "bob-at-invalid", "SecurePass789");
    printf("3. %s\n", result);
    free(result);

    // Test Case 4: Attempt to register with a weak (short) password
    result = registerUser("Charlie Brown", "charlie@example.com", "pass");
    printf("4. %s\n", result);
    free(result);

    // Test Case 5: Attempt to register with an empty name
    result = registerUser("", "david@example.com", "ValidPassword101");
    printf("5. %s\n", result);
    free(result);

    // Print database contents (for verification)
    printf("\nCurrent users in database:\n");
    for (int i = 0; i < userCount; ++i) {
        printf("- User{name='%s', email='%s'}\n", userDatabase[i].name, userDatabase[i].email);
    }

    return 0;
}
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERNAME_LEN 50
#define HASH_LEN 17 // 16 hex chars + null terminator
#define MAX_USERS 10

typedef struct {
    char username[MAX_USERNAME_LEN];
    char hashedPassword[HASH_LEN];
} User;

// WARNING: THIS IS A TOY HASHING FUNCTION FOR DEMONSTRATION ONLY.
// IT IS NOT CRYPTOGRAPHICALLY SECURE.
// In a real C application, you MUST use a well-vetted cryptography library
// like OpenSSL or libsodium to handle password hashing (e.g., with Argon2, scrypt, or bcrypt).
void hashPassword(const char* password, char* hashOutput) {
    if (password == NULL || strlen(password) == 0) {
        hashOutput[0] = '\0';
        return;
    }
    unsigned long hash = 5381;
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c; // djb2 algorithm
    }
    snprintf(hashOutput, HASH_LEN, "%016lx", hash);
}

// Performs a constant-time comparison to help mitigate timing attacks.
bool secureCompare(const char* a, const char* b) {
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

bool authenticateUser(const User* userDatabase, int userCount, const char* username, const char* password) {
    // 1. Basic input validation
    if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
        return false;
    }

    // 2. Find the user
    const User* foundUser = NULL;
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            foundUser = &userDatabase[i];
            break;
        }
    }

    if (foundUser == NULL) {
        return false; // User not found
    }

    // 3. Hash the provided password
    char inputHash[HASH_LEN];
    hashPassword(password, inputHash);

    // 4. Securely compare the hashes
    return secureCompare(foundUser->hashedPassword, inputHash);
}

int main() {
    // In-memory storage for users.
    // In a real application, this would be a database.
    User userDatabase[MAX_USERS];
    int userCount = 2;

    // Pre-populate with some users. Passwords are "hashed" using the insecure demo function.
    // "password123" -> "000000069506164f"
    // "adminPass!" -> "0000000693a81744"
    strcpy(userDatabase[0].username, "user1");
    strcpy(userDatabase[0].hashedPassword, "000000069506164f");
    strcpy(userDatabase[1].username, "admin");
    strcpy(userDatabase[1].hashedPassword, "0000000693a81744");

    printf("--- C Test Cases ---\n");

    // Test Case 1: Correct username, correct password
    bool result1 = authenticateUser(userDatabase, userCount, "user1", "password123");
    printf("Test 1 (user1, password123): %s\n", result1 ? "Success" : "Failure");

    // Test Case 2: Correct username, incorrect password
    bool result2 = authenticateUser(userDatabase, userCount, "user1", "wrongpassword");
    printf("Test 2 (user1, wrongpassword): %s\n", result2 ? "Success" : "Failure");

    // Test Case 3: Incorrect username
    bool result3 = authenticateUser(userDatabase, userCount, "unknownuser", "password123");
    printf("Test 3 (unknownuser, password123): %s\n", result3 ? "Success" : "Failure");

    // Test Case 4: Correct username, empty password
    bool result4 = authenticateUser(userDatabase, userCount, "admin", "");
    printf("Test 4 (admin, ''): %s\n", result4 ? "Success" : "Failure");

    // Test Case 5: NULL username
    bool result5 = authenticateUser(userDatabase, userCount, NULL, "somepass");
    printf("Test 5 (null, somepass): %s\n", result5 ? "Success" : "Failure");

    return 0;
}